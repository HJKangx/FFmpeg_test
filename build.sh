rm -rf ./build
mkdir -p build
cd build
cmake -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true -DCMAKE_BUILD_TYPE=Release .. 
make -j
cd ..
