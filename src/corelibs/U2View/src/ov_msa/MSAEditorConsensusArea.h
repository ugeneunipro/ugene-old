/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_EDITOR_CONSENSUS_AREA_H_
#define _U2_MSA_EDITOR_CONSENSUS_AREA_H_

#include <QWidget>

#include <U2Core/U2Region.h>

#include "MSAEditorConsensusCache.h"

class QHelpEvent;
class QMenu;
class QPainter;

namespace U2 {

class MSAEditor;
class MSAEditorUI;
class GObjectView;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSelection;
class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;

enum MSAEditorConsElement {
    MSAEditorConsElement_HISTOGRAM,
    MSAEditorConsElement_CONSENSUS_TEXT,
    MSAEditorConsElement_RULER
};

class U2VIEW_EXPORT MSAEditorConsensusArea : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MSAEditorConsensusArea)
public:
    MSAEditorConsensusArea(MSAEditorUI* ui);
    ~MSAEditorConsensusArea();

    U2Region getRullerLineYRange() const;

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);
    void setConsensusAlgorithmConsensusThreshold(int val);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const;

    QSharedPointer<MSAEditorConsensusCache> getConsensusCache();

    void paintFullConsensus(QPixmap &pixmap);
    void paintFullConsensus(QPainter& p);

    void paintConsenusPart(QPixmap & pixmap, const U2Region &region, const QList<qint64> &seqIdx);
    void paintConsenusPart(QPainter& p, const U2Region &region, const QList<qint64> &seqIdx);

    void paintRulerPart(QPixmap &pixmap, const U2Region &region);
    void paintRulerPart(QPainter &p, const U2Region &region);

protected:
    bool event(QEvent* e);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

signals:
    void si_consensusAlgorithmChanged(const QString& algoId);
    void si_consensusThresholdChanged(int value);

private slots:
    void sl_startChanged(const QPoint&, const QPoint&);
    void sl_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev);
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);
    void sl_changeConsensusAlgorithm(const QString& algoId);
    void sl_changeConsensusThreshold(int val);
    void sl_onScrollBarActionTriggered( int scrollAction );
    void sl_onConsensusThresholdChanged(int newValue);

    void sl_buildStaticMenu(GObjectView* v, QMenu* m);
    void sl_buildContextMenu(GObjectView* v, QMenu* m);
    void sl_copyConsensusSequence();
    void sl_copyConsensusSequenceWithGaps();
    void sl_configureConsensusAction();
    void sl_zoomOperationPerformed(bool resizeModeChanged);

public:
    void drawContent(QPainter& painter);

private:
    QString createToolTip(QHelpEvent* he) const;
    void restoreLastUsedConsensusThreshold();
    QString getLastUsedAlgoSettingsKey() const;
    QString getThresholdSettingsKey(const QString& factoryId) const;

    void buildMenu(QMenu* m);
    void setupFontAndHeight();
    void updateSelection(int newPos);

    void drawConsensus(QPainter& p);
    void drawConsensus(QPainter& p, int startPos, int lastPos, bool useVirtualCoords = false);

    void drawConsensusChar(QPainter& p, int pos, int firstVisiblePos, bool selected, bool useVirtualCoords = false);
    void drawConsensusChar(QPainter& p, int pos, int firstVisiblePos, char consChar,
                           bool selected, bool useVirtualCoords = false);

    void drawRuler(QPainter& p, int start = -1, int end = -1, bool drawFull = false);

    void drawHistogram(QPainter& p);
    void drawHistogram(QPainter& p, int firstBase, int lastBase);

    void drawSelection(QPainter& p);


    U2Region getYRange(MSAEditorConsElement e) const;

    MSAConsensusAlgorithmFactory* getConsensusAlgorithmFactory();
    void updateConsensusAlgorithm();

    MSAEditor*          editor;
    const MSAEditorUI*  ui;
    QFont               rulerFont;
    int                 rulerFontHeight;
    QAction*            copyConsensusAction;
    QAction*            copyConsensusWithGapsAction;
    QAction*            configureConsensusAction;
    int                 curPos;
    bool                scribbling, selecting;

    QSharedPointer<MSAEditorConsensusCache>    consensusCache;

    bool                completeRedraw;
    QPixmap*            cachedView;

    QObject *childObject;
};

}//namespace
#endif

