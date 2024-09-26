cmake_minimum_required(VERSION 3.5)
project(my-hdf5-workspace CXX)

# find hdf5
find_package(HDF5 REQUIRED COMPONENTS CXX)

add_subdirectory(01-cmake-cpp-basics)
