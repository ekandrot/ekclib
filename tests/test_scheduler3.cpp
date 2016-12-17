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
Sample output on an 8 core machine (with __DEBUG on):

Thread 3 called:  8
Thread 0 called:  6
Thread 6 called:  3
Thread 7 called:  4
Thread 2 called:  6
Thread 5 called:  5
Thread 4 called:  4
Thread 1 called:  4
---  standard scheduler (zero overhead)  ---
Wall Time = 0.291238
CPU Time  = 0

Thread 1 called:  5
Thread 0 called:  5
Thread 5 called:  4
Thread 6 called:  3
Thread 7 called:  5
Thread 2 called:  5
Thread 4 called:  4
Thread 3 called:  9
---  1 millisecond to per workload to generate  ---
Wall Time = 0.299269
CPU Time  = 0

Thread 3 called:  5
Thread 4 called:  5
Thread 5 called:  4
Thread 0 called:  5
Thread 6 called:  4
Thread 7 called:  4
Thread 1 called:  8
Thread 2 called:  5
---  10 millisecond to per workload to generate  ---
Wall Time = 0.474617
CPU Time  = 0

Thread 2 called:  5
Thread 5 called:  5
Thread 0 called:  5
Thread 6 called:  5
Thread 3 called:  5
Thread 4 called:  5
Thread 7 called:  5
Thread 1 called:  5
---  100 millisecond to per workload to generate  ---
Wall Time = 4.0957
CPU Time  = 0


We should see something like the above when testing.  It shows that with 1 ms and baseline scheduler,
there is almost no difference in timings, even though there is 1ms x 40 overhead; overhead is covered
by the threads doing work.
With the 10 ms overhead, this is masking the much of the threads work, because
10 x 40 ms = 0.400 seconds, then + 0.290 of thread work = .690 seconds, but we see less than
that, implying that the masking is working.
In the case of 100 ms overhead, it is much longer than any of the workloads, so it will be the same
speed as if it were single threaded 40 x 100 ms = 4.0 seconds.
With the _DEBUG numbers turned on, we can see the distribution of WORKLOAD per thread.  Since the
workloads are non-uniform, when there is little overhead, the thread calls are uneven.  But as we
increase the overhead, the thread calls become more uniform.
*/

#include "../scheduler.h"
#include "../ext_timer.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>


#define WORKLOADS  40

//-------------------------------------------------------------------------
// the class that has a lot of work to do
struct nonUniformWork : worker {
    std::vector<int> _sleepTimes;
    nonUniformWork() {
        // create some non-uniform timings to simulate varying workloads
        srand(0);   // assign the seed, so that call code gets the same timings
        for (int i = 0; i < WORKLOADS; ++i) {
            _sleepTimes.push_back(rand() % 100);
        }
    }

    void do_work(int work) {
        // do work, based on the index variable work
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleepTimes[work]));
    }
};


//-------------------------------------------------------------------------
// the new class that has a lot of work to do, and takes time to generate it
struct nonUniformWork_wait : worker {
    std::vector<int> _sleepTimes;
    static thread_local int _locals;
    nonUniformWork_wait() {
    }

    void generate_work(scheduler &s, int timeToGenerate) {
        srand(0);   // assign the seed, so that call code gets the same timings
        for (int i = 0; i < WORKLOADS; ++i) {
            // simulate the time it takes to generate this work for a thread
            std::this_thread::sleep_for(std::chrono::milliseconds(timeToGenerate));
            // create some non-uniform timings to simulate varying workloads
            _sleepTimes.push_back(rand() % 100);
            // now let the scheduler know it has some work it can do
            s.add_work();
        }
    }

    // overriding worker's method
    void do_work(int work) {
        // do work, based on the index variable work
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleepTimes[work]));
    }
};
thread_local int nonUniformWork_wait::_locals = 0;

//-------------------------------------------------------------------------

void scheduler_test() {
    // the code needed to drive the scheduler
    nonUniformWork work;
    scheduler s(&work, WORKLOADS);
    s.run();
    s.join();
}

void scheduler_wait_test(int timeToGenerate) {
    // the code needed to drive the scheduler
    nonUniformWork_wait work;
    scheduler s(&work, 0);
    s.run();
    work.generate_work(s, timeToGenerate);
    s.join();
}

//-------------------------------------------------------------------------

int main(int argc, char **argv) {
    double wall0, cpu0, wall1, cpu1;

    //  Start Timers
    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    scheduler_test();

    wall1 = get_wall_time();
    cpu1 = get_cpu_time();

    std::cout << "---  standard scheduler (zero overhead)  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;


    //  Start Timers
    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    scheduler_wait_test(1);

    wall1 = get_wall_time();
    cpu1 = get_cpu_time();

    std::cout << "---  1 millisecond to per workload to generate  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;


    //  Start Timers
    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    scheduler_wait_test(10);

    wall1 = get_wall_time();
    cpu1 = get_cpu_time();

    std::cout << "---  10 millisecond to per workload to generate  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;


    //  Start Timers
    wall0 = get_wall_time();
    cpu0 = get_cpu_time();

    scheduler_wait_test(100);

    wall1 = get_wall_time();
    cpu1 = get_cpu_time();

    std::cout << "---  100 millisecond to per workload to generate  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;

    return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
