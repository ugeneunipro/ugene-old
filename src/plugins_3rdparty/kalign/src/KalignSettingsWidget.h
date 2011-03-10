#ifndef _U2_KALIGN_SETTINGS_WIDGET_H_
#define _U2_KALIGN_SETTINGS_WIDGET_H_

#include "ui/ui_KalignSettingsWidget.h"
#include "U2View/MSAAlignGUIExtension.h"

namespace U2{

class KalignSettingsWidget : public MSAAlignAlgorithmMainWidget, Ui_KalignSettingsWidget {
    Q_OBJECT
public:
    KalignSettingsWidget(QWidget* parent);
    virtual QMap<QString,QVariant> getMSAAlignCustomSettings();    
};

}

#endif