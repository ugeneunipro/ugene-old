#ifndef _U2_IDENTIFIABLE_H_
#define _U2_IDENTIFIABLE_H_

#include <U2Core/global.h>

namespace U2 {

// A template of object with identity

template<typename T> class Identifiable {
public:
    Identifiable(const T& _id) : id(_id) {}

    const T& getId() const {return id;}

private:
    T id;
};

} //namespace

#endif
