#ifndef _U2_WORKLFOW_SUPPORT_HR_FORMAT_UTILS_H_
#define _U2_WORKLFOW_SUPPORT_HR_FORMAT_UTILS_H_

#include "WorkflowViewController.h"
#include "WorkflowViewItems.h"
#include "ItemViewStyle.h"

namespace U2 {

class HRSceneSerializer : QObject {
    Q_OBJECT
public:
    static QString scene2String(WorkflowScene * scene, const Metadata & meta);
    // returns error or empty string
    static QString string2Scene(const QString & data, WorkflowScene * scene, Metadata * meta = NULL, 
                                bool select = false, bool pasteMode = false);
    
    static QString items2String(const QList<QGraphicsItem*> & items, const QList<Iteration> & iterations);
    
}; // HRSceneSerializer

} // U2

#endif // _U2_WORKLFOW_SUPPORT_HR_FORMAT_UTILS_H_
