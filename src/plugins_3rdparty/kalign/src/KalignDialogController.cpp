#include "KalignDialogController.h"

#include <U2Core/AppContext.h>
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
}

void KalignDialogController::setupUiExt() {
	gridLayout_2->removeWidget(gapOpenSpinBox);
	delete gapOpenSpinBox;
	gapOpenSpinBox = new QClearableDoubleSpinBox();
	gapOpenSpinBox->setObjectName(QString::fromUtf8("gapOpenSpinBox"));
	gapOpenSpinBox->setEnabled(false);
	gapOpenSpinBox->setValue(53.9);
	gridLayout_2->addWidget(gapOpenSpinBox, 1, 1, 1, 1);

	gridLayout_2->removeWidget(gapExtensionPenaltySpinBox);
	delete gapExtensionPenaltySpinBox;
	gapExtensionPenaltySpinBox = new QClearableDoubleSpinBox();
	gapExtensionPenaltySpinBox->setObjectName(QString::fromUtf8("gapExtensionPenaltySpinBox"));
	gapExtensionPenaltySpinBox->setEnabled(false);
	gapExtensionPenaltySpinBox->setValue(8.52);
	gridLayout_2->addWidget(gapExtensionPenaltySpinBox, 2, 1, 1, 1);

	gridLayout_2->removeWidget(terminalGapSpinBox);
	delete terminalGapSpinBox;
	terminalGapSpinBox = new QClearableDoubleSpinBox();
	terminalGapSpinBox->setObjectName(QString::fromUtf8("terminalGapSpinBox"));
	terminalGapSpinBox->setEnabled(false);
	terminalGapSpinBox->setValue(4.42);
	gridLayout_2->addWidget(terminalGapSpinBox, 3, 1, 1, 1);

	gridLayout_2->removeWidget(bonusScoreSpinBox);
	delete bonusScoreSpinBox;
	bonusScoreSpinBox = new QClearableDoubleSpinBox();
	bonusScoreSpinBox->setObjectName(QString::fromUtf8("bonusScoreSpinBox"));
	bonusScoreSpinBox->setEnabled(false);
	bonusScoreSpinBox->setValue(0.02);
	gridLayout_2->addWidget(bonusScoreSpinBox, 4, 1, 1, 1);

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
}

void KalignWithExtFileSpecifyDialogController::setupUiExt() {
	gridLayout_2->removeWidget(gapOpenSpinBox);
	delete gapOpenSpinBox;
	gapOpenSpinBox = new QClearableDoubleSpinBox();
	gapOpenSpinBox->setObjectName(QString::fromUtf8("gapOpenSpinBox"));
	gapOpenSpinBox->setEnabled(false);
	gapOpenSpinBox->setValue(53.9);
	gridLayout_2->addWidget(gapOpenSpinBox, 1, 1, 1, 1);

	gridLayout_2->removeWidget(gapExtensionPenaltySpinBox);
	delete gapExtensionPenaltySpinBox;
	gapExtensionPenaltySpinBox = new QClearableDoubleSpinBox();
	gapExtensionPenaltySpinBox->setObjectName(QString::fromUtf8("gapExtensionPenaltySpinBox"));
	gapExtensionPenaltySpinBox->setEnabled(false);
	gapExtensionPenaltySpinBox->setValue(8.52);
	gridLayout_2->addWidget(gapExtensionPenaltySpinBox, 2, 1, 1, 1);

	gridLayout_2->removeWidget(terminalGapSpinBox);
	delete terminalGapSpinBox;
	terminalGapSpinBox = new QClearableDoubleSpinBox();
	terminalGapSpinBox->setObjectName(QString::fromUtf8("terminalGapSpinBox"));
	terminalGapSpinBox->setEnabled(false);
	terminalGapSpinBox->setValue(4.42);
	gridLayout_2->addWidget(terminalGapSpinBox, 3, 1, 1, 1);

	gridLayout_2->removeWidget(bonusScoreSpinBox);
	delete bonusScoreSpinBox;
	bonusScoreSpinBox = new QClearableDoubleSpinBox();
	bonusScoreSpinBox->setObjectName(QString::fromUtf8("bonusScoreSpinBox"));
	bonusScoreSpinBox->setEnabled(false);
	bonusScoreSpinBox->setValue(0.02);
	gridLayout_2->addWidget(bonusScoreSpinBox, 4, 1, 1, 1);

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

