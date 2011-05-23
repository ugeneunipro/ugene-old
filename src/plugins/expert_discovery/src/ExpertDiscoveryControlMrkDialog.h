#pragma once

#include <ui/ui_ExpertDiscoveryControlMrkDialog.h>

namespace U2 {

class ExpertDiscoveryControlMrkDialog : public QDialog, public Ui_ExpertDiscoveryControlMrkDialog{
    Q_OBJECT
public:
    ExpertDiscoveryControlMrkDialog(QWidget *parent);

    virtual void accept();

    QString getFirstFileName() const {return firstFileName;}
protected slots:
    void sl_openFirstFile();

private:
    QString firstFileName;


    QString filter;

};

}//namespace
