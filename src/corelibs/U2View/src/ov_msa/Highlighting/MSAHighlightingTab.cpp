/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "MSAHighlightingTab.h"

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2View/MSAColorScheme.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2Core/AppContext.h>

namespace U2{

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

QWidget* MSAHighlightingTab::createColorGroup(){
    QWidget * group = new QWidget(this);

    QVBoxLayout * layout = initVBoxLayout(group);
    colorScheme = new QComboBox();
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
    highlightingScheme->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    hint = new QLabel("");
    hint->setWordWrap(true);

    useDots = new QCheckBox(tr("Use dots"));

    layout->addSpacing(TITLE_SPACING);  
    layout->addWidget(highlightingScheme);
    layout->addWidget(hint);
    layout->addWidget(useDots);

    return group;
}

MSAHighlightingTab::MSAHighlightingTab(MSAEditor* m):msa(m){
    QVBoxLayout* mainLayout = initVBoxLayout(this);
    mainLayout->setSpacing(0);

    QWidget * colorGroup = new ShowHideSubgroupWidget("COLOR", tr("Color"), createColorGroup(), true);
    mainLayout->addWidget(colorGroup);

    QWidget * highlightingGroup = new ShowHideSubgroupWidget("HIGHLIGHTING", tr("Highlighting"), createHighlightingGroup(), true);
    mainLayout->addWidget(highlightingGroup);

    seqArea = msa->getUI()->getSequenceArea();

    initColorCB();
    sl_sync();
    connect(colorScheme, SIGNAL(currentIndexChanged(const QString &)), seqArea, SLOT(sl_changeColorSchemeOutside(const QString &)));
    connect(highlightingScheme, SIGNAL(currentIndexChanged(const QString &)), seqArea, SLOT(sl_changeColorSchemeOutside(const QString &)));
    connect(highlightingScheme, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_updateHint(const QString &)));
    connect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_useDots(int)));

    connect(seqArea, SIGNAL(si_highlightingChanged()), SLOT(sl_sync()));

    connect(m, SIGNAL(si_referenceSeqChanged(const QString &)), SLOT(sl_updateHint(const QString &)));
}

void MSAHighlightingTab::initColorCB(){
    colorScheme->clear();
    colorScheme->addItems(seqArea->getAvailableColorSchemes());

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
    colorScheme->setCurrentIndex(colorScheme->findText(s->getFactory()->getName()));

    MSAHighlightingScheme *sh = seqArea->getCurrentHighlightingScheme();
    highlightingScheme->setCurrentIndex(highlightingScheme->findText(sh->getFactory()->getName()));


    disconnect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_useDots(int))); //disconnect-connect to prevent infinite loop
    useDots->setChecked(seqArea->getUseDotsCheckedState());
    connect(useDots, SIGNAL(stateChanged(int)), seqArea, SLOT(sl_useDots(int)));
    sl_updateHint(QString(""));
}

void MSAHighlightingTab::sl_updateHint(const QString &str){
    Q_UNUSED(str);

    if(msa->getRefSeqName().isEmpty()){
        if(!seqArea->getCurrentHighlightingScheme()->getFactory()->isRefFree()){
            hint->setText(tr("Hint: select a reference above"));
            hint->setStyleSheet(
                "color: green;"
                "font: bold;");
            return;
        }
    }
    hint->setText("");
}

}//ns
