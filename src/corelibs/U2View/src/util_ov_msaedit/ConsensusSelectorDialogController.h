#ifndef _U2_CONSENSUS_SELECTOR_DIALOG_CONTROLLER_H_
#define _U2_CONSENSUS_SELECTOR_DIALOG_CONTROLLER_H_

#include "ui/ui_ConsensusSelectorDialog.h"

#include <U2Core/global.h>
#include <U2Algorithm/MSAConsensusAlgorithm.h>

namespace U2{

class U2VIEW_EXPORT ConsensusSelectorDialogController : public QDialog, Ui_ConsensusSelectorDialog { 
    Q_OBJECT
public:
    ConsensusSelectorDialogController(const QString& selectedAlgorithmId, ConsensusAlgorithmFlags flags, QWidget *p = NULL);

    const QString& getSelectedAlgorithmId() const {return selectedAlgorithmId;}

    // enables threshold selector group, sets threshold range to 'r' 
    // and current value for slider and spin box to 'value'
    void enableThresholdSelector(int minVal, int maxVal, int value, const QString& suffix);
    
    // disables threshold selector group
    void disableThresholdSelector();
    
    int getThresholdValue() const;

signals:
    void si_algorithmChanged(const QString& algoId);
    
    void si_thresholdChanged(int val);

private slots:
    void sl_algorithmSelectionChanged(int);
    void sl_thresholdSliderChanged(int);
    void sl_thresholdSpinChanged(int);
    void sl_resetThresholdClicked(bool);

private:
    void updateSelectedAlgorithmDesc();

    QString selectedAlgorithmId;
};

}

#endif
