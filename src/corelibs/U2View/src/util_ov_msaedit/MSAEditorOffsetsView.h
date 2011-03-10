#ifndef _U2_MSA_EDITOR_OFFSETS_VIEW_H_
#define _U2_MSA_EDITOR_OFFSETS_VIEW_H_

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QEvent>

#include <QtGui/QWidget>

namespace U2 {

class MSAEditor;
class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;
class MSAEditorSequenceArea;
class MSAEditorBaseOffsetCache;
class MSAEditorOffsetsViewWidget;

class MSAEditorOffsetsViewController : public QObject {
    Q_OBJECT
public:
    MSAEditorOffsetsViewController(QObject* p, MSAEditor* editor, MSAEditorSequenceArea* seqArea);

    MSAEditorOffsetsViewWidget* getLeftWidget() const {return lw;}
    MSAEditorOffsetsViewWidget* getRightWidget() const {return rw;}

    QAction* getToggleColumnsViewAction() const {return viewAction;}
    bool eventFilter(QObject* o, QEvent* e);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&){updateOffsets();}
    void sl_startChanged(const QPoint& , const QPoint& ) {updateOffsets();}
    void sl_fontChanged() {updateOffsets();}
    void sl_showOffsets(bool);
private:
    void updateOffsets();
    
    MSAEditorSequenceArea*      seqArea;
    MSAEditor*                  editor;
    MSAEditorOffsetsViewWidget* lw;
    MSAEditorOffsetsViewWidget* rw;
    QAction*                    viewAction;
};

class MSAEditorOffsetsViewWidget : public QWidget {
    friend class MSAEditorOffsetsViewController;
public:
    MSAEditorOffsetsViewWidget(MSAEditor* editor, MSAEditorSequenceArea* seqArea, MSAEditorBaseOffsetCache* cache, bool showStartPos);
    ~MSAEditorOffsetsViewWidget();
    
protected:
    void paintEvent(QPaintEvent* e);
    void updateView();
    void drawAll(QPainter& p);
    QFont getOffsetsFont();

private:
    MSAEditorSequenceArea*      seqArea;
    MSAEditor*                  editor;
    MSAEditorBaseOffsetCache*   cache;
    bool                        showStartPos;
    bool                        completeRedraw;
    QPixmap*                    cachedView;
};

}//namespace;

#endif
