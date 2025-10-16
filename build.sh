clear
mkdir -p build
rm -rf build/*

# 配置 CMake 为 Release 模式
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..

# 构建
make

# make install