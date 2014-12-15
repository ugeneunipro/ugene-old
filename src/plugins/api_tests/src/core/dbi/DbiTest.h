#ifndef DBITEST_H
#define DBITEST_H

#include <U2Core/U2Dbi.h>
#include <unittest.h>

namespace U2 {

/*Helper to provide dbi for tests tests.
In case you need to open a connection within your test useConnectionPool must be true to use the connection pool
if you don't need to open connections within your test useConnectionPool must be false to use created dbi without the pool*/

class TestDbiProvider{
public:
    TestDbiProvider();
    ~TestDbiProvider();

    bool init(const QString& dbiFileName, bool useConnectionPool);
    void close();
    U2Dbi* getDbi();
private:
    bool initialized;
    bool useConnectionPool;
    QString dbUrl;
    U2Dbi* dbi;
};

template<> inline QString toString<U2DataId>(const U2DataId &a) { return "0x" + QString(a.toHex()); }
template<> inline QString toString<U2Region>(const U2Region &r) { return r.toString(); }

} // namespace U2

#endif // DBITEST_H
