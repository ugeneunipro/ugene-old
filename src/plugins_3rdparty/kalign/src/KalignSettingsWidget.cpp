#include "KalignSettingsWidget.h"
#include "KalignTask.h"


namespace U2 {

KalignSettingsWidget::KalignSettingsWidget(QWidget* parent) : MSAAlignAlgorithmMainWidget(parent) {
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);
}

QMap<QString,QVariant> KalignSettingsWidget::getMSAAlignCustomSettings() {
    QMap<QString,QVariant> settings;

    if (gapOpenCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_GAP_OPEN_PENALTY, gapOpenSpinBox->value());
    }

    if (gapExtensionPenaltyCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_GAP_EXTENSION_PENALTY, gapExtensionPenaltySpinBox->value());
    }

    if (terminalGapCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_TERMINAL_GAP_PENALTY, terminalGapSpinBox->value());
    }

    if (bonusScoreCheckBox->isChecked()) {
        settings.insert(KalignMainTask::OPTION_BONUS_SCORE, bonusScoreSpinBox->value());
    }    
    return settings;
}
} //namespace