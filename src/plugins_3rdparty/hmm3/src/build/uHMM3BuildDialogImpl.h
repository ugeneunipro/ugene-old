#ifndef _GB2_UHMM3_BUILD_DLG_IMPL_H_
#define _GB2_UHMM3_BUILD_DLG_IMPL_H_

#include <QtGui/QDialog>

#include "uHMM3BuildTask.h"
#include <ui/ui_UHMM3BuildDialog.h>

namespace U2 {

struct UHMM3BuildDialogModel {
    UHMM3BuildTaskSettings  buildSettings;
    
    /* one of this is used */
    QString                 inputFile;
    MAlignment              alignment;
    bool                    alignmentUsing;
    
}; // UHMM3BuildDialogModel

class UHMM3BuildDialogImpl : public QDialog, public Ui_UHMM3BuildDialog {
Q_OBJECT
public:
    static const QString MA_FILES_DIR_ID;
    static const QString HMM_FILES_DIR_ID;
    
public:
    UHMM3BuildDialogImpl( const MAlignment & ma, QWidget * p = NULL );
    
private:
    void setModelValues();
    void getModelValues();
    QString checkModel(); /* return error or empty string */
    void setSignalsAndSlots();
    void initialize();
    
private slots:
    void sl_maOpenFileButtonClicked();
    void sl_outHmmFileButtonClicked();
    void sl_buildButtonClicked();
    void sl_cancelButtonClicked();
    void sl_fastMCRadioButtonChanged( bool checked );
    void sl_wblosumRSWRadioButtonChanged( bool checked );
    void sl_eentESWRadioButtonChanged( bool checked );
    void sl_eclustESWRadioButtonChanged( bool changed );
    void sl_esetESWRadioButtonChanged( bool checked );
    
private:
    UHMM3BuildDialogModel   model;
    
}; // UHMM3BuildDialogImpl

} // U2

#endif // _GB2_UHMM3_BUILD_DLG_IMPL_H_
