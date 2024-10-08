//
// Created by saleh on 9/26/24.
//

#include <algorithm>
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
            auto timeSpent = CTime::ForLambda([&]() {
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

inline H5::DataSpace CreateDataSpaceStringVariableLength() {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    H5::DataSpace dataspace(1, dims, maxdims);
    return dataspace;
}

void test03() {
    cout << "\tRunning Test03:" << endl;

    const size_t tensorLen = 100;

    H5::H5File file("test03.h5", H5F_ACC_TRUNC);

    // Create a dataset creation property list and enable chunking
    H5::DSetCreatPropList prop;
    hsize_t chunk_dims[1] = {10};
    prop.setChunk(1, chunk_dims);

    // Create a dataspace with unlimited dimensions
    hsize_t dims[1] = {tensorLen};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    H5::DataSpace dataspace(1, dims, maxdims);

    // Create tensors
    std::vector<H5::Group> tensors = {file.createGroup("GroupTnId0"), file.createGroup("GroupTnId1")};

    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);

    // Create a dataset called "ExprSEs" of type string
    for (auto& tn : tensors) {
        auto dataset = tn.createDataSet("ExprSEs", strdatatype, dataspace, prop);

        // We can get the dataset this way too:
        //H5::DataSet dataset = tn.openDataSet("ExprSEs");


        for (int i = 0; i < tensorLen; i++) {
            // get the dataset "ExprSEs"
            dataset.write("expr" + std::to_string(i), strdatatype);
        }
    }

    for (auto& tn : tensors) {
        auto gStates = tn.createGroup("GroupStates");
        auto gFirsts = gStates.createGroup("GroupPairFirsts");
        auto gSeconds = gStates.createGroup("GroupPairSeconds");

        for (int i = 0; i < tensorLen; i++) {
            auto dSet = gFirsts.createDataSet("flat" + std::to_string(i), H5::StrType(H5::PredType::C_S1, H5T_VARIABLE),
                                              dataspace, prop);
            for (int j = 0; j < 3; j++) {
                dSet.write("stateExpr" + std::to_string(j), H5::StrType(H5::PredType::C_S1, H5T_VARIABLE));
            }

            auto dSet2 = gSeconds.createDataSet("flat" + std::to_string(i),
                                                H5::StrType(H5::PredType::C_S1, H5T_VARIABLE), dataspace, prop);
            for (int j = 0; j < 3; j++) {
                dSet2.write("combs" + std::to_string(j), H5::StrType(H5::PredType::C_S1, H5T_VARIABLE));
            }
        }

        // create the following attributes for `tn`: {"tnName": "foo", "tnLen": 1234}
        H5::Attribute attr = tn.createAttribute("tnName", H5::StrType(H5::PredType::C_S1, H5T_VARIABLE), dataspace);
        attr.write(H5::StrType(H5::PredType::C_S1, H5T_VARIABLE), "foo");

        H5::Attribute attr2 = tn.createAttribute("tnLen", H5::PredType::NATIVE_INT, H5::DataSpace(H5S_SCALAR));
        int tnLen = 1234;
        attr2.write(H5::PredType::NATIVE_INT, &tnLen);
    }

    file.flush(H5F_SCOPE_LOCAL);
    file.close();
}

void test04() {
    // Initialize the vector of strings
    std::vector<std::string> strings = {"len1", "len1", "len1"};

    // Create a new HDF5 file
    H5::H5File file("test04.h5", H5F_ACC_TRUNC);

    // Define the datatype for the string dataset
    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);

    // Define the dataspace for the dataset
    hsize_t dims[1] = {strings.size()};
    H5::DataSpace dataspace(1, dims);

    // Create the dataset
    H5::DataSet dataset = file.createDataSet("strings_dataset", strdatatype, dataspace);

    // Write the data to the dataset
    dataset.write(strings[0], strdatatype);
    dataset.write(strings[1], strdatatype);
    dataset.write(strings[2], strdatatype);
}

