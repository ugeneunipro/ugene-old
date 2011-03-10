#ifndef _U2_ROLLING_MATRIX_H_
#define _U2_ROLLING_MATRIX_H_

#include <cstdio>

namespace U2 {

//TODO: rename n,m to rows, columns    

class U2ALGORITHM_EXPORT RollingMatrix {

public:
    RollingMatrix(int _n, int _m) : n(_n), m(_m), column0(0) {
        data = new int[n*m];
    }

    virtual ~RollingMatrix() {
        delete[] data;
    }

    virtual int get(int x, int y) const {
        int transX = transposeX(x);
        int transY = transposeY(y);
        int idx = getIdx(transX, transY);
        return data[idx];
    }

    void set(int x, int y, int v) {
        int transX = transposeX(x);
        int transY = transposeY(y);
        int idx = getIdx(transX, transY);
        data[idx] = v;
    }

    void dump() const {
        printf("----------------\n");
        for (int j=0; j<m; j++) {
            for (int i=0; i<n; i++) {
                printf("%x ", get(i, j));
            }
            printf("\n");
        }
    }

    void shiftColumn() {
        if (++column0  == n) {
            column0 = 0;
        }
    }

private:

    int getIdx(int x, int y) const { 
        assert(x>=0 && y>=0 && x<n && y<m);
        return  x*m+y;
    }


    int transposeX(int x) const { 
        assert(x >= 0 && x < n); 
        return (column0 + x) % n ;
    }

    int transposeY(int y) const {
        assert(y >= 0 && y < m);
        return y;
    }

protected:
    int n;
    int m;
    int* data;
    int column0;
};



} //namespace

#endif
