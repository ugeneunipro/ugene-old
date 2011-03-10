#ifndef _SEND_SELECTION_DIALOG_H_
#define _SEND_SELECTION_DIALOG_H_

#include "ui/ui_RemoteBLASTDialog.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include <QtGui/QMessageBox>

#include "RemoteBLASTTask.h" 
#include "RemoteBLASTConsts.h"


namespace U2 {

class CreateAnnotationWidgetController;
class AnnotationTableObject;

class SendSelectionDialog: public QDialog, Ui_Dialog {
    Q_OBJECT
public:
    SendSelectionDialog( const DNASequenceObject* dnaso, bool _isAminoSeq, QWidget *p = NULL );
    QString getGroupName() const;
    AnnotationTableObject* getAnnotationObject() const;
    const CreateAnnotationModel *getModel() const;
    QString getUrl() const;
private slots:
    void sl_scriptSelected( int index );
    void sl_megablastChecked(int state);
    void sl_serviceChanged(int);
    void sl_OK();
    void sl_Cancel();

private:
    void setupDataBaseList();
    void setupAlphabet();
    void setUpSettings();
    void saveSettings();
    void alignComboBoxes();

public:
    int retries;
    QString db;
    QString requestParameters;
    bool translateToAmino;
    int filterResults;
    bool useEval;
    RemoteBLASTTaskSettings cfg;

private:
    bool isAminoSeq;
    CreateAnnotationWidgetController * ca_c;
    bool extImported;
};

}

#endif
