#ifndef _U2_CREATE_FRAGMENT_DIALOG_H_
#define _U2_CREATE_FRAGMENT_DIALOG_H_

#include "DNAFragment.h"

#include <ui/ui_CreateFragmentDialog.h>

namespace U2 {

class CreateAnnotationWidgetController;
class ADVSequenceObjectContext;
class DNASequenceSelection;

class CreateFragmentDialog : public QDialog, public Ui_CreateFragmentDialog {
    Q_OBJECT
public:
    CreateFragmentDialog(ADVSequenceObjectContext* seqCtx, QWidget* p);
    CreateFragmentDialog(DNASequenceObject* seqObj, QWidget* p);
    virtual void accept();
    const DNAFragment& getFragment() { return dnaFragment; }
private:
    CreateAnnotationWidgetController* ac;
    DNASequenceObject*  seqObj;
    DNASequenceSelection* seqSelection;
    QList<AnnotationTableObject*> relatedAnnotations;
    QSet<QString>  enzymesSelection;
    void setupAnnotationsWidget();
    DNAFragment dnaFragment;
};


} //namespace

#endif //  _U2_CREATE_FRAGMENT_DIALOG_H_
