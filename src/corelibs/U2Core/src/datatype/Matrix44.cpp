#include "Matrix44.h"

namespace U2 {

/* class Matrix44 */
Matrix44::Matrix44() {
    loadZero();
}

void Matrix44::loadIdentity() {
    loadZero();
    for (int i = 0; i < 4; ++i) {
        m[i + i*4] = 1.0;
    }
}

void Matrix44::loadZero() {
    for (int i = 0; i < 16; ++i) {
        m[i] = 0.0;
    }
}

float& Matrix44::operator[] (unsigned int i) {
    assert(i < 16);
    return m[i];
}

float Matrix44::operator[] (unsigned int i) const {
    assert(i < 16);
    return m[i];
}

Matrix44& Matrix44::operator= (const Matrix44 &matrix) {
    if (this == &matrix) {
        return *this;
    }
    memcpy((void*) m, (void*) matrix.m, sizeof(matrix.m) );
    return *this;
}

void Matrix44::load( QVariantList values )
{
    Q_ASSERT(values.size() == 16);
    for (int i = 0; i < 16; ++i) {
        m[i] = values.at(i).value<float>();
    }

}

QVariantList Matrix44::store()
{
    QVariantList values;
    for (int i = 0; i < 16; ++i ) {
        values.append(QVariant::fromValue(m[i]));
    }
    return values;
}

}   // namespace U2
