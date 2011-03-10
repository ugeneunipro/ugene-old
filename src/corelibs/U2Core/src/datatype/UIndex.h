
#ifndef _U2_UINDEX_H_
#define _U2_UINDEX_H_

#include <QObject>
#include <QString>
#include <QHash>

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT UIndex  {
public:
    //errors
    enum UIndexError {
        NO_ERR,
        EMPTY_IO_ID,
        EMPTY_DOC_FORMAT,
        EMPTY_IO_ADAPTER,
        EMPTY_URL,
        BAD_OFFSET,
        NO_SUCH_IO,
        EMPTY_KEY_VAL
    };
    
    struct U2CORE_EXPORT ItemSection {
        QString                  ioSectionId;
        DocumentFormatId         docFormat;
        qint64                   startOff;
        qint64                   endOff;
        QHash<QString, QString > keys;

        UIndexError checkConsistentcy() const;
    };

    struct U2CORE_EXPORT IOSection {
        QString                   sectionId;
        IOAdapterId               ioAdapterId;
        QString                   url;
        QHash< QString, QString > keys;

        UIndexError checkConsistentcy() const;
    };
    
    IOSection getIOSection(const QString& id) const;
    bool hasItems() const;
    
    QList< ItemSection > items;
    QList< IOSection   > ios;
    
    UIndexError checkConsistency() const;
}; // UIndex

} // U2

#endif // _U2_UINDEX_H_
