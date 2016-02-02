#/bin/sh

rm -rf local
mkdir local
cd local
cmake -DCMAKE_BUILD_TYPE=DEBUG -DLOCAL=ON ..

cd ..

rm -rf debug
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=DEBUG ..

cd ..

rm -rf release
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
