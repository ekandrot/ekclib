//
//  scheduler.h
//  Base class for certain types of threading code.
//  Loosely based on CUDA
//  http://developer.download.nvidia.com/compute/cuda/docs/CUDA_Architecture_Overview.pdf
//
//  Created by ekandrot on 11/14/16.
//
//

#ifndef scheduler_h
#define scheduler_h

#include <thread>
#include <mutex>
#include <vector>

// inherent from this, override do_work() with your own method
struct worker {
    virtual void do_work(int work) =0;
};


class scheduler {
    // internal private variables
    int _maxWork;   // the last of the range of work indexes
    int _nextWork;  // the next free index of work, shared access by all threads
    worker *_w;     // pointer to the class that implements do_work()
    std::mutex _workMutex;  // a shared mutex to lock access to _nextWork between threads
    std::vector<std::thread> _threads;  // the list of threads that are running
    int _threadCount;   // number of threads run() should create
    
public:
    scheduler(worker *w, int maxWork) : _maxWork(maxWork), _nextWork(0), _w(w) {
        // we initialize with the number of threads hardware says we have
        // could create an accessor in case someone needs to override that number
        _threadCount = std::thread::hardware_concurrency();
    }

    // resets the starting work load index.  Then creates a number of threads to do work.
    void run() {
        _threads.clear();   // clear away any old threads stored from possible previous invocations
        _nextWork = 0;   // reset so we can have multiple, sequential runs per object
        for (int i=0; i<_threadCount; ++i) {
            _threads.push_back(std::thread(&code_block, this, _w));
        }
    }

    // wait for all of the threads to claim they have no more work.
    void join() {
        for (auto& th : _threads) th.join();
        _threads.clear();   // clear away the threads now that we are done with them
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
            _nextWork += 1;
        } else {
            retVal = -1;
        }
        return retVal;
    }

    // this the is function that is actually called by std::thread
    // it gets the next index of work, if it is -1 then this thread is done.
    // Otherwise, it invokes the do_work method with that work index.
    // Repeat until this thread has no more work to do from the pool.
    static void code_block(scheduler *t, worker *w) {
        while (true) {
            int work = t->get_work();
            
            if (work == -1) {
                return;
            }
            w->do_work(work);
        }
    }
};

#endif /* scheduler_h */
