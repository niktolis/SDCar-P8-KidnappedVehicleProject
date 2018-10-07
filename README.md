[//]: # (Image References)

[image1]: ./screenshots/SuccessScreenshot.PNG "Successful Result on Simulator"

# Overview

The project implements a [Particle Filter](https://en.wikipedia.org/wiki/Particle_filter). The implementation is tested using the Udacity [SD simulator](https://github.com/udacity/self-driving-car-sim/releases). The simulator generates noisy landmark observations and provides them to the algorithm using via WebSocket. The particle filter is fed with a map and some initial localization information (analogous to what a GPS would provide). At each time step the filter will also get observation and control data.

## Running the Code
This project involves the Term 2 Simulator which can be downloaded [here](https://github.com/udacity/self-driving-car-sim/releases)

This repository includes two files that can be used to set up and install uWebSocketIO for either Linux or Mac systems. For windows you can use either Docker, VMware, or even Windows 10 Bash on Ubuntu to install uWebSocketIO.

Once the install for uWebSocketIO is complete, the main program can be built and ran by doing the following from the project top directory.

1. mkdir build
2. cd build
3. cmake ..
4. make
5. ./particle_filter

Alternatively some scripts have been included to streamline this process, these can be leveraged by executing the following in the top directory of the project:

1. ./clean.sh
2. ./build.sh
3. ./run.sh

### Visual Studio on Windows

1. Clone this repo.
2. Use [this](https://github.com/fkeidel/CarND-Term2-ide-profile-VisualStudio) repo to install the necessary dependencies for Visual Studio.
   Do not copy the `main.cpp` file and don't perform any changes on the `CMakeLists.txt` file as they are already modified to facilitate compilation on both VS and GNU environment.
3. Open the solution generated inside `./products` folder on your Visual Studio Environment.

During the installation of some libraries with vcpkg some of the mirrors may be inactive. You may alter the vcpkg to search into another mirror.

## Inputs to the Particle Filter
You can find the inputs to the particle filter in the `data` directory. 

#### The Map*
`map_data.txt` includes the position of landmarks (in meters) on an arbitrary Cartesian coordinate system. Each row has three columns
1. x position
2. y position
3. landmark id

### All other data the simulator provides, such as observations and controls.

> * Map data provided by 3D Mapping Solutions GmbH.

# Result

As it is also noted in the followin screenshot taken by the simulator the algorithm manages to localize the vehicle within the desired accuracy in the specified time of 100 seconds.

![alt text][image1]

