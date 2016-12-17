//
//  test_scheduler3.cpp
//  Test for scheduling class.  To show off the non-uniform workloads via sleep.
//  This test simlulates a main thread that is doing some work to generate the payloads.
//  This would be like a JPEG decoder doing a HUffman decode in the main thread, and when
//  each block of the image is decoded, then let the non-uniform thread scheduler handle
//  the DCT and color conversion for each block (with a block == a workload).
//
//  Created by ekandrot on 12/16/16.
//

/*
Sample output on an 8 core machine:

---  Time using scheduler  ---
Wall Time = 0.537011
CPU Time  = 0

---  Time using single CPU core  ---
Wall Time = 3.97234
CPU Time  = 0

---  Time using 8 threads  ---
Wall Time = 0.571863
CPU Time  = 0
*/


#include "../scheduler.h"
#include "../ext_timer.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>

/*
build this example code from the command line with:
g++ test_scheduler3.cpp -std=c++14

currently, tested on Windows and Mac.
*/

#define WORKLOADS  80

//-------------------------------------------------------------------------
// the class that has a lot of work to do
struct nonUniformWork : worker {
    std::vector<int> _sleepTimes;
    static thread_local int _locals;
    nonUniformWork() {
    }

    void generate_work(scheduler &s) {
        srand(0);   // assign the seed, so that call code gets the same timings
        for (int i = 0; i < WORKLOADS; ++i) {
            // simulate the time it takes to generate this work for a thread
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // create some non-uniform timings to simulate varying workloads
            _sleepTimes.push_back(rand() % 100);
            // now let the scheduler know it has some work it can do
            s.add_work();
        }
        s.done_adding_work();
    }

    // overriding worker's method
    void do_work(int work) {
        // do work, based on the index variable work
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleepTimes[work]));
    }
};
thread_local int nonUniformWork::_locals = 0;

//-------------------------------------------------------------------------

void scheduler_test() {
    // the code needed to drive the scheduler
    nonUniformWork work;
    scheduler s(&work, 0);
    s.run_wait();
    work.generate_work(s);
    s.join();

}

void single_cpu_test() {
    nonUniformWork work;
    for (int i = 0; i < WORKLOADS; ++i) {
        work.do_work(i);
    }
}


//----------------------------------------------------------------------------------
// code just using std::threads, no locks, breaks workload into 8 equal parts
// by range, not by time.  Based on standard C++ example.
void foo(worker *w, int first, int last) {
    while (first != last) {
        w->do_work(first);
        ++first;
    }
}

void eight_thread_test() {
    nonUniformWork work;

    std::vector<std::thread> threads;
    const int workload_per_core = WORKLOADS / 8;
    for (int i = 0; i<8; ++i) {
        threads.push_back(std::thread(foo, &work, i*workload_per_core, (i+1)*workload_per_core));
    }

    for (int i = 0; i < 8; ++i) {
        threads[i].join();
    }
}
//----------------------------------------------------------------------------------


int main(int argc, char **argv) {
    double wall0, cpu0, wall1, cpu1;

    //--- scheduler timing code ---

    //  Start Timers
    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    scheduler_test();

    //  Stop timers
    wall1 = get_wall_time();
    cpu1 = get_cpu_time();

    std::cout << "---  Time using scheduler  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;
#if 0
    //--- single CPU timing code ---

    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    single_cpu_test();

    //  Stop timers
    wall1 = get_wall_time();
    cpu1 = get_cpu_time();


    std::cout << "---  Time using single CPU core  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;

    //--- fixed work-range, 8 thread timing code ---

    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    eight_thread_test();

    //  Stop timers
    wall1 = get_wall_time();
    cpu1 = get_cpu_time();


    std::cout << "---  Time using 8 threads  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
#endif
    return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
