#ifndef _DIST_BUILD_SARRAY_WIDGET_H_
#define _DIST_BUILD_SARRAY_WIDGET_H_

#include "ui/ui_BuildSuffixArraySettings.h"
#include "U2View/DnaAssemblyGUIExtension.h"

namespace U2{

class BuildSArraySettingsWidget : public DnaAssemblyAlgorithmBuildIndexWidget, Ui_BuildSuffixArraySettings {
    Q_OBJECT
public:
    BuildSArraySettingsWidget(QWidget* parent);
    virtual QMap<QString,QVariant> getBuildIndexCustomSettings();
    virtual QString getIndexFileExtension();
};

}

#endif
