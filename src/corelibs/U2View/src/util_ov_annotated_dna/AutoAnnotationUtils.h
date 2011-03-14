#ifndef _U2_AUTO_ANNOTATION_UTILS_H_
#define _U2_AUTO_ANNOTATION_UTILS_H_

#include "ADVSingleSequenceWidget.h"

namespace U2 {

class MWMDIWindow;
class AutoAnnotationObject;

class AutoAnnotationsADVAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    AutoAnnotationsADVAction(ADVSequenceWidget* widget, AutoAnnotationObject* aaObj);
private slots:
    void sl_toggle(bool toggled);
private:
    void updateMenu();
    AutoAnnotationObject* aaObj;
    QMenu* menu;
};


}

#endif // _U2_AUTO_ANNOTATION_UTILS_H_
