#ifndef _U2_STRUCTURAL_ALIGNMENT_DIALOG_H_
#define _U2_STRUCTURAL_ALIGNMENT_DIALOG_H_

#include "ui_StructuralAlignmentDialog.h"

namespace U2 {

class BioStruct3DObject;


class StructuralAlignmentDialog : public QDialog, public Ui::StructuralAlignmentDialog
{
    Q_OBJECT

public:
    StructuralAlignmentDialog(const QList<BioStruct3DObject*> biostructs, const BioStruct3DObject *fixedRef = 0, int fixedRefModel = 0, QWidget *parent = 0);

public slots:
    virtual void accept();

private slots:
    virtual void sl_biostructChanged(int idx);
    
private:
    static void createModelList(QComboBox *biostruct, int idx, QComboBox *model);
    void createModelLists();
};

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_DIALOG_H_
