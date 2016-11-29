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

class raytrace : worker {
  void do_work(int work) {
    // do whatever work is needed for pixel row "work"
    // write it to _bitmap
  }
  Bitmap  _bitmap;
}

main() {
  raytrace ray;
  scheduler s(ray, 1024);  // for a 1024 row image
  s.run();
  s.join();
  // ray._bitmap is now filled in, write it to disk, show it, etc
}
```
