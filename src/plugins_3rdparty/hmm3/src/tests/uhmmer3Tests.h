#ifndef _GB2_UHMMER3_TESTS_H_
#define _GB2_UHMMER3_TESTS_H_

#include <QtCore/QList>
#include <U2Test/XMLTestFormat.h>

namespace U2 {

class UHMMER3Tests {
public:
    static QList<XMLTestFactory*> createTestFactories();
}; // UHMMER3Tests

} // GB2

#endif // _GB2_UHMMER3_TESTS_H_
