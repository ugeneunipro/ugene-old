/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QDoubleValidator>
#include <QMessageBox>
#include <QValidator>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Core/QObjectScopedPointer.h>

#include "ExpertDiscoveryAdvSetDialog.h"

namespace U2 {

ExpertDiscoveryAdvSetDialog::ExpertDiscoveryAdvSetDialog(QWidget *parent,
        double& dIntProbability1,
        double& dIntFisher1,
        int&    nMinComplexity1,
        int&    nMaxComplexity1,
        double& dMinPosCorrelation1,
        double& dMaxPosCorrelation1,
        double& dMinNegCorrelation1,
        double& dMaxNegCorrelation1,
        bool&   bCorrelationImportant1)
: QDialog(parent),
    dIntProbability(dIntProbability1),
    dIntFisher(dIntFisher1),
    nMinComplexity(nMinComplexity1),
    nMaxComplexity(nMaxComplexity1),
    dMinPosCorrelation(dMinPosCorrelation1),
    dMaxPosCorrelation(dMaxPosCorrelation1),
    dMinNegCorrelation(dMinNegCorrelation1),
    dMaxNegCorrelation(dMaxNegCorrelation1),
    bCorrelationImportant(bCorrelationImportant1){

    setupUi(this);

    groupBox_2->setDisabled(true);
    condProbEdit->setText(QString("%1").arg(dIntProbability));
    fishCritEdit->setText(QString("%1").arg(dIntFisher));
    minComplexEdit->setText(QString("%1").arg(nMinComplexity));
    maxComplEdit->setText(QString("%1").arg(nMaxComplexity));
    minCorPosEdit->setText(QString("%1").arg(dMinPosCorrelation));
    maxCorPosEdit->setText(QString("%1").arg(dMaxPosCorrelation));
    minCorNegEdit->setText(QString("%1").arg(dMinNegCorrelation));
    maxCorNegEdit->setText(QString("%1").arg(bCorrelationImportant));

    QDoubleValidator* dIntProbabilityValid = new QDoubleValidator(0,100,5,this);
    QDoubleValidator* dIntFisherValid = new QDoubleValidator(0,1,5,this);
    QIntValidator* nComplexityValid = new QIntValidator(0,1000,this);
    QDoubleValidator* dCorrelationValid = new QDoubleValidator(-1,1,5,this);


    condProbEdit->setValidator(dIntProbabilityValid);
    fishCritEdit->setValidator(dIntFisherValid);
    minComplexEdit->setValidator(nComplexityValid);
    maxComplEdit->setValidator(nComplexityValid);
    minCorPosEdit->setValidator(dCorrelationValid);
    maxCorPosEdit->setValidator(dCorrelationValid);
    minCorNegEdit->setValidator(dCorrelationValid);
    maxCorNegEdit->setValidator(dCorrelationValid);
    connect( checkCorrel, SIGNAL(clicked()), SLOT(sl_checkCorrel()) );

    hideParameters();
    new HelpButton(this, buttonBox, "17466264");

}

void ExpertDiscoveryAdvSetDialog::accept(){
    int minCom = 0;
    int maxCom = 0;
    minCom = minComplexEdit->text().toInt();
    maxCom = maxComplEdit->text().toInt();

    if(minCom>maxCom || minCom<0){
        QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Wrong parameters"), tr("Minimal complexity must not be grater then maximal complexity and positive"));
        mb->exec();
        CHECK(!mb.isNull(), );
    }else{
        if(check(condProbEdit) && check(fishCritEdit) &&
            check(minCorPosEdit) && check(maxCorPosEdit)
            && check(minCorNegEdit) && check(maxCorNegEdit) ){
            dIntProbability = condProbEdit->text().toDouble();
            dIntFisher = fishCritEdit->text().toDouble();
            nMinComplexity = minCom;
            nMaxComplexity = maxCom;
            bCorrelationImportant = checkCorrel->isChecked();
            if(bCorrelationImportant){
                dMinPosCorrelation = minCorPosEdit->text().toDouble();
                dMaxPosCorrelation = maxCorPosEdit->text().toDouble();
                dMinNegCorrelation = minCorNegEdit->text().toDouble();
                dMaxNegCorrelation = maxCorNegEdit->text().toDouble();
            }
            QDialog::accept();
        }
    }

}
void ExpertDiscoveryAdvSetDialog::sl_checkCorrel(){
    groupBox_2->setEnabled(checkCorrel->isChecked());
}

bool ExpertDiscoveryAdvSetDialog::check(const QLineEdit* lineE){
    if(!lineE->validator()){
        return false;
    }
    const QDoubleValidator* validator = qobject_cast<const QDoubleValidator*>(lineE->validator());
    int pos = 0;
        QString textValue=lineE->text();
        if(validator->validate(textValue,pos)!=QValidator::Acceptable){
        QString msg = QString("Entered value must be from %1 to %2").arg(validator->bottom()).arg(validator->top());
        QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Wrong parameters"), tr(msg.toStdString().c_str()));
        mb->exec();
        return false;
    }
    return true;
}

void ExpertDiscoveryAdvSetDialog::hideParameters(){
    groupBox_2->hide();
    fishCritEdit->hide();
    label->hide();
    label_2->hide();
    condProbEdit->hide();
    checkCorrel->hide();
}


}//namespace


