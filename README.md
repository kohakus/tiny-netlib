**A tiny network library**

This project is created for learning purpose.

A network library or server project is often very helpful for 
learning and practice, for the basic knowledge of network programming, operating system and programming language
can be integrated together, which is also the primary motivation of this project.

**The Model**

- NIO (Non-blocking IO) + one (event) loop per thread
- Reactor and event driven (implemented by linux epoll)

**Technical summary**

- RAII resources management
- Stream style log front-end,  which is implemented by {fmt} wrapper that guarantee the type safe
- Asynchronous multi-thread log based on buffer swapping strategy
- Log file rolling strategy that consider both day time and log file size
- The "work-thread-transfer" strategy for transferring specific tasks among threads to avoid locking
- Efficient timer manager implemented by binary heap structure without redundant memorization
- Unified event source management by using timerfd and eventfd

**Main references:**

- [Muduo](https://github.com/chenshuo/muduo) network library
- [Book series written by W. Richard Stevens](http://www.kohala.com/start/#books)
- [The Linux Programming Interface (TLPI)](https://man7.org/tlpi/) written by Michael Kerrisk