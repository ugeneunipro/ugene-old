#ifndef _DIST_MATRIX_MODEL_WIDGET_H_
#define _DIST_MATRIX_MODEL_WIDGET_H_

#include <U2View/CreatePhyTreeWidget.h>
#include "ui/ui_DistMatrixModel.h"

#include <U2Core/MAlignment.h>


namespace U2{

class DistMatrixModelWidget : public CreatePhyTreeWidget, Ui_DistMatrixModel {
    Q_OBJECT

public:
    DistMatrixModelWidget(QWidget* parent, const MAlignment& ma);
    virtual void fillSettings(CreatePhyTreeSettings& settings);
private slots:
    void sl_onModelChanged(const QString& modelName);


};

}

#endif