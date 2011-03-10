#ifndef _SEQ_BOOT_MODEL_WIDGET_H_
#define _SEQ_BOOT_MODEL_WIDGET_H_

#include <U2View/CreatePhyTreeWidget.h>
#include "ui/ui_SeqBootModel.h"

#include <U2Core/MAlignment.h>


namespace U2{

class SeqBootModelWidget : public CreatePhyTreeWidget, Ui_SeqBootModel {
    Q_OBJECT

public:
    SeqBootModelWidget(QWidget* parent, const MAlignment& ma);
    virtual void fillSettings(CreatePhyTreeSettings& settings);
private slots:
    void sl_onCheckBox();


};

}

#endif