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

    scheduler(worker *w, int maxWork, int threadCount=0) : _maxWork(maxWork), _nextWork(0), _threadCount(threadCount), _w(w) {
        if (_threadCount < 1) {
            // initialize with the number of threads hardware says we have
            _threadCount = std::thread::hardware_concurrency();
        }
        _doneAddingWork = false;  // used by wait
    }

    void add_work() {
        ++_maxWork;
        _cv.notify_one();
    }

    // resets the starting work load index.  Then creates a number of threads to do work.
    void run() {
        _threads.clear();   // clear away any old threads stored from possible previous invocations
        _nextWork = 0;   // reset so we can have multiple, sequential runs per object
        for (int i = 0; i<_threadCount; ++i) {
            _threads.push_back(std::thread(code_block, i, this, _w));
        }
    }

    // wait for all of the threads to claim they have no more work.
    void join() {
        _doneAddingWork = true;
        _cv.notify_all();
        for (auto& th : _threads) th.join();
        _threads.clear();   // clear away the threads now that we are done with them
    }

private:

    // internal private variables
    int _maxWork;   // the last of the range of work indexes
    int _nextWork;  // the next free index of work, shared access by all threads
    int _threadCount;   // number of threads run() should create
    worker *_w;     // pointer to the class that implements do_work()
    std::vector<std::thread> _threads;  // the list of threads that are running
    std::mutex _workMutex;  // a shared mutex to lock access to _nextWork between threads
    std::condition_variable _cv;    // allows for the addition of workloads to the queue, works with _workMutex and _doneAddingWork to make that happen
    bool _doneAddingWork;   // tells scheduler that the client is done adding work, just wait out the queue now
#ifdef __DEBUG__
    std::mutex _printMutex;  // a shared mutex for std::cout usage
    static thread_local int _callCount;     // per thread, number of times this thread was used
#endif



    // this function has a lock around a work index counter.  As a thread
    // requests more work, update the index and return it to the code_block to
    // pass to the actual worker method.
    // returns -1 when there are no more indexes of work.
    // is called by multiple threads of code_block.
    int get_work() {
        std::unique_lock<std::mutex> lk(_workMutex);
        _cv.wait(lk, [this] {return _doneAddingWork || (_nextWork < _maxWork);});
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
};

#ifdef __DEBUG__
thread_local int scheduler::_callCount = 0;
#endif

#endif /* scheduler_h */
