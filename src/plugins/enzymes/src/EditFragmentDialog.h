#ifndef _U2_EDIT_FRAGMENT_DIALOG_H_
#define _U2_EDIT_FRAGMENT_DIALOG_H_

#include "DNAFragment.h"

#include <ui/ui_EditFragmentDialog.h>

namespace U2 {

class DNATranslation;

class EditFragmentDialog : public QDialog, public Ui_EditFragmentDialog {
    Q_OBJECT
public:
   EditFragmentDialog(DNAFragment& fragment, QWidget* p);
   virtual void accept();
private:
   DNAFragment& dnaFragment;
   DNATranslation* transl;
   QString seq,trseq;
   void updatePreview();
   void resetLeftOverhang();
   void resetRightOverhang();
   bool isValidOverhang(const QString& text);
private slots:
   void sl_updatePreview();
   void sl_onLeftResetClicked();
   void sl_onRightResetClicked();
   void sl_customOverhangSet(const QString& text);

};


} //namespace

#endif
