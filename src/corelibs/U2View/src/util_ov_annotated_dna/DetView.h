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

#ifndef _U2_DET_VIEW_H_
#define _U2_DET_VIEW_H_

#include "GSequenceLineViewAnnotated.h"

#include <U2Core/U2Annotation.h>

#include <QtGui/QFont>
#include <QtGui/QAction>

class QActionGroup;

namespace U2 {

class DNATranslation;

class U2VIEW_EXPORT DetView : public GSequenceLineViewAnnotated {
    Q_OBJECT

public:
    DetView(QWidget* p, ADVSequenceObjectContext* ctx);

    bool isOneLineMode()  {return getComplementTT() == NULL && getAminoTT() == NULL;}
    bool hasTranslations()  {return getAminoTT() != NULL;}
    bool hasComplementaryStrand()  {return getComplementTT() != NULL;}
    
    DNATranslation* getComplementTT() const;
    DNATranslation* getAminoTT() const;

    void setShowComplement(bool t);
    void setShowTranslation(bool t);

    QAction* getShowComplementAction() const {return showComplementAction;}
    QAction* getShowTranslationAction() const {return showTranslationAction;}

protected slots:
    virtual void sl_sequenceChanged();
    void sl_onAminoTTChanged();
    void sl_showComplementToggle(bool v) {setShowComplement(v);}
    void sl_showTranslationToggle(bool v) {setShowTranslation(v);}

protected:
    void showEvent(QShowEvent * e);
    void hideEvent(QHideEvent * e);
    
    void mouseReleaseEvent(QMouseEvent* me);
    
    void updateActions();

    void resizeEvent(QResizeEvent *e);
    void updateSize();

    QAction*        showComplementAction;
    QAction*        showTranslationAction;
};


class DetViewRenderArea : public GSequenceLineViewAnnotatedRenderArea {
public:
    DetViewRenderArea(DetView* d);

    virtual int coordToPos(int x) const;
    virtual float posToCoordF(int x, bool useVirtualSpace = false) const;
    virtual float getCurrentScale() const;

    DetView* getDetView() const {return static_cast<DetView*>(view);}

    virtual U2Region getAnnotationYRange(Annotation* a, int region, const AnnotationSettings* as) const;
    
    void updateSize();

    bool isOnTranslationsLine(int y) const;

protected:
    virtual void drawAll(QPaintDevice* pd);


private:
    int getLineY(int line) const {return 2 + line * lineHeight;}
    int getTextY(int line) const {return getLineY(line) + lineHeight - yCharOffset;}

    void updateLines();
    void drawDirect(QPainter& p);
    void drawComplement(QPainter& p);
    void drawTranslations(QPainter& p);
    void drawSequenceSelection(QPainter& p);
    void drawRuler(QPainter& p);
    void highlight(QPainter& p, const U2Region& r, int line);

    int posToComplTransLine(int p) const;
    int posToDirectTransLine(int p) const;
    bool deriveTranslationCharColor(int pos, U2Strand strand, QList<Annotation*> annotationsInRange, QColor& result);

    int numLines;
    int rulerLine;
    int baseLine;
    int complementLine;
    int firstDirectTransLine;
    int firstComplTransLine;
};


}//namespace;

#endif
