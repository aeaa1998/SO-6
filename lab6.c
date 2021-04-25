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

struct rk_sema check_value, check_pass;
#define THREAD_NUM 100
#define THREAD_ITER_NUM 5
int shared_resources = 5000000;
pthread_t threads[THREAD_NUM];

void *mutate_resources(void *vargp)
{

    pthread_t ptid = pthread_self();

    printf("Iniciando threads %ld \n", (long)ptid);
    int it_num;
    for (it_num = 0; it_num < THREAD_ITER_NUM; it_num++)
    {
        printf("%ld - Iniciando iteracion %d\n", (long)ptid, it_num + 1);
        // rk_sema_wait(&check_pass);
        // if (shared_resources <= 1)
        // {
        rk_sema_wait(&check_value);
        // }
        // rk_sema_post(&check_pass);
        shared_resources--;
        printf("Recurso tomado\n");
        //Pretends to do some work
        sleep(rand() % 2);
        printf("Buenos dias recurso usado :)\n");
        shared_resources++;
        printf("Recurso retornado\n");
        rk_sema_post(&check_value);
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
    rk_sema_init(&check_value, 1);
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
