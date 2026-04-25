#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TARGET_COUNT 1000000000LL
#define OUTPUT_FILE "processes_semaphore_output.txt"

typedef struct {
    long long counter;
} shared_data_t;

static FILE *g_log_file = NULL;

static void log_message(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    if (g_log_file != NULL) {
        va_start(args, fmt);
        vfprintf(g_log_file, fmt, args);
        va_end(args);
        fflush(g_log_file);
    }

    fflush(stdout);
}

static long long parse_positive_int(const char *text, const char *name) {
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (text == end || *end != '\0' || value <= 0) {
        fprintf(stderr, "Valor invalido para %s: %s\n", name, text);
        exit(EXIT_FAILURE);
    }

    return value;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s <N_PROCESSOS> [TARGET_COUNT]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n_processes = (int)parse_positive_int(argv[1], "N_PROCESSOS");
    long long target_count = TARGET_COUNT;
    if (argc == 3) {
        target_count = parse_positive_int(argv[2], "TARGET_COUNT");
    }

    g_log_file = fopen(OUTPUT_FILE, "w");
    if (g_log_file == NULL) {
        fprintf(stderr, "Falha ao abrir %s: %s\n", OUTPUT_FILE, strerror(errno));
        return EXIT_FAILURE;
    }

    shared_data_t *shared = mmap(NULL, sizeof(*shared), PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) {
        fprintf(stderr, "Falha no mmap: %s\n", strerror(errno));
        fclose(g_log_file);
        return EXIT_FAILURE;
    }
    shared->counter = 0;

    char sem_name[64];
    snprintf(sem_name, sizeof(sem_name), "/counter_sem_%ld", (long)getpid());

    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 1);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Falha no sem_open (%s): %s\n", sem_name, strerror(errno));
        munmap(shared, sizeof(*shared));
        fclose(g_log_file);
        return EXIT_FAILURE;
    }

    long long base = target_count / n_processes;
    long long rem = target_count % n_processes;

    log_message("=== Processos (fork) com semaforo (sem_open) ===\n");
    log_message("Meta global: %lld\n", target_count);
    log_message("Numero de processos: %d\n", n_processes);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n_processes; i++) {
        long long quota = base + (i < rem ? 1 : 0);
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Erro no fork para processo %d: %s\n", i, strerror(errno));
            sem_close(sem);
            sem_unlink(sem_name);
            munmap(shared, sizeof(*shared));
            fclose(g_log_file);
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            long long interval = quota / 4;
            if (interval == 0) {
                interval = quota;
            }

            log_message("[Processo %d | PID=%ld] Inicio: quota=%lld\n", i, (long)getpid(), quota);

            for (long long j = 1; j <= quota; j++) {
                sem_wait(sem);
                shared->counter++;
                sem_post(sem);

                if (interval > 0 && (j % interval == 0 || j == quota)) {
                    long long percent = (j * 100LL) / quota;
                    log_message("[Processo %d | PID=%ld] Progresso: %lld/%lld (%lld%%)\n",
                                i, (long)getpid(), j, quota, percent);
                }
            }

            log_message("[Processo %d | PID=%ld] Fim\n", i, (long)getpid());
            sem_close(sem);
            munmap(shared, sizeof(*shared));
            fclose(g_log_file);
            _exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < n_processes; i++) {
        int status = 0;
        pid_t finished = wait(&status);
        if (finished < 0) {
            fprintf(stderr, "Erro no wait: %s\n", strerror(errno));
            sem_close(sem);
            sem_unlink(sem_name);
            munmap(shared, sizeof(*shared));
            fclose(g_log_file);
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            log_message("[Pai] Filho PID=%ld terminou com codigo %d\n", (long)finished, WEXITSTATUS(status));
        } else {
            log_message("[Pai] Filho PID=%ld terminou de forma anormal\n", (long)finished);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (double)(end.tv_sec - start.tv_sec) +
                     (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    log_message("Contador final observado: %lld\n", shared->counter);
    log_message("Esperado: %lld\n", target_count);
    log_message("Tempo total: %.6f segundos\n", elapsed);
    log_message("Log salvo em: %s\n", OUTPUT_FILE);

    sem_close(sem);
    sem_unlink(sem_name);
    munmap(shared, sizeof(*shared));
    fclose(g_log_file);
    return EXIT_SUCCESS;
}
