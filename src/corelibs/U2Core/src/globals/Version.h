#ifndef _U2_VERSION_H_
#define _U2_VERSION_H_

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT Version {
public:
    // creates empty version structure
    Version();

    // Current UGENE instance version
    static Version ugeneVersion();

    // Current QT version
    static Version qtVersion();

    static Version parseVersion(const QString& text);

    bool operator  >  (const Version& v) const;
    bool operator  >= (const Version& v) const;
    bool operator  <  (const Version& v) const;
    bool operator  <= (const Version& v) const;
    bool operator  == (const Version& v) const;

    int major;
    int minor;
    int patch;
    bool debug;
    QString text;
    QString suffix;
    bool isDevVersion;
};

} //namespace

#endif
