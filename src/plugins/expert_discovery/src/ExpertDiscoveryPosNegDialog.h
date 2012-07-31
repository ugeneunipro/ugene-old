#pragma once

#include <ui/ui_ExpertDiscoveryPosNegDialog.h>

namespace U2 {

class ExpertDiscoveryPosNegDialog : public QDialog, public Ui_ExpertDiscoveryPosNegDialog{
    Q_OBJECT
public:
    ExpertDiscoveryPosNegDialog(QWidget *parent);

    virtual void accept();

    QString getFirstFileName() const {return firstFileName;}
    QString getSecondFileName() const {return secondFileName;}
    bool isGenerateNegative() const {return generateNeg;}
    int getNegativePerPositive() const {return negativePerPositive;}
protected slots:
    void sl_openFirstFile();
    void sl_openSecondFile();
    void sl_oneSequence();

private:
    QString firstFileName;
    QString secondFileName;

    bool generateNeg;
    int negativePerPositive;

    QString filter;

};

}//namespace
