import h5py


def test01():
    print('test01')
    with h5py.File('test.h5', 'w') as f:
        group1 = f.create_group('group1')
        group1.create_dataset('dataset1', data=[1, 2, 3])
        group2 = f.create_group('group2')
        group2.create_dataset('dataset2', data=[4, 5, 6])

        # print a summary of the file
        print(f)
        print(f['group1'])
        print(f['group1']['dataset1'])
        print(f['group1']['dataset1'].name)
        print(f['group2'])
        print(f['group2']['dataset2'])
        print(f['group2']['dataset2'].name)


if __name__ == '__main__':
    test01()
