/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Gui/DialogUtils.h>

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

KalignDialogController::KalignDialogController(QWidget* w, const MAlignment& _ma, KalignTaskSettings& _settings, bool translateEnabled) 
: QDialog(w), ma(_ma), settings(_settings)
{
    setupUi(this);
	setupUiExt();
    translateCheckBox->setEnabled(ma.getAlphabet()->isNucleic() && translateEnabled);
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
    return translateCheckBox->isChecked();
}

}//namespace

