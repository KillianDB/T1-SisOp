# Trabalho 1 - Sistemas Operacionais

Trabalho 1 da cadeira de Sistemas Operacionais ministrada pelo professor doutor Filipo Mor.

## Enunciado

![Enunciado - Parte 1](p1.png)
![Enunciado - Parte 2](p2.png)

## Membros do Grupo

- Antonio Marcos de Oliveira Pereira
- Felipe Burgdurf Seelend
- Killian Domingues Batista

## Como Rodar o Projeto

1. `make clean`
2. `make`
3. `ls -la threads_no_sync threads_mutex processes_no_sync processes_semaphore`
4. Execute uma das implementacoes:
   - `./threads_no_sync <numero de threads>`
   - `./threads_mutex <numero de threads>`
   - `./processes_no_sync <numero de processos>`
   - `./processes_semaphore <numero de processos>`

Observacao: a cada execucao, o arquivo de saida `.txt` correspondente sera sobrescrito (tempo total e resultado de cada ciclo). Para cada implementacao, existe apenas um arquivo por valor de N.

## Hardware Usado (`lscpu`)

```text
Arquitetura:                  x86_64
  Modo(s) operacional da CPU: 32-bit, 64-bit
  Tamanhos de endereco:       45 bits physical, 48 bits virtual
  Ordem dos bytes:            Little Endian
CPU(s):                       4
  Lista de CPU(s) on-line:    0-3
ID de fornecedor:             GenuineIntel
  Nome do modelo:             Intel(R) Xeon(R) Gold 6132 CPU @ 2.60GHz
    Familia da CPU:           6
    Modelo:                   85
    Thread(s) per nucleo:     1
    Nucleo(s) por soquete:    1
    Soquete(s):               4
    Step:                     4
    BogoMIPS:                 5187,81
    Opcoes:                   fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse
                              sse2 ss syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon nopl xtopology tsc_reliable no
                              nstop_tsc cpuid tsc_known_freq pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic movbe p
                              opcnt tsc_deadline_timer aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch pti s
                              sbd ibrs ibpb stibp fsgsbase tsc_adjust bmi1 avx2 smep bmi2 invpcid avx512f avx512dq rdseed a
                              dx smap clflushopt clwb avx512cd avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves arat pku os
                              pke md_clear flush_l1d arch_capabilities
Recursos de virtualizacao:
  Fabricante do hipervisor:   VMware
  Tipo de virtualizacao:      completo
Caches (soma de todos):
  L1d:                        128 KiB (4 instancias)
  L1i:                        128 KiB (4 instancias)
  L2:                         4 MiB (4 instancias)
  L3:                         77 MiB (4 instancias)
NUMA:
  No(s) de NUMA:              1
  CPU(s) de no0 NUMA:         0-3
Vulnerabilidades:
  Gather data sampling:       Unknown: Dependent on hypervisor status
  Indirect target selection:  Mitigation; Aligned branch/return thunks
  Itlb multihit:              KVM: Mitigation: VMX unsupported
  L1tf:                       Mitigation; PTE Inversion
  Mds:                        Mitigation; Clear CPU buffers; SMT Host state unknown
  Meltdown:                   Mitigation; PTI
  Mmio stale data:            Mitigation; Clear CPU buffers; SMT Host state unknown
  Reg file data sampling:     Not affected
  Retbleed:                   Mitigation; IBRS
  Spec rstack overflow:       Not affected
  Spec store bypass:          Mitigation; Speculative Store Bypass disabled via prctl
  Spectre v1:                 Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:                 Mitigation; IBRS; IBPB conditional; STIBP disabled; RSB filling; PBRSB-eIBRS Not affected; BH
                              I SW loop, KVM SW loop
  Srbds:                      Not affected
  Tsa:                        Not affected
  Tsx async abort:            Not affected
  Vmscape:                    Not affected
```

## Tabela de Tempos de Execucao

| N | T1 | T2 | P1 | P2 |
|---|---|---|---|---|
| 2 | 7.252620 segundos | 57.042310 segundos | 7.031661 segundos | 120.289410 segundos |
| 4 | 5.102238 segundos | 62.200252 segundos | 5.155013 segundos | 379.260856 segundos |
| 8 | 4.888641 segundos | 61.825573 segundos | 5.359794 segundos | 369.568808 segundos |

## Analise de Corrupcao

Resultados finais do contador em T1 e P1:

| N | T1 | P1 |
|---|---|---|
| 2 | 533416328 | 520059733 |
| 4 | 324749523 | 339601791 |
| 8 | 313987900 | 334229627 |

Os resultados de T1 e P1 nao conseguiram chegar ao resultado esperado por race condition. Como o contador utilizado era global e nao havia mecanismo de sincronizacao, o sistema operacional nao conseguiu gerenciar adequadamente o acesso ao recurso, fazendo com que threads ou processos sobrescrevessem valores sem contabilizar incrementos anteriores. O Debian Linux usa escalonamento preemptivo, interrompendo processos para alternar a execucao, o que pode aumentar a discrepancia entre o valor esperado e o resultado final. Alem disso, para executar `counter++`, o compilador nao faz apenas a escrita do `+1`: primeiro ocorre a leitura da variavel global.