void test05() {
    // Initialize the vector of strings
    std::vector<std::string> strings = {
        "str1", "str22", "str333", "str4444", "str55555", "str666666", "str7", "str8", "str9", "str10"
    };

    // Create a new HDF5 file
    H5::H5File file("strings.h5", H5F_ACC_TRUNC);

    // Define the dimensions of the dataspace
    hsize_t dims[1] = {strings.size()};

    // Create a simple dataspace with the specified dimensions
    H5::DataSpace dataspace(1, dims);

    // Define the datatype for the string dataset
    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);

    // Create the dataset
    H5::DataSet dataset = file.createDataSet("strings_dataset", strdatatype, dataspace);

    for (int i = 0; i < strings.size(); i++) {
        // Select the hyperslab for the current element
        hsize_t offset[1] = {static_cast<hsize_t>(i)};
        hsize_t count[1] = {1};
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
        H5::DataSpace memspace(1, count);

        dataset.write(strings[i], strdatatype, memspace, dataspace);
    }

    // It seems that without hyperslab selection, a single call to dataset.write() should write all the elements of the dataset in one go.
    // If called one by one without hyperslab selection, it crashes with SIGSEV.
}

void test06() {
    cout << "\tRunning Test06:" << endl;

    H5::H5File file("test06.h5", H5F_ACC_TRUNC);

    // Expandable datasets
    hsize_t initial_size[1] = {0};
    hsize_t max_size[1] = {H5S_UNLIMITED};
    H5::DataSpace dataspace(1, initial_size, max_size);

    // Create the dataset creation property list and enable chunking
    H5::DSetCreatPropList prop;
    hsize_t chunk_size[1] = {1};
    prop.setLayout(H5D_CHUNKED);
    prop.setChunk(1, chunk_size);

    // Create the datasets for keys and values
    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::DataSet expandableDataset = file.createDataSet("expandableDataset", strdatatype, dataspace, prop);

    // Function to append data to the datasets
    auto append_to_dataset = [&](const std::vector<std::string>& data) {
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
    std::vector<std::string> data_chunk;
    data_chunk.reserve(chunk_size[0]);

    for (size_t i = 0; i < 1000 * chunk_size[0]; ++i) {
        // Create a string, e.g., "test_string_0001"
        data_chunk.push_back("test_string_" + std::to_string(i));;

        if (data_chunk.size() == chunk_size[0]) {
            auto timeSpent = CTime::ForLambda([&]() {
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


void test07() {
    // create a hdf5 file
    H5::H5File file("test07.h5", H5F_ACC_TRUNC);

    {
        // create a dataset of size int with 10 elements
        hsize_t dims[1] = {10};
        H5::DataSpace dataspace(1, dims);
        H5::DataSet dataset = file.createDataSet("int_dataset", H5::PredType::NATIVE_INT, dataspace);
    }

    {
        auto dataset = file.openDataSet("int_dataset");
        hsize_t dims[1] = {10};
        H5::DataSpace dataspace(1, dims);


        // write to index 1 of the dataset the value of 101
        int data = 101;
        hsize_t offset[1] = {1};
        hsize_t count[1] = {1};
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
        H5::DataSpace memspace(1, count);
        dataset.write(&data, H5::PredType::NATIVE_INT, memspace, dataspace);
    }

    file.close();
}

void DatasetWrite(H5::DataSet& dataset, size_t datasetSize, size_t index, const std::string& data) {
    hsize_t offset[1] = {index};
    hsize_t count[1] = {1}; // 1 element
    hsize_t dims[1] = {datasetSize};
    H5::DataSpace dataspace(1, dims);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    H5::DataSpace memspace(1, count);
    //H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::VarLenType vlenDatatype(H5::PredType::NATIVE_CHAR);
    dataset.write(data, vlenDatatype, memspace, dataspace);
}
void DatasetWrite2(H5::H5File &file, const std::string &datasetPath, size_t datasetSize, size_t index, const char *data, size_t dataLen) {
    auto dset = file.openDataSet(datasetPath);
    hsize_t offset[1] = {index};
    hsize_t count[1] = {1}; // 1 element
    hsize_t dims[1] = {datasetSize};
    H5::DataSpace dataspace(1, dims);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    H5::DataSpace memspace(1, count);
    //H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::VarLenType vlenDatatype(H5::PredType::NATIVE_CHAR);

    /**
    - serialized symengine expr string could contain termination char mid string.This is data, but HDF5 treats it as a terminator.
    - std::string::size() returns the size of the first substring to the first terminator. So we have to use sizeof(std::string::data()) instead.
     */
    hvl_t varLenData;
    varLenData.p = (void *)data;
    varLenData.len = dataLen;

    dset.write(&varLenData, vlenDatatype, memspace, dataspace);
}

hvl_t DatasetRead2(H5::H5File &file, const std::string &datasetPath, size_t datasetSize, size_t index) {
    auto dset = file.openDataSet(datasetPath);
    hsize_t offset[1] = {index};
    hsize_t count[1] = {1}; // 1 element
    hsize_t dims[1] = {datasetSize};
    H5::DataSpace dataspace(1, dims);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    H5::DataSpace memspace(1, count);
    H5::VarLenType vlenDatatype(H5::PredType::NATIVE_CHAR);

    hvl_t varLenData;
    dset.read(&varLenData, vlenDatatype, memspace, dataspace);
    return varLenData;
}

H5::DataSet DatasetCreateStringVariableLen(H5::H5File& file,
                                    const std::string& path,
                                    size_t datasetSize) {
    hsize_t dims[1] = {datasetSize};
    H5::DataSpace dataspace(1, dims);
    //H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::VarLenType vlenDatatype(H5::PredType::NATIVE_CHAR);
    return file.createDataSet(path, vlenDatatype, dataspace);
}

void test08() {
    // create a hdf5 file
    H5::H5File file("/tmp/test08.h5", H5F_ACC_TRUNC);

    /*
    // create a dataset of variable length string with 10 elements
    hsize_t dims[1] = {10};
    H5::DataSpace dataspace(1, dims);
    H5::StrType strdatatype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::DataSet dataset = file.createDataSet("string_dataset", strdatatype, dataspace);
    */

    file.createGroup("tensors");
    auto dataset = DatasetCreateStringVariableLen(file, "tensors/dset1", 10);

    DatasetWrite2(file, "tensors/dset1", 10, 0, "Hello\0World", 12);
    DatasetWrite2(file, "tensors/dset1", 10, 1, "string11", 8);
    DatasetWrite2(file, "tensors/dset1", 10, 2, "string222", 9);
    DatasetWrite2(file, "tensors/dset1", 10, 3, "string3333", 10);
    DatasetWrite2(file, "tensors/dset1", 10, 4, "string44444", 11);
    DatasetWrite2(file, "tensors/dset1", 10, 5, "string555555", 12);
    DatasetWrite2(file, "tensors/dset1", 10, 6, "string6666666", 13);
    DatasetWrite2(file, "tensors/dset1", 10, 7, "string77777777", 14);
    DatasetWrite2(file, "tensors/dset1", 10, 8, "string888888888", 15);
    DatasetWrite2(file, "tensors/dset1", 10, 9, "string9999999999", 16);


    auto data = DatasetRead2(file, "tensors/dset1", 10, 0);

    // compare each element of data.p with the char array Hello\0World
    cout << "data.len: " << data.len << endl;
    cout << "data.p: " << endl;
    for (int i = 0; i < 12; i++) {
        cout << ((char*)data.p)[i];
    }

    file.close();
}

#include "H5Cpp.h"
#include <vector>

int test09() {
    // File and group creation
    H5::H5File file("/tmp/test09.h5", H5F_ACC_TRUNC);
    H5::Group group = file.createGroup("/my_group");

    // Define the datatype for the data in the file (variable-length byte array)
    H5::VarLenType vlenDatatype(H5::PredType::NATIVE_CHAR);

    // Define the size of the dataset (1D array with 2 elements)
    hsize_t dims[1] = {2};
    H5::DataSpace dataspace(1, dims);

    // Create the dataset
    H5::DataSet dataset = group.createDataSet("my_dataset", vlenDatatype, dataspace);

    // Prepare variable-length data
    std::vector<uint8_t> data0(32, 0x01); // 32 bytes of value 0x01
    std::vector<uint8_t> data1(35, 0x02); // 35 bytes of value 0x02

    // Create HDF5 variable-length data structures
    hvl_t vl_data[2];
    vl_data[0].p = data0.data();
    vl_data[0].len = data0.size();
    vl_data[1].p = data1.data();
    vl_data[1].len = data1.size();

    // Write the variable-length data to the dataset
    dataset.write(vl_data, vlenDatatype);

    // select the offset of dataset and write the data to it
    hsize_t offset[1] = {1};
    hsize_t count[1] = {1};
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    dataset.write(vl_data + 1, vlenDatatype, dataspace);




    // Close the dataset and file
    dataset.close();
    group.close();
    file.close();

    return 0;
}

int main() {
    cout << "Running 01-cmake-cpp-basics" << endl;
    //test01();
    //test02();
    //test03();
    //test04();
    //test05();
    //test06();
    //test07();
    test08();
    //test09();
}
