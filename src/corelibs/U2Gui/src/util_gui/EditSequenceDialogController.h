#ifndef _U2_EDIT_SEQUENCE_CONTROLLER_H_
#define _U2_EDIT_SEQUENCE_CONTROLLER_H_

#include <U2Core/U2Region.h>
#include <U2Core/U2AnnotationUtils.h>
#include <U2Core/GUrl.h>
#include <U2Gui/SeqPasterWidgetController.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QDialog>

class Ui_EditSequenceDialog;

namespace U2{

enum EditSequenceDialogMode {
    EditSequenceMode_Replace,
    EditSequenceMode_Insert
};


struct U2GUI_EXPORT EditSequencDialogConfig {
    EditSequenceDialogMode mode;
    U2Region source;
    DNAAlphabet* alphabet;
    QByteArray initialText;
};

class U2GUI_EXPORT EditSequenceDialogController : public QDialog {
    Q_OBJECT
public:
    EditSequenceDialogController(EditSequencDialogConfig cfg, QWidget *p = NULL);
    ~EditSequenceDialogController();
    virtual void accept();

    DNASequence getNewSequence(){return w->getSequence();};
    GUrl getDocumentPath(); 
    int getPosToInsert(){return pos;};
    U2AnnotationUtils::AnnotationStrategyForResize getAnnotationStrategy();
    bool mergeAnnotations();
    DocumentFormatId getDocumentFormatId();
private slots:
    void sl_browseButtonClicked();
    void sl_indexChanged(int);
    void sl_mergeAnnotationsToggled(bool);

private:
    void addSeqpasterWidget();
    bool modifyCurrentDocument();


    QString filter;
    int pos;
    SeqPasterWidgetController *w;
    EditSequencDialogConfig config;
    Ui_EditSequenceDialog* ui;
};

}//ns

#endif

