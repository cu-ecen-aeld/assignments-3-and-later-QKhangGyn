#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
// #define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data *thread_data_ptr = (struct thread_data*)thread_param;
    unsigned int wait_to_obtain_ms = thread_data_ptr->wait_to_obtain_ms;
    unsigned int wait_to_release_ms = thread_data_ptr->wait_to_release_ms;
    int ret_val = -1;
    // Wait for a period before obtaining a mutex
    usleep(wait_to_obtain_ms*1000);
    // Lock the mutex
    pthread_mutex_lock(&thread_data_ptr->mutex);
    // Wait for a period to release a mutex
    usleep(wait_to_release_ms);
    //release the mutex
    ret_val = pthread_mutex_unlock(&thread_data_ptr->mutex);
    if(ret_val == 0) thread_data_ptr->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    // Initialize variable
    int return_value = 0;
    pthread_t thread_id;
    bool result = false;
    // Allocate memory for thread_data
    struct thread_data *thread_data_ptr = NULL;
    thread_data_ptr = (struct thread_data*) malloc(sizeof(struct thread_data));
    if(thread_data_ptr == NULL){
        ERROR_LOG("Fail to allocate memory");
        return false;
    }
    // Setup mutex and wait argument
    thread_data_ptr->wait_to_obtain_ms = (unsigned int) wait_to_obtain_ms;
    thread_data_ptr->wait_to_release_ms = (unsigned int) wait_to_release_ms;
    thread_data_ptr->thread_complete_success = false;
    pthread_mutex_init(&thread_data_ptr->mutex,NULL);
    // Handle the mutex
    return_value = pthread_create(&thread_id, NULL, threadfunc, (void*)thread_data_ptr);
    if(return_value){ //error
        ERROR_LOG("Cannot create a thread");
    } else {
        //Joining the thread
        pthread_join(thread_id, NULL);
    }
    // Get the result
    result = thread_data_ptr->thread_complete_success;
    //Destroy the mutex
    pthread_mutex_destroy(&thread_data_ptr->mutex);
    // Deallocate the memory
    free(thread_data_ptr);
    thread_data_ptr = NULL;

    return result;
}

