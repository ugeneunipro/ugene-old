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

#ifndef _U2_MSA_HIGHLIGHTING_TAB_H_
#define _U2_MSA_HIGHLIGHTING_TAB_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QtGui>
#else
#include <QtWidgets/QtWidgets>
#endif

#include <U2View/MSAEditor.h>

namespace U2 {

class U2VIEW_EXPORT MSAHighlightingTab : public QWidget
{
    Q_OBJECT
public:
    MSAHighlightingTab(MSAEditor* msa);

private slots:
    void sl_sync();
    void sl_updateHint();
    void sl_exportHighlightningClicked();

private:
    QWidget* createColorGroup();
    QWidget* createHighlightingGroup();
    void initColorCB();

    MSAEditor *msa;
    MSAEditorSequenceArea *seqArea;
    QComboBox *colorScheme;
    QComboBox *highlightingScheme;
    QLabel *hint;
    QCheckBox *useDots;
    QPushButton *exportHighlightning;
};

} // namespace

#endif
