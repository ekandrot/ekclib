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

struct worker {
    virtual void do_work(int work) =0;
};

class scheduler {
    int _maxWork;
    int _nextWork;
    worker *_w;
    std::mutex _workMutex;
    std::vector<std::thread> _threads;
    int _threadCount;
    
public:
    scheduler(worker *w, int maxWork) : _maxWork(maxWork), _nextWork(0), _w(w) {
        _threadCount = std::thread::hardware_concurrency();
    }
    void run() {
        _nextWork = 0;   // reset so we can have multiple, sequential runs per object
        for (int i=0; i<_threadCount; ++i) {
            _threads.push_back(std::thread(&code_block, this, _w));
        }

    }

    void join() {
        for (auto& th : _threads) th.join();
    }
    
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
