//
//  scheduler.h
//  rayexp
//
//  Created by ekandrot on 11/14/16.
//
//

#ifndef scheduler_h
#define scheduler_h

#include <thread>

struct worker {
    virtual void do_work(int work) =0;
};

class scheduler {
    int maxWork;
    int nextWork;
    worker *w;
    std::mutex workMutex;
    std::vector<std::thread> threads;
    int threadCount;
    
public:
    scheduler(worker *s, int mw) : maxWork(mw), nextWork(0), w(s) {
        threadCount = std::thread::hardware_concurrency();
    }
    void run() {
        nextWork = 0;   // reset so we can have multiple, sequential runs per object
        for (int i=0; i<threadCount; ++i) {
            threads.push_back(std::thread(&code_block, this, w));
        }

    }

    void join() {
        for (auto& th : threads) th.join();
    }
    
    int get_work() {
        std::lock_guard<std::mutex> lk(workMutex);
        int retVal = nextWork;
        if (retVal < maxWork) {
            nextWork += 1;
        } else {
            retVal = -1;
        }
        return retVal;

    }

    static void code_block(scheduler *t, worker *s) {
        while (true) {
            int work = t->get_work();
            
            if (work == -1) {
                return;
            }
            s->do_work(work);
        }

    }
};

#endif /* scheduler_h */
