#ifndef _U2_EDIT_ANNOTATION_DIALOG_CONTROLLER_H_
#define _U2_EDIT_ANNOTATION_DIALOG_CONTROLLER_H_

#include "ui/ui_EditAnnotationDialog.h"

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationTableObject.h>

namespace U2{

class U2VIEW_EXPORT EditAnnotationDialogController : public QDialog, public Ui_EditAnnotationDialog {
    Q_OBJECT
public:
    EditAnnotationDialogController(Annotation* a, U2Region _seqRange, QWidget* p = NULL);

    virtual void accept ();

    QString getName() const {return currentName;}
    U2Location getLocation() const {return location;}
    
private slots:
    void sl_onTextChanged(const QString&);
    void sl_setPredefinedAnnotationName();
    void sl_complementLocation();

private:
    QMenu* createAnnotationNamesMenu(QWidget* p, QObject* receiver);

    U2Region            seqRange;
    U2Location          location;
    QString             currentName;
};

}//ns

#endif //_U2_EDIT_ANNOTATION_DIALOG_CONTROLLER_H_
