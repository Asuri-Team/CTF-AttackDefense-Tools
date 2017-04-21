#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define KEY 61634
#define SHM_SIZE 128
#define PROCESS_CNT 3

int make_daemon();
void sig_ignore(int);
void clear_shs();
void shmutex_init();
void shmutex_lock();
void shmutex_unlock();
void child_do();

int shmid, semid;
struct shmid_ds shm_ds;
struct semid_ds sem_ds;
struct sembuf sem_b;
int  *shm_ptr;
int main(void) {
    //make_daemon();
    pid_t pid, ppid;
    int i, psid;
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    clear_shs();
    
    shmid = shmget(KEY, SHM_SIZE, IPC_CREAT | 0600);
    if ( shmid < 0 ){
        perror("shmget() - ");
        exit(-1);
    }
    semid = semget(KEY, 1, IPC_CREAT | 0600);
    if (semid < 0){
        perror("semget() - ");
        exit(-1);
    }
    shmutex_init();
    shm_ptr = (int *)shmat(shmid, NULL, 0);
    if(!shm_ptr){
        perror("shmat() - ");
    }
    puts("try lock");
    shmutex_lock();
    puts("locked");
    shm_ptr[0] = PROCESS_CNT; 
    shm_ptr[1] = getpid();
    for(i=0; i<PROCESS_CNT - 1; ++i) {
        ppid = fork();
        if (ppid == -1) {
            perror("fork() - ");
            exit(-1);
        } else if (ppid == 0) {
            puts("child process.");
            psid = setsid();
            if(psid == -1) {
                perror("setsid() - ");
                exit(-1);
            }
            goto CHILD;
        } else {
            puts("save_ppid");
            shm_ptr[i + 2] = ppid;
        }
    }
    // for(i=0; i<128/4; ++i) {
    //     printf("%d\n", shm_ptr[i]);
    // }
    shmutex_unlock();
CHILD:
    printf("process %d go child\n", getpid());
    while(1) {
        child_do();
    }
    return 0;
}

void *thread_loop(void *a){
    while(1) {
        sleep(1);
    }
}

void child_do() {
    pthread_t thread;
    int i, status, psid;
    pthread_create(&thread, NULL, thread_loop, NULL);
    shmutex_lock();
    printf("total_cnt: %d\n", shm_ptr[0]);
    while(1){
        for (i = 1; i<= shm_ptr[0]; ++i) {
            status = kill(shm_ptr[i], 0);
            //if (!status) printf("process %d check %d: %d\n", getpid(), shm_ptr[i], status);
            if (status != 0) {
                status = fork();
                if(status == -1) {
                    perror("fork()");
                    exit(-1);
                } else if (status == 0){
                    psid = setsid();
                    if(psid == -1) {
                        perror("setsid()");
                        exit(-1);
                    }
                    return;
                } else {
                    shm_ptr[i] = status;
                }
            }
        }
    }
}

void shmutex_init() {
    union semun op;
    op.val = 1;
    if(semctl(semid, 0, SETVAL, op) == -1) {
        perror("semctl() - SETVAL");
    }
}

void shmutex_lock() {
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = 0;
    if(!semop(semid, &sem_b, 1)){
        perror("semop() - lock");
    };
    printf("Process %d got mutex\n", getpid());
}

void shmutex_unlock() {
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = 0;
    if(!semop(semid, &sem_b, 1)){
        perror("semop() - unlock");
    };
    printf("Process %d release mutex\n", getpid());
}

int make_daemon() {
    pid_t ppid,psid;
    ppid = fork();
    if(ppid < 0) {
        perror("fork()");
        exit(-1);
    } else if(ppid == 0) {
        psid = setsid();
        if(psid == -1) {
            perror("setsid()");
            exit(-1);
        }
        return 0;
    }
    exit(EXIT_SUCCESS);
    return -1;
}

void clear_shs(){
    shmctl(KEY, IPC_RMID, &shm_ds);
    semctl(KEY, 0, IPC_RMID);
}
