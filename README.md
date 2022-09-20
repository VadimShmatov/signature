# signature
## Task details
[Task C++, Signature.pdf](https://github.com/VadimShmatov/signature/files/9605535/Task.C%2B%2B.Signature.pdf)

## Solution
This repository contains multi-threaded crossplatform (Windows & Linux) application for signature generation.
![Signature](https://user-images.githubusercontent.com/8525363/191213101-cbfce725-94ca-4fcb-85b5-fb2187c86dd5.png)

## Building project
This project depends on Boost dynamic libraries, make sure they are installed before building/running.

On Windows, open project directory in Visual Studio, generate build files with CMake and build the solution.

On Linux, build using cmake and make: `mkdir build && cd build && cmake .. && make`

## Running project
Usage:
```
signature input_file output_file [block_size_bytes (default value: 1 Mb)]
```
Example:
```
signature input.bin output.txt 512
```
