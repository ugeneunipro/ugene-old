#ifndef _U2_AUTO_ANNOTATION_UTILS_H_
#define _U2_AUTO_ANNOTATION_UTILS_H_

#include "ADVSingleSequenceWidget.h"

namespace U2 {

class MWMDIWindow;

class AutoAnnotationsADVAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    AutoAnnotationsADVAction(ADVSequenceWidget* widget);
private slots:
    void sl_windowActivated(MWMDIWindow* w);
    void sl_onAutoAnnotationsUpdate(const QString& groupName);
private:
    void updateMenu();
    QMenu* menu;
};


}

#endif // _U2_AUTO_ANNOTATION_UTILS_H_
