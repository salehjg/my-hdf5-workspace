//
// Created by saleh on 9/26/24.
//

#include <iostream>
#include <vector>
#include <map>
#include <H5Cpp.h>.
#include "CTime.h"

using namespace std;

void test01() {
    cout << "\tRunning Test01:" << endl;
    vector<int> v = {1, 2, 3, 4, 5};

    H5::H5File file("test01.h5", H5F_ACC_TRUNC);

    hsize_t dims[1] = {v.size()};
    H5::DataSpace dataspace(1, dims);
    H5::DataSet dataset = file.createDataSet("my_dataset", H5::PredType::NATIVE_INT, dataspace);
    dataset.write(v.data(), H5::PredType::NATIVE_INT);
}

void test02() {
    cout << "\tRunning Test02:" << endl;

    H5::H5File file("test02.h5", H5F_ACC_TRUNC);

    // Expandable datasets
    hsize_t initial_size[1] = {0};
    hsize_t max_size[1] = {H5S_UNLIMITED};
    H5::DataSpace dataspace(1, initial_size, max_size);

    // Create the dataset creation property list and enable chunking
    H5::DSetCreatPropList prop;
    hsize_t chunk_size[1] = {256};
    prop.setLayout(H5D_CHUNKED);
    prop.setChunk(1, chunk_size);

    // Create the datasets for keys and values
    H5::StrType strdatatype(H5::PredType::NATIVE_ULLONG);
    H5::DataSet expandableDataset = file.createDataSet("expandableDataset", strdatatype, dataspace, prop);

    // Function to append data to the datasets
    auto append_to_dataset = [&](const std::vector<size_t>& data) {
        // Get the current size of the dataset
        H5::DataSpace filespace = expandableDataset.getSpace();
        hsize_t size[1];
        filespace.getSimpleExtentDims(size);

        // Extend the dataset
        hsize_t new_size[1] = {size[0] + data.size()};
        expandableDataset.extend(new_size);

        // Select the hyperslab in the extended part
        filespace = expandableDataset.getSpace();
        hsize_t offset[1] = {size[0]};
        hsize_t count[1] = {data.size()};
        filespace.selectHyperslab(H5S_SELECT_SET, count, offset);

        // Create a memory dataspace
        hsize_t mem_dims[1] = {data.size()};
        H5::DataSpace memspace(1, mem_dims);

        // Write the data to the extended part
        expandableDataset.write(data.data(), strdatatype, memspace, filespace);
    };

    // Append keys and values to the datasets
    float timeSpentAcc = 0;
    std::vector<size_t> data_chunk;
    data_chunk.reserve(256);

    for (size_t i = 0; i < 100000000; ++i) {
        // Create a string, e.g., "test_string_0001"
        data_chunk.push_back(i);

        if (data_chunk.size() == 256) {
            auto timeSpent = CTime::ForLambda([&](){
                append_to_dataset(data_chunk);
            });
            timeSpentAcc += timeSpent;

            data_chunk.clear();
        }

        if (i % 256 == 0) {
            std::cout << "Average time spent per iteration: " << timeSpentAcc / 1000.0f << std::endl;
            std::cout << "Iters: " << i << std::endl;

            timeSpentAcc = 0;
        }
    }

    // Write any remaining data
    if (!data_chunk.empty()) {
        append_to_dataset(data_chunk);
    }

}

int main() {
    cout << "Running 01-cmake-cpp-basics" << endl;
    test01();
    test02();
}