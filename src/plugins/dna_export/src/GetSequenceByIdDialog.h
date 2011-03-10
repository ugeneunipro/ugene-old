#ifndef GET_SEQUENCE_BY_ID_
#define GET_SEQUENCE_BY_ID_

#include "ui/ui_GetSequenceByIdDialog.h"

namespace U2 {

class GetSequenceByIdDialog: public QDialog, public Ui_getSequenceByIdDialog{
    Q_OBJECT

public:
    GetSequenceByIdDialog(QWidget *w = NULL);
    void accept();
    bool isAddToProject() const {return addToProject;}
    QString getDirectory() const {return dir;}

private slots:
    void sl_saveFilenameButtonClicked();

private:
    QString dir;
    bool addToProject;

};


}


#endif