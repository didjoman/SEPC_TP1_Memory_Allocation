ensimag-allocator
==================

These files implement a memory-allocator.
This project was an assignment for the course of "Operating System & Concurrent computing" at Ensimag ("Système d'Exploitation et Programmation Concurrente" in french).

All the files are covered by the licence GPLV3+

Introduction
----------

The goal of the course was to explore different ways to implement a physical memory allocation.
In this assignment, we implemented the Buddy algorithm (see http://en.wikipedia.org/wiki/Buddy_memory_allocation).
A modified version of this algorithm is also used in the Linux Kernel.

How to build the project
----------

> `cd SEPC_TP1_Memory_Allocation`

> `cd build`

> `cmake ..`

> `make`

Usage
----------
In order to test the memory allocator implemented, we can use the provided shell (memshell), launch the command `alloc <size>` and then observe the data structure (named TZL) in gdb : 


> `cd SEPC_TP1_Memory_Allocation`

> `cd build`

> `gdb ./memshell`

> `break`

> `mem_alloc`

> `layout`

> `run`

> `alloc 64`

> `print TZL`


Tests
----------
Unit-tests (Google tests) have been provided by the professors to check the validity of the memory allocation.
In order to launch it : 

> `cd SEPC_TP1_Memory_Allocation`

> `cd build`

> `make test`

> `make check`
