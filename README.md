# App-CV

## Description

App-CV is an OpenCV based application used as the brain of the Airhockey project. The main feature of App-CV is to recognize a puck in an Air Hockey table, predict its trajectory and command a robot to move to a calculed position so the puck would be hit, avoiding a goal. Currently, the algorithm uses OpenCV `findContours()` to identify the puck and calculate the remaining problem variables.

## Instructions

To execute, go to the project folder that contains a `CMakeLists.txt` and run:

```sh
cmake .
make
```

A binary with the name of the project should have been created. To execute, for example, ContourMatching project:

```sh
./ContourMatching
```

## Dependencies

To run App-CV projects, you should have `g++`, `make` and `OpenCV` installed

Installing OpenCV from default apk store would not work, since most projects are using extra OpenCV modules. To install those modules, OpenCV has to be compiled and installed from source code with extra modules. Here is the tutorial followed by the developers of App-CV:

[How to use Cmake to build and install OpenCV and Extra Modules from source and configure your Pycharm IDE](https://towardsdatascience.com/how-to-install-opencv-and-extra-modules-from-source-using-cmake-and-then-set-it-up-in-your-pycharm-7e6ae25dbac5)