#ifndef __DATA_THREAD_USAGE_
#define __DATA_THREAD_USAGE_

#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/resource.h>
#include <time.h>
#include <stdio.h>

sem_t work_done;

pthread_t target_t;

#define init_signal  sem_init(&work_done, 0, 0)

#define post_signal  sem_post(&work_done)

#define go_work(func) pthread_create(&target_t, NULL, (void*)func, NULL)

#define end_work pthread_join(target_t, NULL)

static long miliseconds = 1;


static inline void thread_record_dmemory(void){
    printf("thread in\n");
    FILE *ptr = NULL;
    ptr = fopen("memory.txt", "w");
    if (!ptr) return;

    struct rusage resource_m;
    struct timespec req, rem;
    if (miliseconds > 999) {   
        req.tv_sec = (int)(miliseconds / 1000);                            /* Must be Non-Negative */
        req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000; /* Must be in range of 0 to 999999999 */
    }else{   
        req.tv_sec = 0;                         /* Must be Non-Negative */
        req.tv_nsec = miliseconds * 1000000;    /* Must be in range of 0 to 999999999 */
    }  
    do {
        getrusage(RUSAGE_SELF, &resource_m);
        fprintf(ptr, "user time %ld (ms), memory %ld (KB), page fault count %ld \n", resource_m.ru_utime.tv_usec, resource_m.ru_maxrss, resource_m.ru_majflt);
        //nanosleep(&req, &rem);
    } while (sem_trywait(&work_done) != 0);
    fclose(ptr);
}
#endif /*__DATA_THREAD_USAGE_*/