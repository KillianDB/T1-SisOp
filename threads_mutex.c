#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TARGET_COUNT 1000000000LL

static long long global_counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *g_log_file = NULL;

typedef struct {
    int id;
    long long increments;
    long long checkpoints;
} thread_args_t;

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

static void *worker(void *arg) {
    thread_args_t *data = (thread_args_t *)arg;
    long long interval = data->checkpoints > 0 ? data->checkpoints : data->increments;

    log_message("[Thread %d] Inicio: quota=%lld\n", data->id, data->increments);

    for (long long i = 1; i <= data->increments; i++) {
        pthread_mutex_lock(&counter_mutex);
        global_counter++;
        pthread_mutex_unlock(&counter_mutex);

        if (interval > 0 && (i % interval == 0 || i == data->increments)) {
            long long percent = (i * 100LL) / data->increments;
            log_message("[Thread %d] Progresso: %lld/%lld (%lld%%)\n", data->id, i, data->increments, percent);
        }
    }

    log_message("[Thread %d] Fim\n", data->id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s <N_THREADS> [TARGET_COUNT]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n_threads = (int)parse_positive_int(argv[1], "N_THREADS");
    long long target_count = TARGET_COUNT;
    if (argc == 3) {
        target_count = parse_positive_int(argv[2], "TARGET_COUNT");
    }
    char output_file[64];
    snprintf(output_file, sizeof(output_file), "threads_mutex_%d.txt", n_threads);
    pthread_t *threads = malloc((size_t)n_threads * sizeof(*threads));
    thread_args_t *args = malloc((size_t)n_threads * sizeof(*args));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Falha ao alocar memoria: %s\n", strerror(errno));
        free(threads);
        free(args);
        return EXIT_FAILURE;
    }

    g_log_file = fopen(output_file, "w");
    if (g_log_file == NULL) {
        fprintf(stderr, "Falha ao abrir %s: %s\n", output_file, strerror(errno));
        free(threads);
        free(args);
        return EXIT_FAILURE;
    }

    long long base = target_count / n_threads;
    long long rem = target_count % n_threads;

    log_message("=== Threads com pthread_mutex ===\n");
    log_message("Meta global: %lld\n", target_count);
    log_message("Numero de threads: %d\n", n_threads);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n_threads; i++) {
        args[i].id = i;
        args[i].increments = base + (i < rem ? 1 : 0);
        args[i].checkpoints = args[i].increments / 4;

        int rc = pthread_create(&threads[i], NULL, worker, &args[i]);
        if (rc != 0) {
            fprintf(stderr, "Erro ao criar thread %d: %s\n", i, strerror(rc));
            fclose(g_log_file);
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < n_threads; i++) {
        int rc = pthread_join(threads[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "Erro ao aguardar thread %d: %s\n", i, strerror(rc));
            fclose(g_log_file);
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (double)(end.tv_sec - start.tv_sec) +
                     (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    log_message("Contador final observado: %lld\n", global_counter);
    log_message("Esperado: %lld\n", target_count);
    log_message("Tempo total: %.6f segundos\n", elapsed);
    log_message("Log salvo em: %s\n", output_file);

    fclose(g_log_file);
    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
