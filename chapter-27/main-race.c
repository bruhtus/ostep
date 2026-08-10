#include <assert.h>
#include <pthread.h>

#define Pthread_create(thread, attr, start_routine, arg) \
	assert(pthread_create(thread, attr, start_routine, arg) == 0)

#define Pthread_join(thread, value_ptr) \
	assert(pthread_join(thread, value_ptr) == 0);

#define Pthread_mutex_lock(lock) \
	assert(pthread_mutex_lock(lock) == 0)

#define Pthread_mutex_unlock(lock) \
	assert(pthread_mutex_unlock(lock) == 0)

static int balance = 0;

static void *worker(void *arg) {
    pthread_mutex_t *lock = arg;

    Pthread_mutex_lock(lock);
    balance++; // unprotected access
    Pthread_mutex_unlock(lock);

    return NULL;
}

/*
 * Check with this command (with or without lock):
 * valgrind --tool=helgrind ./main-race
 *
 * Documentation:
 * https://valgrind.org/docs/manual/hg-manual.html
 */
int main(int argc, char *argv[]) {
    pthread_t p;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    Pthread_create(&p, NULL, worker, &lock);

    Pthread_mutex_lock(&lock);
    balance++; // unprotected access
    Pthread_mutex_unlock(&lock);

    Pthread_join(p, NULL);

    return 0;
}
