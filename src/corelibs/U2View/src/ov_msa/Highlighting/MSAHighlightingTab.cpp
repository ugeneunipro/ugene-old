/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/MSAColorScheme.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

#include "MSAHighlightingTab.h"

namespace U2 {

static const int ITEMS_SPACING = 6;
static const int TITLE_SPACING = 1;

static inline QVBoxLayout * initVBoxLayout(QWidget * w) {
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

static inline QHBoxLayout * initHBoxLayout(QWidget * w) {
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

QWidget* MSAHighlightingTab::createColorGroup(){
    QWidget * group = new QWidget(this);

    QVBoxLayout * layout = initVBoxLayout(group);
    colorScheme = new QComboBox();
    colorScheme->setObjectName("colorScheme");
    colorScheme->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    layout->addSpacing(TITLE_SPACING);
    layout->addWidget(colorScheme);
    layout->addSpacing(ITEMS_SPACING);

    return group;
}

QWidget* MSAHighlightingTab::createHighlightingGroup() {
    QWidget * group = new QWidget(this);

    QVBoxLayout * layout = initVBoxLayout(group);
    highlightingScheme = new QComboBox();
    highlightingScheme->setObjectName("highlightingScheme");

    hint = new QLabel("");
    hint->setWordWrap(true);

    useDots = new QCheckBox(tr("Use dots"));
    useDots->setObjectName("useDots");

    exportHighlightning = new QToolButton();
    exportHighlightning->setText(tr("Export"));
    exportHighlightning->setObjectName("exportHighlightning");

    QWidget *buttonAndSpacer = new QWidget(this);
    QHBoxLayout * layout2 = initHBoxLayout(buttonAndSpacer);
    layout2->addWidget(exportHighlightning);
    layout2->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    layout->setSpacing(ITEMS_SPACING);
    layout->addSpacing(TITLE_SPACING);
    layout->addWidget(highlightingScheme);
    layout->addWidget(hint);
    layout->addWidget(useDots);
#ifdef Q_OS_MAC
    layout->addSpacerItem(new QSpacerItem(40, 8, QSizePolicy::Expanding, QSizePolicy::Minimum));
#endif
    layout->addWidget(buttonAndSpacer);

    return group;
}

MSAHighlightingTab::MSAHighlightingTab(MSAEditor* m)
    : msa(m), savableTab(this, GObjectViewUtils::findViewByName(m->getName()))
{
    setObjectName("HighlightingOptionsPanelWidget");
    QVBoxLayout* mainLayout = initVBoxLayout(this);
    mainLayout->setSpacing(0);

    QWidget * colorGroup = new ShowHideSubgroupWidget("COLOR", tr("Color"), createColorGroup(), true);
    mainLayout->addWidget(colorGroup);

    QWidget * highlightingGroup = new ShowHideSubgroupWidget("HIGHLIGHTING", tr("Highlighting"), createHighlightingGroup(), true);
    mainLayout->addWidget(highlightingGroup);

    seqArea = msa->getUI()->getSequenceArea();

    initColorCB();
    sl_sync();
    connect(colorScheme, SIGNAL(currentIndexChanged(const QString &)), seqArea,
        SLOT(sl_changeColorSchemeOutside(const QString &)));
    connect(highlightingScheme, SIGNAL(currentIndexChanged(const QString &)), seqArea,
        SLOT(sl_changeColorSchemeOutside(const QString &)));
    connect(highlightingScheme, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_updateHint()));
    connect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_doUseDots()));

    connect(seqArea, SIGNAL(si_highlightingChanged()), SLOT(sl_sync()));

    connect(m, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_updateHint()));

    connect(exportHighlightning, SIGNAL(clicked()), SLOT(sl_exportHighlightningClicked()));

    sl_updateHint();

    U2WidgetStateStorage::restoreWidgetState(savableTab);
}

void MSAHighlightingTab::initColorCB(){
    colorScheme->clear();
    colorScheme->addItems(seqArea->getAvailableColorSchemes());

    highlightingScheme->clear();
    highlightingScheme->addItems(seqArea->getAvailableHighlightingSchemes());
}

void MSAHighlightingTab::sl_sync(){
    //check custom schemes changed
    if(seqArea->getAvailableColorSchemes().size() != colorScheme->count()){
        disconnect(colorScheme, SIGNAL(currentIndexChanged(const QString &)), seqArea, SLOT(sl_changeColorSchemeOutside(const QString &)));
        initColorCB();
        connect(colorScheme, SIGNAL(currentIndexChanged(const QString &)), seqArea, SLOT(sl_changeColorSchemeOutside(const QString &)));
    }

    MSAColorScheme *s = seqArea->getCurrentColorScheme();
    SAFE_POINT(s != NULL, "Current scheme is NULL", );
    SAFE_POINT(s->getFactory() != NULL, "Current scheme color factory is NULL", );
    colorScheme->setCurrentIndex(colorScheme->findText(s->getFactory()->getName()));

    MSAHighlightingScheme *sh = seqArea->getCurrentHighlightingScheme();
    SAFE_POINT(sh != NULL, "Current highlighting scheme is NULL!", );
    SAFE_POINT(sh->getFactory() != NULL, "Current highlighting scheme factory is NULL!", );
    highlightingScheme->setCurrentIndex(highlightingScheme->findText(sh->getFactory()->getName()));


    disconnect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_doUseDots())); //disconnect-connect to prevent infinite loop
    useDots->setChecked(seqArea->getUseDotsCheckedState());
    connect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_doUseDots()));
    sl_updateHint();
}

void MSAHighlightingTab::sl_updateHint() {
    if (MAlignmentRow::invalidRowId() == msa->getReferenceRowId()
        && !seqArea->getCurrentHighlightingScheme()->getFactory()->isRefFree())
    {
        hint->setText(tr("Hint: select a reference above"));
        hint->setStyleSheet(
            "color: green;"
            "font: bold;");
        exportHighlightning->setDisabled(true);
        return;
    }
    hint->setText("");
    MSAHighlightingScheme *s = seqArea->getCurrentHighlightingScheme();
    if(s->getFactory()->isRefFree()){
        exportHighlightning->setDisabled(true);
    }else{
        exportHighlightning->setEnabled(true);
    }
}

void MSAHighlightingTab::sl_exportHighlightningClicked(){
    msa->exportHighlighted();
}

}//ns
