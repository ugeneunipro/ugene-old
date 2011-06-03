/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "KalignDialogController.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>

/* TRANSLATOR U2::KalignDialogController */    

namespace U2 {

class QClearableDoubleSpinBox : public QDoubleSpinBox {
public:	
	QString textFromValue(double val) const {
		return isEnabled() ? QDoubleSpinBox::textFromValue(val) : "";
	}
	
	bool event(QEvent *ev) {
		if(ev->type() == QEvent::EnabledChange) {
			this->setValue(this->value());
		}
		return QDoubleSpinBox::event(ev);
	}

};

KalignDialogController::KalignDialogController(QWidget* w, const MAlignment& _ma, KalignTaskSettings& _settings) 
: QDialog(w), ma(_ma), settings(_settings)
{
    setupUi(this);
	setupUiExt();
    translateCheckBox->setEnabled(ma.getAlphabet()->isNucleic());
}

void KalignDialogController::setupUiExt() {    
	gapOpenSpinBox->setEnabled(false);
	gapExtensionPenaltySpinBox->setEnabled(false);
	terminalGapSpinBox->setEnabled(false);
	bonusScoreSpinBox->setEnabled(false);

    if(ma.getAlphabet()->isAmino()) {
        gapOpenSpinBox->setValue(53.9);
        gapExtensionPenaltySpinBox->setValue(8.52);
        terminalGapSpinBox->setValue(4.42);
        bonusScoreSpinBox->setValue(0.02);
    } else {
        gapOpenSpinBox->setValue(217);
        gapExtensionPenaltySpinBox->setValue(39.4);
        terminalGapSpinBox->setValue(292.6);
        bonusScoreSpinBox->setValue(28.3);
    }

	QObject::connect(gapOpenCheckBox, SIGNAL(clicked(bool)), gapOpenSpinBox, SLOT(setEnabled(bool)));
	QObject::connect(gapExtensionPenaltyCheckBox, SIGNAL(clicked(bool)), gapExtensionPenaltySpinBox, SLOT(setEnabled(bool)));
	QObject::connect(terminalGapCheckBox, SIGNAL(clicked(bool)), terminalGapSpinBox, SLOT(setEnabled(bool)));
	QObject::connect(bonusScoreCheckBox, SIGNAL(clicked(bool)), bonusScoreSpinBox, SLOT(setEnabled(bool)));
}

void KalignDialogController::accept() {

	if(gapOpenCheckBox->isChecked()) {
		settings.gapOpenPenalty = gapOpenSpinBox->value();
	}
	if(gapExtensionPenaltyCheckBox->isChecked()) {
		settings.gapExtenstionPenalty = gapExtensionPenaltySpinBox->value();
	}
	if(terminalGapCheckBox->isChecked()) {
		settings.termGapPenalty = terminalGapSpinBox->value();
	}

    QDialog::accept();
}

bool KalignDialogController::translateToAmino()
{
    return translateCheckBox->isEnabled();
}

////////////////////////////////////////
//KalignWithExtFileSpecifyDialogController
KalignWithExtFileSpecifyDialogController::KalignWithExtFileSpecifyDialogController(QWidget* w, KalignTaskSettings& _settings)
: QDialog(w), settings(_settings)
{
    setupUi(this);
	setupUiExt();
    QWidget * widget = new QWidget(w);
    inputFileLineEdit= new FileLineEdit("","", false, widget);
    inputFileLineEdit->setReadOnly(false);
    inputFileLineEdit->setText("");
    QToolButton * selectToolPathButton = new QToolButton(widget);
    selectToolPathButton->setVisible(true);
    selectToolPathButton->setText("...");
    connect(selectToolPathButton, SIGNAL(clicked()), inputFileLineEdit, SLOT(sl_onBrowse()));
    connect(inputFileLineEdit,SIGNAL(textChanged(QString)),this, SLOT(sl_inputFileLineEditChanged(QString)));

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->addWidget(inputFileLineEdit);
    layout->addWidget(selectToolPathButton);

    QGroupBox* inputFileGroupBox=new QGroupBox(tr("Select input file"),widget);
    inputFileGroupBox->setLayout(layout);
    QBoxLayout* parentLayout = qobject_cast<QBoxLayout*>(this->layout());
    assert(parentLayout);
    parentLayout->insertWidget(0, inputFileGroupBox);
    alignButton->setEnabled(false);
    translateCheckBox->setEnabled(false);
}

void KalignWithExtFileSpecifyDialogController::setupUiExt() {
	gridLayout->removeWidget(gapOpenSpinBox);
	delete gapOpenSpinBox;
	gapOpenSpinBox = new QClearableDoubleSpinBox();
	gapOpenSpinBox->setObjectName(QString::fromUtf8("gapOpenSpinBox"));
	gapOpenSpinBox->setEnabled(false);
	gapOpenSpinBox->setValue(53.9);
	gridLayout->addWidget(gapOpenSpinBox, 0, 1, 1, 1);

	gridLayout->removeWidget(gapExtensionPenaltySpinBox);
	delete gapExtensionPenaltySpinBox;
	gapExtensionPenaltySpinBox = new QClearableDoubleSpinBox();
	gapExtensionPenaltySpinBox->setObjectName(QString::fromUtf8("gapExtensionPenaltySpinBox"));
	gapExtensionPenaltySpinBox->setEnabled(false);
	gapExtensionPenaltySpinBox->setValue(8.52);
	gridLayout->addWidget(gapExtensionPenaltySpinBox, 1, 1, 1, 1);

	gridLayout->removeWidget(terminalGapSpinBox);
	delete terminalGapSpinBox;
	terminalGapSpinBox = new QClearableDoubleSpinBox();
	terminalGapSpinBox->setObjectName(QString::fromUtf8("terminalGapSpinBox"));
	terminalGapSpinBox->setEnabled(false);
	terminalGapSpinBox->setValue(4.42);
	gridLayout->addWidget(terminalGapSpinBox, 2, 1, 1, 1);

	gridLayout->removeWidget(bonusScoreSpinBox);
	delete bonusScoreSpinBox;
	bonusScoreSpinBox = new QClearableDoubleSpinBox();
	bonusScoreSpinBox->setObjectName(QString::fromUtf8("bonusScoreSpinBox"));
	bonusScoreSpinBox->setEnabled(false);
	bonusScoreSpinBox->setValue(0.02);
	gridLayout->addWidget(bonusScoreSpinBox, 3, 1, 1, 1);

	QObject::connect(gapOpenCheckBox, SIGNAL(clicked(bool)), gapOpenSpinBox, SLOT(setEnabled(bool)));
	QObject::connect(gapExtensionPenaltyCheckBox, SIGNAL(clicked(bool)), gapExtensionPenaltySpinBox, SLOT(setEnabled(bool)));
	QObject::connect(terminalGapCheckBox, SIGNAL(clicked(bool)), terminalGapSpinBox, SLOT(setEnabled(bool)));
	QObject::connect(bonusScoreCheckBox, SIGNAL(clicked(bool)), bonusScoreSpinBox, SLOT(setEnabled(bool)));
}

void KalignWithExtFileSpecifyDialogController::sl_inputFileLineEditChanged(const QString& str){
    alignButton->setEnabled(!str.isEmpty());
}

void KalignWithExtFileSpecifyDialogController::accept() {
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtensionPenaltyCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtensionPenaltySpinBox->value();
    }
    if(terminalGapCheckBox->isChecked()) {
        settings.termGapPenalty = terminalGapSpinBox->value();
    }
    if(!inputFileLineEdit->text().isEmpty()){
        settings.inputFilePath=inputFileLineEdit->text();
    }else{
        assert(NULL);
        reject();
    }
    QDialog::accept();
}

}//namespace

