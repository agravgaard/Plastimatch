#include <stdio.h>
#include "itkConditionVariable.h"
#include "itkMultiThreader.h"
#include "itkMutexLock.h"

#if _WIN32
#include <windows.h>
#define plm_sleep(x) Sleep(x)
#else
#include <unistd.h>
#define plm_sleep(x) usleep(1000*x)
#endif

class Thread_struct
{
public:
    itk::SimpleMutexLock mutex;
    itk::ConditionVariable::Pointer condition;
    bool semaphore_available;
    bool die;
public:
    Thread_struct () {
        this->condition = itk::ConditionVariable::New(); 
        this->die = false;
        this->semaphore_available = true;
    }
    ~Thread_struct () {
    }
    void release_semaphore ()
    {
        this->mutex.Lock();
        this->semaphore_available = true;
        this->mutex.Unlock();
        this->condition->Signal ();
    }
    void grab_semaphore ()
    {
        this->mutex.Lock();
        while (this->semaphore_available == false) {
            this->condition->Wait (&mutex);
        }
        this->semaphore_available = false;
        this->mutex.Unlock();
    }
};


ITK_THREAD_RETURN_TYPE 
thread_func (void* param)
{
    itk::MultiThreader::ThreadInfoStruct *info 
        = (itk::MultiThreader::ThreadInfoStruct*) param;
    Thread_struct* ts = (Thread_struct*) info->UserData;

    while (1) {
        ts->grab_semaphore ();
        plm_sleep (300);
        printf ("Child execute\n");
        ts->release_semaphore ();
        if (ts->die) {
            break;
        }
    }
    return ITK_THREAD_RETURN_VALUE;
}

int main ()
{
    Thread_struct ts;

    itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
    int thread_no = threader->SpawnThread (thread_func, (void*) &ts);

    /* Parent and child execute simultaneously */
    for (int i = 0; i < 3; i++) {
        plm_sleep (770);
        printf ("Parent execute\n");
    }

    /* Only parent executes */
    ts.grab_semaphore ();
    printf (">>> Parent only\n");
    for (int i = 0; i < 15; i++) {
        plm_sleep (70);
        printf ("Parent execute\n");
    }
    printf (">>> End parent only\n");
    ts.release_semaphore ();

    /* Parent and child execute simultaneously */
    for (int i = 0; i < 3; i++) {
        plm_sleep (770);
        printf ("Parent execute\n");
    }

    ts.die = true;
    threader->TerminateThread (thread_no);

    return 0;
}
