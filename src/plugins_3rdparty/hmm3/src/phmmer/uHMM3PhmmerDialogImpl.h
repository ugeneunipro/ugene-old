#ifndef _GB2_UHMM3_PHMMER_DIALOG_IMPL_H_
#define _GB2_UHMM3_PHMMER_DIALOG_IMPL_H_

#include <QtGui/QDialog>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequence.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include <phmmer/uhmm3phmmer.h>
#include <ui/ui_UHMM3PhmmerDialog.h>

namespace U2 {

class UHMM3PhmmerDialogModel {
public:
    UHMM3PhmmerSettings phmmerSettings;
    QString             queryfile;
    DNASequence         dbSequence;
}; // UHMM3PhmmerDialogModel

class UHMM3PhmmerDialogImpl : public QDialog, public Ui_UHMM3PhmmerDialog {
    Q_OBJECT
private:
    static const QString    QUERY_FILES_DIR;
    static const QString    DOM_E_PLUS_PREFIX;
    static const QString    DOM_E_MINUS_PREFIX;
    static const QString    ANNOTATIONS_DEFAULT_NAME;
    static const int        ANNOTATIONS_WIDGET_LOCATION = 1;
public:
    UHMM3PhmmerDialogImpl( const DNASequenceObject * seqObj, QWidget * p = NULL );
    
private:
    void setModelValues();
    void getModelValues();
    QString checkModel();
    
private slots:
    void sl_queryToolButtonClicked();
    void sl_cancelButtonClicked();
    void sl_okButtonClicked();
    void sl_useEvalTresholdsButtonChanged( bool checked );
    void sl_useScoreTresholdsButtonChanged( bool checked );
    void sl_domZCheckBoxChanged( int state );
    void sl_maxCheckBoxChanged( int state );
    void sl_domESpinBoxChanged( int newVal );
    
private:
    UHMM3PhmmerDialogModel model;
    CreateAnnotationWidgetController *  annotationsWidgetController;
    
}; // UHMM3PhmmerDialogImpl

} // U2

#endif // _GB2_UHMM3_PHMMER_DIALOG_IMPL_H_
