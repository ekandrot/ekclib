//
//  test_scheduler1.cpp
//  Test for scheduling class.  Very basic - just a sleep as the workload.
//
//  Created by ekandrot on 12/09/16.
//

/*
Sample output on an 8 core machine:

---  Time using scheduler  ---
Wall Time = 0.503025
CPU Time  = 0

---  Time using single CPU core  ---
Wall Time = 4.02376
CPU Time  = 0
*/

#include "../scheduler.h"
#include "../ext_timer.h"
#include <iostream>
#include <chrono>
#include <thread>

/*
build this example code from the command line with:
g++ test_scheduler1.cpp -std=c++14

currently, tested on Windows and Mac.
*/


#define MILLISECONDS_PER_WORKLOAD   50
#define WORKLOADS  80

//-------------------------------------------------------------------------
// the class that has a lot of work to do
struct nonUniformWork : worker {
    void do_work(int work) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_PER_WORKLOAD));
    }
};

//-------------------------------------------------------------------------

void scheduler_test() {
    // the code needed to drive the scheduler
    nonUniformWork work;
    scheduler s(&work, WORKLOADS);
    s.run();
    s.join();

}

void single_cpu_test() {
    nonUniformWork work;
    for (int i = 0; i < WORKLOADS; ++i) {
        work.do_work(i);
    }
}


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

    std::cout << "Simple scheduler test that performs uniform waits as 'work'." << std::endl;
    std::cout << "---  Time using scheduler  ---" << std::endl;
    std::cout << "Wall Time = " << wall1 - wall0 << std::endl;
    std::cout << "CPU Time  = " << cpu1 - cpu0 << std::endl;
    std::cout << std::endl;

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

    return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
