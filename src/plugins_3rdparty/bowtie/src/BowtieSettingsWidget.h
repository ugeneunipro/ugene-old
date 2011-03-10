#ifndef _DIST_MATRIX_MODEL_WIDGET_H_
#define _DIST_MATRIX_MODEL_WIDGET_H_

#include "ui/ui_BowtieSettings.h"
#include "U2View/DnaAssemblyGUIExtension.h"

namespace U2{

class BowtieSettingsWidget : public DnaAssemblyAlgorithmMainWidget, Ui_BowtieSettings {
    Q_OBJECT
public:
    BowtieSettingsWidget(QWidget* parent);
    virtual QMap<QString,QVariant> getDnaAssemblyCustomSettings();
	virtual void buildIndexUrl(const GUrl& url) {};
	virtual bool isIndexOk() {return true;};
};

}

#endif