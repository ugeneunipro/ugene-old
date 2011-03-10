#include "CreateRulerDialogController.h"

#include <U2Core/TextUtils.h>

#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QPalette>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QColorDialog>

namespace U2 {

CreateRulerDialogController::CreateRulerDialogController(const QSet<QString>& namesToFilter, 
                                                         const U2Region& seqRange, int defaultOffset, QWidget* p) 
: QDialog(p)
{
    setupUi(this);
    setMaximumHeight(layout()->minimumSize().height());
    
    filter = namesToFilter;

    sampleLabel->setAutoFillBackground(true);
   
    nameEdit->setText(TextUtils::variate(tr("New ruler"), "_", filter));

    spinBox->setMinimum(seqRange.startPos + 1);
    spinBox->setMaximum(seqRange.endPos());
    spinBox->setValue(seqRange.contains(defaultOffset+1) ? defaultOffset + 1 : spinBox->minimum());

    color = Qt::darkBlue;
    
    updateColorSample();

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButtonClicked()));
    setWindowIcon(QIcon(":/ugene/images/ugene_16.png"));
}

void CreateRulerDialogController::updateColorSample() {
    QPalette lPal;
    lPal.setColor(QPalette::Window, Qt::white);
    lPal.setColor(QPalette::WindowText, color);
    sampleLabel->setPalette(lPal);
}

void CreateRulerDialogController::sl_colorButtonClicked() {
    QColor c = QColorDialog::getColor(color, this);
    if (!c.isValid()) {
        return;
    }
    color = c;
    updateColorSample();
}

void CreateRulerDialogController::accept() {
    QString n = nameEdit->text();
    if (n.isEmpty()) {
        QMessageBox::critical(NULL, tr("Error"), tr("Ruler name is empty!"));
        nameEdit->setFocus();
        return;
    }
    if (filter.contains(n)) {
        QMessageBox::critical(NULL, tr("Error"), tr("Ruler with the same name is already exists!"));
        nameEdit->setFocus();
        return;
    }
    name = n;
    offset = spinBox->value() - 1;
    QDialog::accept();
}

}//namespace


