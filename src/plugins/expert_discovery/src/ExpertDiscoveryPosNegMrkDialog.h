#pragma once

#include <ui/ui_ExpertDiscoveryPosNegMrkDialog.h>

namespace U2 {

class ExpertDiscoveryPosNegMrkDialog : public QDialog, public Ui_ExpertDiscoveryPosNegMrkDialog{
	Q_OBJECT
public:
	ExpertDiscoveryPosNegMrkDialog(QWidget *parent);

	virtual void accept();

	QString getFirstFileName() const {return firstFileName;}
	QString getSecondFileName() const {return secondFileName;}
    QString getThirdFileName() const {return thirdFileName;}
	bool isGenerateDescr() const {return generateDescr;}
    bool isAppendToCurrentMarkup() const {return addToCurrentCheck->isChecked();}
protected slots:
	void sl_openFirstFile();
	void sl_openSecondFile();
    void sl_openThirdFile();
	void sl_oneSequence();

private:
	QString firstFileName;
	QString secondFileName;
    QString thirdFileName;

	bool generateDescr;

	QString filter;
	
};

}//namespace
