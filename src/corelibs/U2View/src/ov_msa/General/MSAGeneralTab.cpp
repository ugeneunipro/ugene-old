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

#include "MSAGeneralTab.h"

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Core/MAlignmentObject.h>


namespace U2{

static const int ITEMS_SPACING = 10;
static const int TITLE_SPACING = 5;

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
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);

    w->setLayout(layout);
    return layout;
}

MSAGeneralTab::MSAGeneralTab(MSAEditor* m):msa(m){
    connect(msa, SIGNAL(si_refrenceSeqChanged(const QString &)), SLOT(sl_refSeqChanged(const QString &)));

    QVBoxLayout* mainLayout = initVBoxLayout(this);
    mainLayout->setSpacing(0);

    //Refrence sequence
    QWidget * refrenceGroup = new ShowHideSubgroupWidget("REFRENCE", tr("Refrence sequence"), createRefrenceGroup(), true);
    mainLayout->addWidget(refrenceGroup);
}

QWidget* MSAGeneralTab::createRefrenceGroup(){
    QWidget * group = new QWidget(this);
    QVBoxLayout * layout = initVBoxLayout(group);

    reSeqSelector = new SequenceSelectorWidgetController(msa);  

    connect(reSeqSelector, SIGNAL(si_textControllerChanged()), SLOT(si_textControllerChanged()));

    layout->addSpacing(TITLE_SPACING);  
    layout->addWidget(reSeqSelector);
    layout->addSpacing(ITEMS_SPACING);

    return group;
}

void MSAGeneralTab::sl_refSeqChanged( const QString & str){
    reSeqSelector->setText(str);
}

void MSAGeneralTab::si_textControllerChanged(){
        msa->setRefrence(reSeqSelector->text());
}

}