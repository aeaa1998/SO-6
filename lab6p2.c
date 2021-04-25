#include <pthread.h>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct rk_sema
{
#ifdef __APPLE__
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
};
static inline void
rk_sema_init(struct rk_sema *s, uint32_t value)
{
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s->sem, 0, value);
#endif
}

static inline void
rk_sema_wait(struct rk_sema *s)
{

#ifdef __APPLE__
    dispatch_semaphore_wait(s->sem, DISPATCH_TIME_FOREVER);
#else
    int r;
    do
    {
        r = sem_wait(&s->sem);
    } while (r == -1 && errno == EINTR);
#endif
}

static inline void
rk_sema_post(struct rk_sema *s)
{
#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
#else
    sem_post(&s->sem);
#endif
}

struct rk_sema mutex, check_pass;
#define THREAD_NUM 100
#define THREAD_ITER_NUM 5
int shared_resources = 5;
pthread_t threads[THREAD_NUM];

void *mutate_resources(void *vargp)
{
    int it_num;
    for (it_num = 0; it_num < THREAD_ITER_NUM; it_num++)
    {
        rk_sema_wait(&check_pass);
        int copy = shared_resources - 1;
        if (copy <= 0)
        {
            rk_sema_wait(&mutex);
        }
        shared_resources--;
        rk_sema_post(&check_pass);
        printf("El valor actual de recursos %d \n", shared_resources);
        sleep(rand() % 2);
        shared_resources++;
        if (shared_resources > 0 && copy <= 0)
        {
            rk_sema_post(&mutex);
        }
    }

    return NULL;
}

void init_threads()
{
    int i;
    for (i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&threads[i], NULL, mutate_resources, NULL);
    }
}

int main()
{
    rk_sema_init(&mutex, 1);
    rk_sema_init(&check_pass, 1);
    init_threads();
    int index_thread;
    for (index_thread = 0; index_thread < THREAD_NUM; index_thread++)
    {
        pthread_join(threads[index_thread], NULL);
    }
    printf("END\n");
    return 0;
}
