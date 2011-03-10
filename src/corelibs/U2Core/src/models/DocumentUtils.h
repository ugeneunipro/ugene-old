#ifndef _U2_DOCUMENT_UTILS_H_
#define _U2_DOCUMENT_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/IOAdapter.h>

#include "DocumentModel.h"

namespace U2 {

class U2CORE_EXPORT DocumentUtils: public QObject    {
public:
    /* returns set with document urls */
    static QSet<QString> getURLs(const QList<Document*>& docs);
        
    /*  The set of urls that should not be used for new documents
        returns list of loaded urls. Gets them from the active project 
    */
    static QSet<QString> getNewDocFileNameExcludesHint();

    // the best match goes first in the returned list
    static QList<DocumentFormat*> detectFormat(const GUrl& url);

    // io - opened io adapter
    static QList<DocumentFormat*> detectFormat( IOAdapter* io );

    static QList<DocumentFormat*> detectFormat(const QByteArray& rawData, const QString& ext = QString(), const GUrl& url = GUrl());
};

}//namespace

#endif

