# CS452 Kernel

* Taylor Stark
* Victor Lai

## Builds

CMake is used for compilation. All builds should be done out of source, i.e. in another directory. The `cmake_setup.sh` script can be run to generate the local, debug and release build folders. 

### Local 

    cd CS452-Kernel 
    mkdir local
    cd local
    cmake -DLOCAL=ON -DCMAKE_BUILD_TYPE=DEBUG ..

### Debug

    cd CS452-Kernel 
    mkdir debug
    cd debug
    cmake -DCMAKE_BUILD_TYPE=DEBUG ..

### Release

    cd CS452-Kernel
    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=RELEASE ..
