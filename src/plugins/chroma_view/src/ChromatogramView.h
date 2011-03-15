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

#ifndef _U2_CHROMATOGRAM_VIEW_H_
#define _U2_CHROMATOGRAM_VIEW_H_

#include <U2Core/GObject.h>
#include <U2Core/DNAChromatogram.h>
#include <U2View/GSequenceLineView.h>
#include <U2Misc/ScaleBar.h>

#include <QtGui>


namespace U2 {

class ChromatogramViewRenderArea;
class GObjectView;
class Task;

class ChromatogramView : public GSequenceLineView {
    Q_OBJECT
    friend class ChromatogramViewRenderArea;
public:
    ChromatogramView(QWidget* p, ADVSequenceObjectContext* v, GSequenceLineView* cv, const DNAChromatogram& chroma);

    virtual void buildPopupMenu(QMenu& m);

    virtual bool isWidgetOnlyObject(GObject* o) const;

    DNASequenceObject* getEditedSequence() const {return editDNASeq;}
    bool showQV() const {return showQVAction->isChecked();}

protected:
    virtual void pack();
    void mousePressEvent(QMouseEvent* me);

private slots:
    void setRenderAreaHeight(int k);
    void sl_onPopupMenuCkicked(QAction* a);
    void sl_addNewSequenceObject();
    void sl_onAddExistingSequenceObject();
    void sl_onSequenceObjectLoaded(Task*);
    void sl_clearEditableSequence();
    void sl_removeChanges();


    void sl_onObjectRemoved(GObjectView*, GObject*);


private:
    bool checkObject(GObject* obj);
    int getEditSeqIndex(int bcIndex);

    DNASequenceObject*          editDNASeq;
    QByteArray                  currentBaseCalls;
    QSet<int>                   indexOfChangedChars;
    QList<int>                  gapIndexes;

    ScaleBar*                   scaleBar;
    ChromatogramViewRenderArea* ra;
    QMenu*                      mP;
    int                         selIndex;
    QAction*                    addNewSeqAction;
    QAction*                    addExistSeqAction;
    QAction*                    clearEditableSequence;
    QAction*                    removeChanges;
    QAction*                    showQVAction;
};


class ChromatogramViewRenderArea : public GSequenceLineViewRenderArea {
    Q_OBJECT
public:
    ChromatogramViewRenderArea(ChromatogramView* p, const DNAChromatogram& chroma);
    ~ChromatogramViewRenderArea();

    int getHeightAreaBC() const {return heightAreaBC;}
    void setAreaHeight(int newH);
    virtual int coordToPos(int x) const;
    virtual int posToCoord(int p, bool useVirtualSpace = false) const;
    QRectF posToRect(int p) const;
    QRectF  selRect;
    bool    hasSel;
    qreal   charWidth;
    qreal   charHeight;
    qreal   addUpIfQVL;


protected:
    virtual void drawAll(QPaintDevice* pd);

private:
    QColor getBaseColor(char base);
    void drawChromatogramTrace(qreal x, qreal y, qreal w, qreal h, QPainter& p, const U2Region& visible);
    void drawOriginalBaseCalls(qreal x, qreal y, qreal w, qreal h, QPainter& p, const U2Region& visible, const QByteArray& ba, bool is = true);
    void drawQualityValues(qreal x, qreal y, qreal w, qreal h, QPainter& p, const U2Region& visible, const QByteArray& ba);
    void drawChromatogramBaseCallsLines(qreal x, qreal y, qreal w, qreal h, QPainter& p, const U2Region& visible, const QByteArray& ba);

    DNAChromatogram chroma;
    int             chromaMax;
    QPen            linePen;
    QFont           font;
    QFont           fontBold;
    int             heightPD;
    int             heightAreaBC;
    int             areaHeight;
    qreal           kLinearTransformTrace;
    qreal           bLinearTransformTrace;
    qreal           kLinearTransformBaseCallsOfEdited;
    qreal           bLinearTransformBaseCallsOfEdited;
    float           k;
    float           b;
    qreal           xBaseCallsOfEdited;
    qreal           yBaseCallsOfEdited;
    qreal           wBaseCallsOfEdited;
    qreal           hBaseCallsOfEdited;
    bool            heightChanged;

private:
    int             lineHeight;
    int             yCharOffset;
    int             xCharOffset;


    int             lm;
    int             rm;
};


} // namespace


#endif
