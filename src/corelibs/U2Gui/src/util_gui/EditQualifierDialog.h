#ifndef _U2_EDIT_QUALIFIER_DIALOG_H_
#define _U2_EDIT_QUALIFIER_DIALOG_H_

#include <U2Core/global.h>
#include <U2Core/AnnotationTableObject.h>

#include <QtGui/QDialog>

class Ui_EditQualifierDialog;

namespace U2 {

class U2GUI_EXPORT EditQualifierDialog: public QDialog {
    Q_OBJECT
public:
    EditQualifierDialog(QWidget* p, const U2Qualifier& q, bool ro, bool existingQualifier );
    ~EditQualifierDialog();
    
    virtual void accept();
    
    const U2Qualifier& getModifiedQualifier() const {return q;}

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    U2Qualifier q;
    Ui_EditQualifierDialog* ui;
    
};

}//namespace
#endif
