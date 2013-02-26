#include "PairwiseAlignmentHirschbergGUIExtensionFactory.h"
#include "PairwiseAlignmentHirschbergTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/U2SafePoints.h>

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLayout>

#include <QtCore/QList>
#include <QtCore/QString>

namespace U2 {

PairwiseAlignmentHirschbergMainWidget::PairwiseAlignmentHirschbergMainWidget(QWidget *parent, QVariantMap *s, WidgetType _widgetType) :
    PairwiseAlignmentMainWidget(parent, s, _widgetType) {
    switch (widgetType) {
    //add subwidgets
    //fill custom settings
    case OptionsPanelWidgetType: {
        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setContentsMargins(0, 0, 0, 0);;
        mainLayout->setSpacing(3);
        mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QVBoxLayout* gapOpenLayout = new QVBoxLayout;
        gapOpenLayout->setContentsMargins(0, 0, 0, 0);
        gapOpenLayout->setSpacing(3);
        gapOpenLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QLabel* gapOpenLabel = new QLabel(tr("Gap open penalty"));

        gapOpen = new QDoubleSpinBox;
        gapOpen->setMinimum(H_MIN_GAP_OPEN);
        gapOpen->setMaximum(H_MAX_GAP_OPEN);

        QVBoxLayout* gapExtdLayout = new QVBoxLayout;
        gapExtdLayout->setContentsMargins(0, 0, 0, 0);
        gapExtdLayout->setSpacing(3);
        gapExtdLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QLabel* gapExtdLabel = new QLabel(tr("Gap extension penalty"));

        gapExtd = new QDoubleSpinBox;
        gapExtd->setMinimum(H_MIN_GAP_EXTD);
        gapExtd->setMaximum(H_MAX_GAP_EXTD);

        QVBoxLayout* gapTermLayout = new QVBoxLayout;
        gapTermLayout->setContentsMargins(0, 0, 0, 0);
        gapTermLayout->setSpacing(3);
        gapTermLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QLabel* gapTermLabel = new QLabel(tr("Terminate gap penalty"));

        gapTerm = new QDoubleSpinBox;
        gapTerm->setMinimum(H_MIN_GAP_TERM);
        gapTerm->setMaximum(H_MAX_GAP_TERM);

        QVBoxLayout* bonusScoreLayout = new QVBoxLayout;
        bonusScoreLayout->setContentsMargins(0, 0, 0, 0);
        bonusScoreLayout->setSpacing(3);
        bonusScoreLayout->setSizeConstraint(QLayout::SetMinimumSize);

        QLabel* bonusScoreLabel = new QLabel(tr("Bonus score"));

        bonusScore = new QDoubleSpinBox;
        bonusScore->setMinimum(H_MIN_BONUS_SCORE);
        bonusScore->setMaximum(H_MAX_BONUS_SCORE);

        QVBoxLayout* translateLayout = new QVBoxLayout;
        translateLayout->setContentsMargins(0, 0, 0, 0);
        translateLayout->setSpacing(5);
        translateLayout->setSizeConstraint(QLayout::SetMinimumSize);

        translateToAminoCheckBox = new QCheckBox(tr("Translate to amino"));
        connect(translateToAminoCheckBox, SIGNAL(stateChanged(int)), SLOT(sl_translateToAminoChecked(int)));

        QLabel* translateLabel = new QLabel(tr("Translation table:"));

        translationTableComboBox = new QComboBox;
        translationTableComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        translationTableComboBox->setMaximumSize(parent->size());
        translationTableComboBox->resize(parent->width(), translationTableComboBox->height());


        gapOpenLayout->addWidget(gapOpenLabel);
        gapOpenLayout->addWidget(gapOpen);

        gapExtdLayout->addWidget(gapExtdLabel);
        gapExtdLayout->addWidget(gapExtd);

        gapTermLayout->addWidget(gapTermLabel);
        gapTermLayout->addWidget(gapTerm);

        bonusScoreLayout->addWidget(bonusScoreLabel);
        bonusScoreLayout->addWidget(bonusScore);

        translateLayout->addWidget(translateToAminoCheckBox);
        translateLayout->addWidget(translateLabel);
        translateLayout->addWidget(translationTableComboBox);

        mainLayout->addLayout(gapOpenLayout);
        mainLayout->addLayout(gapExtdLayout);
        mainLayout->addLayout(gapTermLayout);
        mainLayout->addLayout(bonusScoreLayout);
        mainLayout->addLayout(translateLayout);
        this->setLayout(mainLayout);

        double defaultGapOpen;
        double defaultGapExtd;
        double defaultGapTerm;
        double defaultBonusScore;
        DNAAlphabetRegistry* alphabetReg = AppContext::getDNAAlphabetRegistry();
        SAFE_POINT(NULL != alphabetReg, "DNAAlphabetRegistry is NULL.", );
        DNAAlphabet* alphabet = alphabetReg->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
        SAFE_POINT(NULL != alphabet, QString("Alphabet %1 not found").arg(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()), );

        if (alphabet->isNucleic()) {
            defaultGapOpen = H_DEFAULT_GAP_OPEN_DNA;
            defaultGapExtd = H_DEFAULT_GAP_EXTD_DNA;
            defaultGapTerm = H_DEFAULT_GAP_TERM_DNA;
            defaultBonusScore = H_DEFAULT_BONUS_SCORE_DNA;
        } else {
            defaultGapOpen = H_DEFAULT_GAP_OPEN;
            defaultGapExtd = H_DEFAULT_GAP_EXTD;
            defaultGapTerm = H_DEFAULT_GAP_TERM;
            defaultBonusScore = H_DEFAULT_BONUS_SCORE;
        }


        if (s->contains(PA_H_GAP_OPEN) && s->value(PA_H_GAP_OPEN, 0).toInt() >= H_MIN_GAP_OPEN && s->value(PA_H_GAP_OPEN, 0).toInt() <= H_MAX_GAP_OPEN) {
            gapOpen->setValue(s->value(PA_H_GAP_OPEN, 0).toInt());
        } else {
            gapOpen->setValue(defaultGapOpen);
        }

        if (s->contains(PA_H_GAP_EXTD) && s->value(PA_H_GAP_EXTD, 0).toInt() >= H_MIN_GAP_EXTD && s->value(PA_H_GAP_EXTD, 0).toInt() <= H_MAX_GAP_EXTD) {
            gapExtd->setValue(s->value(PA_H_GAP_EXTD, 0).toInt());
        } else {
            gapExtd->setValue(defaultGapExtd);
        }

        if (s->contains(PA_H_GAP_TERM) && s->value(PA_H_GAP_TERM, 0).toInt() >= H_MIN_GAP_TERM && s->value(PA_H_GAP_TERM, 0).toInt() <= H_MAX_GAP_TERM) {
            gapTerm->setValue(s->value(PA_H_GAP_EXTD, 0).toInt());
        } else {
            gapTerm->setValue(defaultGapTerm);
        }

        if (s->contains(PA_H_BONUS_SCORE) && s->value(PA_H_BONUS_SCORE, 0).toInt() >= H_MIN_BONUS_SCORE && s->value(PA_H_BONUS_SCORE, 0).toInt() <= H_MAX_BONUS_SCORE) {
            bonusScore->setValue(s->value(PA_H_BONUS_SCORE, 0).toInt());
        } else {
            bonusScore->setValue(defaultBonusScore);
        }

        translateToAminoCheckBox->setChecked(s->value(PA_H_TRANSLATE_TO_AMINO, false).toBool());

        DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
        SAFE_POINT(NULL != tr, "DNATranslationRegistry is NULL.", );
        QList<DNATranslation*> aminoTs = tr->lookupTranslation(alphabet, DNATranslationType_NUCL_2_AMINO);
        SAFE_POINT(false == aminoTs.empty(), "No DNA translations found.", );
        foreach(DNATranslation* t, aminoTs) {
            translationTableComboBox->addItem(t->getTranslationName());
        }

        if (s->contains(PA_H_TRANSLATION_TABLE_NAME)) {
            translationTableComboBox->setCurrentIndex(translationTableComboBox->findText(s->value(PA_H_TRANSLATION_TABLE_NAME, QString()).toString()));
        }


        innerSettings.insert(PA_H_GAP_OPEN, gapOpen->value());
        innerSettings.insert(PA_H_GAP_EXTD, gapExtd->value());
        innerSettings.insert(PA_H_GAP_TERM, gapTerm->value());
        innerSettings.insert(PA_H_BONUS_SCORE, bonusScore->value());
        innerSettings.insert(PA_REALIZATION_NAME, "KAlign");
        innerSettings.insert(PA_H_REALIZATION_NAME, "KAlign");
        innerSettings.insert(PA_H_TRANSLATE_TO_AMINO, translateToAminoCheckBox->isChecked());
        innerSettings.insert(PA_H_TRANSLATION_TABLE_NAME, translationTableComboBox->currentText());
        break;
    }
    case DialogWidgetType: {
        break;
    }
    default: {
        assert(0);
        break;
    }
    }   //switch (type)
}

PairwiseAlignmentHirschbergMainWidget::~PairwiseAlignmentHirschbergMainWidget() {
    getPairwiseAlignmentCustomSettings(true);
}

QMap<QString, QVariant> PairwiseAlignmentHirschbergMainWidget::getPairwiseAlignmentCustomSettings(bool append) {
    fillInnerSettings();
    return PairwiseAlignmentMainWidget::getPairwiseAlignmentCustomSettings(append);
}

void PairwiseAlignmentHirschbergMainWidget::sl_translateToAminoChecked(int newState) {
    translationTableComboBox->setEnabled(newState);
    innerSettings.insert(PA_H_TRANSLATE_TO_AMINO, translateToAminoCheckBox->isChecked());
}

void PairwiseAlignmentHirschbergMainWidget::fillInnerSettings() {
    innerSettings.insert(PA_H_GAP_OPEN, gapOpen->value());
    innerSettings.insert(PA_H_GAP_EXTD, gapExtd->value());
    innerSettings.insert(PA_H_GAP_TERM, gapTerm->value());
    innerSettings.insert(PA_H_BONUS_SCORE, bonusScore->value());
    innerSettings.insert(PA_H_TRANSLATE_TO_AMINO, translateToAminoCheckBox->isChecked());
    innerSettings.insert(PA_H_TRANSLATION_TABLE_NAME, translationTableComboBox->currentText());
}


PairwiseAlignmentHirschbergGUIExtensionFactory::PairwiseAlignmentHirschbergGUIExtensionFactory() :
    PairwiseAlignmentGUIExtensionFactory() {
}

PairwiseAlignmentHirschbergGUIExtensionFactory::~PairwiseAlignmentHirschbergGUIExtensionFactory() {
}

PairwiseAlignmentMainWidget* PairwiseAlignmentHirschbergGUIExtensionFactory::createMainWidget(QWidget *parent,
    QVariantMap *s, WidgetType widgetType) {
    if (mainWidgets.contains(parent)) {
        return mainWidgets.value(parent, NULL);
    }
    PairwiseAlignmentHirschbergMainWidget* newMainWidget = new PairwiseAlignmentHirschbergMainWidget(parent, s, widgetType);
    connect(newMainWidget, SIGNAL(destroyed(QObject*)), SLOT(sl_widgetDestroyed(QObject*)));
    mainWidgets.insert(parent, newMainWidget);
    return newMainWidget;
}

}   //namespace
