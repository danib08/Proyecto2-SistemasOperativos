# Server evaluator with various types of processes
Second project for the CE4303 Principles of Operative Systems course.

## Introduction-

## Requirements
- gcc (gnu compiler collection)
- libpng

`sudo apt-get install libpng-dev`

## How to run?

- Run Makefile to compile and link 

`make`

- Run a Client with arguments, where `IP_ADDRESS` is the ip address of the desired server, `PORT` is the logic port of the server, `PATH` is the path of the image to be filtered, `N-THREADS` is the amount of threads that will be requesting to the server and `N-CYCLES` is the amount of times a thread will repeat its request.

`./client IP_ADDRESS PORT PATH N-THREADS N-CYCLES`