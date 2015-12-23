# ThreadPool
This is a library that implements a thread pool. Is based on C++11 features. The base code was inspired by https://github.com/nbsdx/ThreadPool. 

## Why I need it.
1. Because I do not want to use the heavy boost-asio, or just asio for just a simple thing that can be implemented with c++11 features.
2. Because I need it in my tests. Creating and destroying threads is very costly, so the solution is to use a thread pool.
 
This is still somewhat a work in progress; it's stable, but there are probably places where can be improved.

## Licensing 
Public Domain. If my licensing is wrong, please let me know. Use at your own risk for whatever you want. Apparently licensing is hard and complicated. If your country doesn't have a public domain, feel free to say you found this on the side of the road. 

## Requirements
`ThreadPool` is based on c++11 features (shared pointers, atomics, mutexes, functors)

