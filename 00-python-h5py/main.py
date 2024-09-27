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

        # *: means its a dataset.
        # TnId --> *exprSE: [exprFlat0, exprFlat1, exprFlat2, ...]
        # TnId --> states --> pairs_first -->  *dsetFlat0 : [expr0, expr1, expr2, ...], *dsetFlat1: [...], ...
        # TnId --> states --> pairs_second --> *dsetFlat0 : [serializedAttr0, serializedAttr1, ...], *dsetFlat1: [...], ..
        # TnId ==> ATTRIBUTES: {tnLen: xxx, tnShape: [1,2,3], tnName: "FOO"}


def test02():
    print('test02')
    with h5py.File('test02.h5', 'w') as f:
        group1 = f.create_group('group1')
        group1.create_dataset('dataset1', data=["a", "bbb", "cccccccccccccc"])

if __name__ == '__main__':
    test01()
    test02()
