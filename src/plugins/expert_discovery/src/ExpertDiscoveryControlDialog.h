#pragma once

#include <ui/ui_ExpertDiscoveryControlDialog.h>

namespace U2 {

class ExpertDiscoveryControlDialog : public QDialog, public Ui_ExpertDiscoveryControlDialog{
    Q_OBJECT
public:
    ExpertDiscoveryControlDialog(QWidget *parent);

    virtual void accept();

    QString getFirstFileName() const {return firstFileName;}
protected slots:
    void sl_openFirstFile();

private:
    QString firstFileName;


    QString filter;

};

}//namespace
