#ifndef _GB2_UHMM3_SEARCH_DLG_IMPL_H_
#define _GB2_UHMM3_SEARCH_DLG_IMPL_H_

#include <QtGui/QDialog>
#include <QtGui/QButtonGroup>

#include <U2Core/DNASequence.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Core/DNASequenceObject.h>

#include <ui/ui_UHMM3SearchDialog.h>
#include <search/uHMM3SearchTask.h>

namespace U2 {

struct UHMM3SearchDialogModel {
    UHMM3SearchTaskSettings searchSettings;
    QString                 hmmfile;
    DNASequence             sequence;
}; // UHMM3SearchDialogModel

class UHMM3SearchDialogImpl : public QDialog, public Ui_UHMM3SearchDialog {
    Q_OBJECT
public:
    static const QString DOM_E_PLUS_PREFIX;
    static const QString DOM_E_MINUS_PREFIX;
    static const QString HMM_FILES_DIR_ID;
    static const QString ANNOTATIONS_DEFAULT_NAME;
    
public:
    UHMM3SearchDialogImpl( const DNASequenceObject * seqObj, QWidget * p = NULL );
    
private:
    void setModelValues();
    void getModelValues();
    QString checkModel();
    
private slots:
    void sl_cancelButtonClicked();
    void sl_okButtonClicked();
    void sl_useEvalTresholdsButtonChanged( bool checked );
    void sl_useScoreTresholdsButtonChanged( bool checked );
    void sl_useExplicitScoreTresholdButton( bool checked );
    void sl_maxCheckBoxChanged( int state );
    void sl_domESpinBoxChanged( int newVal );
    void sl_queryHmmFileToolButtonClicked();
    void sl_domZCheckBoxChanged( int state );
    
private:
    QButtonGroup                        useScoreTresholdGroup;
    CreateAnnotationWidgetController *  annotationsWidgetController;
    UHMM3SearchDialogModel              model;
    
}; // UHMM3SearchDialogImpl

} // U2

#endif // _GB2_UHMM3_SEARCH_DLG_IMPL_H_
