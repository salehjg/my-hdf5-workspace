# add executable
add_executable(main main.cpp)

# link hdf5
target_link_libraries(main PRIVATE ${HDF5_CXX_LIBRARIES})