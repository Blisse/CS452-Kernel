#/bin/sh

mkdir local
cd local
cmake -DLOCAL=ON -DCMAKE_BUILD_TYPE=DEBUG ..

cd ..

mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=DEBUG ..

cd ..

mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
