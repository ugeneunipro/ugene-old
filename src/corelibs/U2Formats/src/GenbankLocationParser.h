#ifndef _U2_GENBANK_LOCATION_PARSER_H
#define _U2_GENBANK_LOCATION_PARSER_H

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2Annotation.h>

#include <QtCore/QList>

namespace U2 {

class AnnotationData;

namespace Genbank {

class U2FORMATS_EXPORT LocationParser {
public:
    static void parseLocation(const char* str, int len, U2Location& location);

    static QString buildLocationString(const AnnotationData* a);

    static QString buildLocationString(const QVector<U2Region>& regions);
};


}}//namespace
#endif
