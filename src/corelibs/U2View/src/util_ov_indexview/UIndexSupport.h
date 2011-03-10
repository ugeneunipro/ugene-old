
#ifndef _U2_UINDEX_SUPPORT_H_
#define _U2_UINDEX_SUPPORT_H_

#include <U2Core/global.h>

#include <QtCore/QObject>

namespace U2 {

class U2VIEW_EXPORT UIndexSupport : public QObject {
    Q_OBJECT
public:
    UIndexSupport();
    
private slots:
    void sl_showCreateFileIndexDialog();
    void sl_creatingIndexTaskStateChanged();
    
}; 

} // U2

#endif // _U2_UINDEX_SUPPORT_H_
