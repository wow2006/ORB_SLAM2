echo "Clean build"

rm ./build ./Thirdparty/DBoW2/build ./Thirdparty/g2o/build -rf

echo "Configuring and building Thirdparty/DBoW2 ..."

cd Thirdparty/DBoW2
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=~/Documents/sourceCode/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j

cd ../../g2o

echo "Configuring and building Thirdparty/g2o ..."

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=~/Documents/sourceCode/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j

cd ../../../

echo "Uncompress vocabulary ..."

cd Vocabulary
tar -xf ORBvoc.txt.tar.gz
cd ..

echo "Configuring and building ORB_SLAM2 ..."

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=~/Documents/sourceCode/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j
