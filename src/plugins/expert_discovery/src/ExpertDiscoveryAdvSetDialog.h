#pragma once

#include <ui/ui_ExpertDiscoveryAdvSetDialog.h>

#include <QLineEdit>

namespace U2 {

class ExpertDiscoveryAdvSetDialog : public QDialog, public Ui_ExpertDiscoveryAdvSetDialog{
	Q_OBJECT
public:
	ExpertDiscoveryAdvSetDialog(QWidget *parent,
		double&	dIntProbability, 
		double&	dIntFisher,
		int&	nMinComplexity,
		int&	nMaxComplexity,
		double&	dMinPosCorrelation,
		double&	dMaxPosCorrelation,
		double&	dMinNegCorrelation,
		double&	dMaxNegCorrelation,
		bool&	bCorrelationImportant);

	virtual void accept();

private:
	double&	dIntProbability;
	double&	dIntFisher;
	int&	nMinComplexity;
	int&	nMaxComplexity;
	double&	dMinPosCorrelation;
	double&	dMaxPosCorrelation;
	double&	dMinNegCorrelation;
	double&	dMaxNegCorrelation;
	bool&	bCorrelationImportant;

	bool check(const QLineEdit* lineE);

protected slots:
	void sl_checkCorrel();
};

}//namespace
