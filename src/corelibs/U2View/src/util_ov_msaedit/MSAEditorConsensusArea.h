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
    bool                scribbling;

    MSAEditorConsensusCache*    consensusCache;
    
    bool                completeRedraw;
    QPixmap*            cachedView;
    
    //works in interactive mode with the view -> so we need to cache it
    ConsensusSelectorDialogController* consensusDialog;
};


}//namespace
#endif

