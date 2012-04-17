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

#ifndef _U2_MSA_EDITOR_STATUS_BAR_H_
#define _U2_MSA_EDITOR_STATUS_BAR_H_

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QEvent>

#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

namespace U2 {

class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSequenceArea;
class MSAEditorSelection;

class MSAEditorStatusWidget : public QWidget {
    Q_OBJECT
public:
    MSAEditorStatusWidget(MAlignmentObject* mobj, MSAEditorSequenceArea* seqArea);

    bool eventFilter(QObject* obj, QEvent* ev);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {updateCoords();}
    void sl_lockStateChanged() {updateLock();}
    void sl_selectionChanged(const MSAEditorSelection& , const MSAEditorSelection& ){updateCoords();}
    void sl_findNext();
    void sl_findPrev();
    void sl_findFocus();
    
private:
    void updateCoords();
    void updateLock();
    MAlignmentObject*           aliObj;
    MSAEditorSequenceArea*      seqArea;
    QPixmap                     lockedIcon;
    QPixmap                     unlockedIcon;

    QPushButton*                prevButton;
    QPushButton*                nextButton;
    QLineEdit*                  searchEdit;
    QLabel*                     linesLabel;
    QLabel*                     colsLabel;
    QLabel*                     lockLabel;
    QLabel*                     posLabel;
    QPoint                      lastSearchPos;
    QAction*                    findAction;

};


}//namespace;

#endif
