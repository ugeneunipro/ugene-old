#pragma once

#include <ui/ui_ExpertDiscoverySetupRecBoundDialog.h>

#include <vector>

namespace U2 {



class ExpertDiscoverySetupRecBoundDialog : public QDialog, public Ui_EDSetupRecBoundDlg{
    Q_OBJECT
public:
    ExpertDiscoverySetupRecBoundDialog(double dRecognizationBound, 
        const std::vector<double>& vPosScore, 
        const std::vector<double>& vNegScore);

    virtual void accept();

    double getRecognizationBound() const { return recognizationBound; }

private:
    double recognizationBound;
    double probNegRec;
    double probPosRej;
    const std::vector<double>& posScore;
    const std::vector<double>& negScore;

    void updateProbs();


protected slots:
    void sl_recBoundChaged(double val);
};

}//namespace
