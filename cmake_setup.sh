#/bin/sh

rm -rf local
mkdir local
cd local
cmake -DLOCAL=ON -DCMAKE_BUILD_TYPE=DEBUG ..

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
