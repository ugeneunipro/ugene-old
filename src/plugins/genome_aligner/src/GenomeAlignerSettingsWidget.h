#ifndef _DIST_MATRIX_MODEL_WIDGET_H_
#define _DIST_MATRIX_MODEL_WIDGET_H_

#include "ui/ui_GenomeAlignerSettings.h"
#include "U2View/DnaAssemblyGUIExtension.h"

namespace U2{

class GenomeAlignerSettingsWidget : public DnaAssemblyAlgorithmMainWidget, Ui_GenomeAlignerSettings {
    Q_OBJECT
public:
    GenomeAlignerSettingsWidget(QWidget* parent);
    virtual QMap<QString,QVariant> getDnaAssemblyCustomSettings();
    virtual bool isIndexOk();
    virtual void buildIndexUrl(const GUrl& url);

private slots:
    void sl_onSetIndexFileNameButtonClicked();
    void sl_onOpenPrebuildIndexFileNameButtonClicked();
private:
    QString refUrl;
};

}

#endif
