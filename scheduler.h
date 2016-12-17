//
//  scheduler.h
//  Base class for certain types of threading code.
//  Loosely based on CUDA
//  http://developer.download.nvidia.com/compute/cuda/docs/CUDA_Architecture_Overview.pdf
//
//  Created by ekandrot on 11/14/16.
//
//

#define __DEBUG__

#ifndef scheduler_h
#define scheduler_h

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#ifdef __DEBUG__
#include <iostream>
#endif

// inherent from this, override do_work() with your own method
struct worker {
    virtual void do_work(int work) =0;
};


struct scheduler {

    scheduler(worker *w, int maxWork, int threadCount=0) : _maxWork(maxWork), _nextWork(0), _w(w) {
        // we initialize with the number of threads hardware says we have
        if (threadCount < 1) {
            _threadCount = std::thread::hardware_concurrency();
        }
        _done = false;  // used by wait
    }

    void run_wait() {
        run_all_threads(&code_block_wait);
    }

    void add_work() {
        ++_maxWork;
        _cv.notify_one();
    }

    void done_adding_work() {
        _done = true;
        _cv.notify_all();
    }

    void run() {
        run_all_threads(&code_block);
    }

    // wait for all of the threads to claim they have no more work.
    void join() {
        for (auto& th : _threads) th.join();
        _threads.clear();   // clear away the threads now that we are done with them
    }

private:

    // internal private variables
    int _maxWork;   // the last of the range of work indexes
    int _nextWork;  // the next free index of work, shared access by all threads
    worker *_w;     // pointer to the class that implements do_work()
    std::mutex _workMutex;  // a shared mutex to lock access to _nextWork between threads
    std::vector<std::thread> _threads;  // the list of threads that are running
    int _threadCount;   // number of threads run() should create
#ifdef __DEBUG__
    std::mutex _printMutex;  // a shared mutex for std::cout usage
    static thread_local int _callCount;     // per thread, number of times this thread was used
#endif


    // resets the starting work load index.  Then creates a number of threads to do work.
    void run_all_threads( void (*x)(int threadID, scheduler *t, worker *w) ) {
        _threads.clear();   // clear away any old threads stored from possible previous invocations
        _nextWork = 0;   // reset so we can have multiple, sequential runs per object
        for (int i = 0; i<_threadCount; ++i) {
            _threads.push_back(std::thread(x, i, this, _w));
        }
    }

    // this function has a lock around a work index counter.  As a thread
    // requests more work, update the index and return it to the code_block to
    // pass to the actual worker method.
    // returns -1 when there are no more indexes of work.
    // is called by multiple threads of code_block.
    int get_work() {
        std::lock_guard<std::mutex> lk(_workMutex);
        int retVal = _nextWork;
        if (retVal < _maxWork) {
            ++_nextWork;
            #ifdef __DEBUG__
            ++_callCount;
            #endif
        } else {
            retVal = -1;
        }
        return retVal;
    }

    // this the is function that is actually called by std::thread
    // it gets the next index of work, if it is -1 then this thread is done.
    // Otherwise, it invokes the do_work method with that work index.
    // Repeat until this thread has no more work to do from the pool.
    // threadID is used for debugging.
    static void code_block(int threadID, scheduler *t, worker *w) {
        while (true) {
            int work = t->get_work();

            if (work == -1) {
#ifdef __DEBUG__
                std::lock_guard<std::mutex> lk(t->_printMutex);
                std::cout << "Thread " << threadID << " called:  " << _callCount << std::endl;
#endif
                return;
            }
            w->do_work(work);
        }
    }


    std::condition_variable _cv;
    bool _done;

    int get_work_wait() {
        std::unique_lock<std::mutex> lk(_workMutex);
        _cv.wait(lk, [this] {return _done || (_nextWork < _maxWork);});
        int retVal = _nextWork;
        if (retVal < _maxWork) {
            ++_nextWork;
#ifdef __DEBUG__
            ++_callCount;
#endif
        } else {
            retVal = -1;
        }
        return retVal;
    }


    static void code_block_wait(int threadID, scheduler *t, worker *w) {
        while (true) {
            int work = t->get_work_wait();

            if (work == -1) {
#ifdef __DEBUG__
                std::lock_guard<std::mutex> lk(t->_printMutex);
                std::cout << "Thread " << threadID << " called:  " << _callCount << std::endl;
#endif
                return;
            }
            w->do_work(work);
        }
    }
};

#ifdef __DEBUG__
thread_local int scheduler::_callCount = 0;
#endif

#endif /* scheduler_h */
