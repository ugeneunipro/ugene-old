#include "SeqBootModelWidget.h"

#include <U2Core/DNAAlphabet.h>

namespace U2 {


SeqBootModelWidget::SeqBootModelWidget(QWidget* parent, const MAlignment& ma) : CreatePhyTreeWidget(parent)
{
    setupUi(this);
    
    connect(bootstrapCheckBox, SIGNAL(clicked()), SLOT(sl_onCheckBox()));
 
}

void SeqBootModelWidget::fillSettings( CreatePhyTreeSettings& settings )
{
    settings.bootstrap = bootstrapCheckBox->isChecked();
    settings.replicates = repsSpinBox->value();
}

void SeqBootModelWidget::sl_onCheckBox() {
   label2->setEnabled(bootstrapCheckBox->isChecked());
   repsSpinBox->setEnabled(bootstrapCheckBox->isChecked());
}



} //namespace
