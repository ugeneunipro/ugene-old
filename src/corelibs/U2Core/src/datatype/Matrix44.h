#ifndef _U2_MATRIX_44_H_
#define _U2_MATRIX_44_H_

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT Matrix44 {
public:
    float m[16];

public:
    Matrix44();

    void loadZero();
    void loadIdentity();

    float* getData() { return m; }

    float& operator[] (unsigned int i);
    float operator[] (unsigned int i) const;

    Matrix44& operator= (const Matrix44 &m);

    void load(QVariantList values);
    QVariantList store();
};  // class Matrix44

}   // namespace U2

#endif  // #ifndef _U2_MATRIX_44_H_
