# ekclib
a simple library for classes, functions, and algorithms that I use in multiple projects.

## scheduler.h
This header contains threading code - it makes it easy to multi-thread certain workflows.  Current design is based
on CUDA, creates and passes thread_idx equivalent.   Uses C++ threads, creating a pool equal to the number of CPUs.<br>
To use, include it.  Then derive your base class from worker.  Override the do_work method.
Create a scheduler object with a pointer to your class and the amount of work.  run the class, then join
which will complete when the work is done.
Your do_work will be called with an integer, which tells it which block of work to do for that invocation. The
scheduler class handles creating threads, etc.  Your code just has to map that int to what needs to be done (ex rows in
an image, files to be processes, etc).

Example:
```
#include "scheduler.h"

struct raytrace : worker {
  void do_work(int work) {
    // do whatever work is needed for pixel row "work"
    // write it to _bitmap
  }
  Bitmap  _bitmap;
}

main() {
  raytrace ray;
  scheduler s(&ray, 1024);  // for a 1024 row image
  s.run();
  s.join();
  // ray._bitmap is now filled in, write it to disk, show it, etc
}
```

## tests
### test_scheduler1
This is a test for the scheduler and only does a wait as its workload.  It is a 
performance test to show that the threading is working.  It has 80 work-units 
that take 50 ms each.  On an 8 core machine, it should take 0.5 seconds 
walltime, and 4.0 seconds walltime on a single thread.  The results on my 
machine are as follows:

```
---  Time using scheduler  ---
Wall Time = 0.503025
CPU Time  = 0

---  Time using single CPU core  ---
Wall Time = 4.02376
CPU Time  = 0
```
