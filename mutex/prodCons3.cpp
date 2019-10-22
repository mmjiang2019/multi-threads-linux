#include <iostream>
#include <pthread.h>

#define NUM_ITEMS 10000
#define NUM_THREADS 100

using namespace std;

struct buffData
{
    pthread_mutex_t mutex;
    int buff[NUM_ITEMS];
    int  nPut;
    int nVal;
};

int nitems = 0;
buffData shared = { PTHREAD_MUTEX_INITIALIZER, };

// Declarations
void *produce(void *args);
void *consume(void *args);

int main()
{
    int i = 0;
    int nthreads = 0;
    int count[NUM_ITEMS] = { 0 };

    pthread_t tid_produce[NUM_THREADS] = { 0 };
    pthread_t tid_consume = { 0 };

    std::cout << "Please input the number of items: " << std::endl;
    std::cin >> nitems;
    nitems = nitems >= NUM_ITEMS ? NUM_ITEMS : nitems;
    std::cout << "Please input the number of threads: " << std::endl;
    std::cin >> nthreads;
    nthreads = nthreads >= NUM_THREADS ? NUM_THREADS : nthreads;

    // Set the concurrency level
    pthread_set_concurrency(nthreads + 1);

    // Start the produce threads
    for (int i = 0; i < nthreads; i++)
    {
        count[i] = 0;
        pthread_create(&tid_produce[i], nullptr, produce, &count[i]);
    }
    
    // Start the consumer thread
    pthread_create(&tid_consume, nullptr, consume, nullptr);

    // Wait for all producer threads
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(tid_produce[i], nullptr);
        std::cout << "count[" << i << "] = " << count[i] << std::endl;
    }

    // Wait for the consumer thread
    pthread_join(tid_consume, nullptr);
}

void *produce(void *args)
{
    for ( ; ; ) // infinite loop to check
    {
        pthread_mutex_lock(&shared.mutex);
        if (shared.nPut >= nitems)
        {
            pthread_mutex_unlock(&shared.mutex);
            return nullptr;
        }

        shared.buff[shared.nPut] = shared.nVal;
        shared.nPut++;
        shared.nVal++;

        pthread_mutex_unlock(&shared.mutex);

        *((int *) args) += 1;
    }
}

void consume_wait(int i)
{
    for ( ; ; ) // infinite loop to spinning or polling
    {
        pthread_mutex_lock(&shared.mutex);
        if (i < shared.nPut)
        {
            pthread_mutex_unlock(&shared.mutex);
            return;
        }
        pthread_mutex_unlock(&shared.mutex);
    }
}

void *consume(void *args)
{
    int i = 0;
    for (; i < nitems; i++)
    {
        // wait for the conditions
        consume_wait(i);

        if (shared.buff[i] != i)
        {
            std::cout << "buff[" << i << "] = " << shared.buff[i] << std::endl;
        }
    }

    return nullptr;
}