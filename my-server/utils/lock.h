#ifndef UTILS_MYLOCK_H
#define UTILS_MYLOCK_H

#include <exception>
#include <semaphore.h>
//互斥锁
#include <pthread.h>

class Lock{
public:
    Lock(){
        pthread_mutex_init(&mutex, NULL);//普通锁
    }
    ~Lock(){
        pthread_mutex_destroy(&mutex);
    }
    bool lock(){
        return pthread_mutex_lock(&mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&mutex) == 0;
    }

private:
    pthread_mutex_t mutex;
};

class LockGuard{
public:
    LockGuard(Lock *lk): lock(lk){
        lock->lock();
    }
    ~LockGuard(){
        lock->unlock();
    }
    
private:
    Lock *lock;
};



//条件变量-linux c
class Cond{
public:
    Cond(){
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }
    ~Cond(){
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    //wait
    bool wait(){
        pthread_mutex_lock(&mutex);
        int ret = pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
        return ret == 0;
    }
    //唤醒
    bool signal(){
        return pthread_cond_signal(&cond);
    }
private:
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};

//条件变量-c++
//实现类信号量机制
#include <mutex>
#include <condition_variable>
class Semaphore{
public:
    Semaphore(int cnt = 0):count(cnt){}
    void wait(){
        std::unique_lock<std::mutex> unique(mt);
        --count;
        if(count < 0){
            cond.wait(unique);
        }
    }
    void signal(){
        std::unique_lock<std::mutex> unique(mt);
        ++count;
        if(count<=0){
            cond.notify_one();
        }
    }
private:
    std::mutex mt;
    std::condition_variable cond;
    int count;
};
#endif