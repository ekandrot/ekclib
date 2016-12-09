#include "../scheduler.h"
#include "../ext_timer.h"
#include <iostream>
#include <chrono>
#include <thread>

/*
build this example code from the command line with:
g++ test_scheduler1.cpp -std=c++11

currently, tested on Windows and Mac.
*/


#define MILLISECONDS_PER_ITERATION   50
#define ITERATIONS  80

//-------------------------------------------------------------------------
// the class that has a lot of work to do
struct nonUniformWork : worker {
    void do_work(int work) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MILLISECONDS_PER_ITERATION));
    }
};

//-------------------------------------------------------------------------

void scheduler_test() {
    // the code needed to drive the scheduler
    nonUniformWork work;
    scheduler s(&work, ITERATIONS);
    s.run();
    s.join();

}

void single_cpu_test() {
    nonUniformWork work;
    for (int i = 0; i < ITERATIONS; ++i) {
        work.do_work(i);
    }
}


int main(int argc, char **argv) {

    //--- scheduler timing code ---

    //  Start Timers
    double wall0 = get_wall_time();
    double cpu0 = get_cpu_time();

    scheduler_test();

    //  Stop timers
    double wall1 = get_wall_time();
    double cpu1 = get_cpu_time();

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
