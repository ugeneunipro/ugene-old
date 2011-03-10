#ifndef _U2_SMITH_WATERMAN_DIALOG_IMPL_H_
#define _U2_SMITH_WATERMAN_DIALOG_IMPL_H_

#include "SmithWatermanDialog.h"
#include <ui/ui_SmithWatermanDialogBase.h>

#include <QDialog>
#include <QCloseEvent>

#include <U2Algorithm/SmithWatermanTaskFactory.h>
#include <U2Core/SequenceWalkerTask.h>

#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/SWResultFilterRegistry.h>
#include <U2Algorithm/SmithWatermanTaskFactoryRegistry.h>

namespace U2 {

class SmithWatermanDialog: public QDialog, public Ui::SmithWatermanDialogBase {
    Q_OBJECT
public:
    SmithWatermanDialog(QWidget* p, 
                        ADVSequenceObjectContext* ctx,
                        SWDialogConfig* dialogConfig);

private slots:
    void sl_bttnViewMatrix();

    void sl_bttnRun();
    void sl_spinRangeStartChanged(int val);
    void sl_spinRangeEndChanged(int val);
    void sl_translationToggled(bool toggled);

    void sl_wholeSequenceToggled(bool toggled);
    void sl_selectedRangeToggled(bool toggled);
    void sl_customRangeToggled(bool toggled);
    
    //void sl_remoteRunButtonClicked();
    void sl_patternChanged();
    
private:
    void clearAll();
    void loadDialogConfig();
    void saveDialogConfig();
    bool readParameters();
    void updateVisualState();

    bool readPattern(DNATranslation* aminoTT);
    bool readRegion();
    bool readRealization();
    bool readSubstMatrix();
    bool readResultFilter();
    bool readGapModel();

    void setParameters();
    void connectGUI();
    void addAnnotationWidget();

    SubstMatrixRegistry* substMatrixRegistry;
    SWResultFilterRegistry* swResultFilterRegistry;
    SmithWatermanTaskFactoryRegistry* swTaskFactoryRegistry;

    SWDialogConfig* dialogConfig;
    SmithWatermanSettings config;
    SmithWatermanTaskFactory* realization;

    ADVSequenceObjectContext* ctxSeq;
    CreateAnnotationWidgetController* ac;
};

} // namespace

#endif

