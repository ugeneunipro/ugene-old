/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PAIRWISE_ALIGNMENT_HIRSCHBERG_GUI_EXTENSION_FACTORY_H_
#define _U2_PAIRWISE_ALIGNMENT_HIRSCHBERG_GUI_EXTENSION_FACTORY_H_

#include <U2View/PairwiseAlignmentGUIExtension.h>

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

namespace U2 {

//default values were taken from kalign files
#define H_MIN_GAP_OPEN 0
#define H_MAX_GAP_OPEN 65535        //it isn`t the maximum, it may be less
#define H_DEFAULT_GAP_OPEN_DNA 217  //taken from kalign2_misc.c
#define H_DEFAULT_GAP_OPEN 54.94941
#define H_MIN_GAP_EXTD 0
#define H_MAX_GAP_EXTD 65535        //it isn`t the maximum, it may be less
#define H_DEFAULT_GAP_EXTD_DNA 39.4 //taken from kalign2_misc.c
#define H_DEFAULT_GAP_EXTD 8.52492  //taken from kalign2_misc.c
#define H_MIN_GAP_TERM 0
#define H_MAX_GAP_TERM 65535        //it isn`t the maximum, it may be less
#define H_DEFAULT_GAP_TERM_DNA 292.6//taken from kalign2_misc.c
#define H_DEFAULT_GAP_TERM 4.42410  //taken from kalign2_misc.c
#define H_MIN_BONUS_SCORE 0
#define H_MAX_BONUS_SCORE 65535     //it isn`t the maximum, it may be less
#define H_DEFAULT_BONUS_SCORE_DNA 283//taken from kalign2_misc.c
#define H_DEFAULT_BONUS_SCORE 0.2   //taken from kalign2_misc.c

class PairwiseAlignmentHirschbergMainWidget : public PairwiseAlignmentMainWidget {
    Q_OBJECT

public:
    PairwiseAlignmentHirschbergMainWidget(QWidget* parent, QVariantMap* s, WidgetType _widgetType);
    virtual ~PairwiseAlignmentHirschbergMainWidget();

    virtual QVariantMap getPairwiseAlignmentCustomSettings(bool append);

protected slots:
    void sl_translateToAminoChecked(int newState);

protected:
    virtual void fillInnerSettings();

protected:
    QDoubleSpinBox* gapOpen;
    QDoubleSpinBox* gapExtd;
    QDoubleSpinBox* gapTerm;
    QDoubleSpinBox* bonusScore;
    QCheckBox* translateToAminoCheckBox;
    QComboBox* translationTableComboBox;
};


class PairwiseAlignmentHirschbergGUIExtensionFactory : public PairwiseAlignmentGUIExtensionFactory {
    Q_OBJECT

public:
    PairwiseAlignmentHirschbergGUIExtensionFactory();
    virtual ~PairwiseAlignmentHirschbergGUIExtensionFactory();

    virtual PairwiseAlignmentMainWidget* createMainWidget(QWidget *parent, QVariantMap *s, WidgetType widgetType);
};

}   //namespace

#endif // _U2_PAIRWISE_ALIGNMENT_HIRSCHBERG_GUI_EXTENSION_FACTORY_H_
