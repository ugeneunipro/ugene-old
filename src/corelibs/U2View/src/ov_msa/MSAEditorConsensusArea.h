/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QHelpEvent>

namespace U2 {

class MSAEditor;
class MSAEditorUI;
class GObjectView;
class MSAEditorConsensusCache;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSelection;
class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;
class ConsensusSelectorDialogController;

enum MSAEditorConsElement {
    MSAEditorConsElement_HISTOGRAM,
    MSAEditorConsElement_CONSENSUS_TEXT,
    MSAEditorConsElement_RULER
};

class U2VIEW_EXPORT MSAEditorConsensusArea : public QWidget {
    Q_OBJECT
public:
    MSAEditorConsensusArea(MSAEditorUI* ui);
    ~MSAEditorConsensusArea();

    U2Region getRullerLineYRange() const;

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);
    void setConsensusAlgorithmConsensusThreshold(int val);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const;

protected:
    virtual bool event(QEvent* e);
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

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
    void updateThresholdInfoInConsensusDialog();
    void restoreLastUsedConsensusThreshold();
    QString getLastUsedAlgoSettingsKey() const;
    QString getThresholdSettingsKey(const QString& factoryId) const;

    void buildMenu(QMenu* m);
    void setupFontAndHeight();
    void updateSelection(int newPos);

    void drawConsensus(QPainter& p);
    void drawConsensusChar(QPainter& p, int pos, bool selected);
    void drawRuler(QPainter& p);
    void drawHistogram(QPainter& p);
    void drawSelection(QPainter& p);


    U2Region getYRange(MSAEditorConsElement e) const;

    MSAEditor*          editor;
    const MSAEditorUI*  ui;
    QFont               rulerFont;
    int                 rulerFontHeight;
    QAction*            copyConsensusAction;
    QAction*            copyConsensusWithGapsAction;
    QAction*            configureConsensusAction;
    int                 curPos;
    bool                scribbling, selecting;

    MSAEditorConsensusCache*    consensusCache;
    
    bool                completeRedraw;
    QPixmap*            cachedView;

    QObject *childObject;
    
    //works in interactive mode with the view -> so we need to cache it
    ConsensusSelectorDialogController* consensusDialog;
};


}//namespace
#endif

