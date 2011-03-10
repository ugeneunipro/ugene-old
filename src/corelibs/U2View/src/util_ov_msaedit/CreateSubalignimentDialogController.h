#ifndef _U2_CREATE_SUBALIGNIMENT_DIALOG_CONTROLLER_H_
#define _U2_CREATE_SUBALIGNIMENT_DIALOG_CONTROLLER_H_

#include "ui/ui_CreateSubalignimentDialog.h"

#include <U2Core/global.h>

#include <U2Core/MAlignmentObject.h>

namespace U2{

class U2VIEW_EXPORT CreateSubalignimentDialogController : public QDialog, Ui_CreateSubalignimentDialog {
    Q_OBJECT
public:
    CreateSubalignimentDialogController( MAlignmentObject *_mobj, const QRect& selection, QWidget *p = NULL);

    void accept();

    bool getAddToProjFlag(){return addToProjBox->isChecked();};
    QString getSavePath(){return filepathEdit->text();};
    U2Region getRegion(){return window;};
    QStringList getSelectedSeqNames(){return selectedNames;};

private slots:
    void sl_browseButtonClicked();
    void sl_allButtonClicked();
    void sl_invertButtonClicked();
    void sl_noneButtonClicked();

private:
    void selectSeqNames();

    QString filter;
    MAlignmentObject *mobj;
    U2Region window;
    QStringList selectedNames;
};

}

#endif
