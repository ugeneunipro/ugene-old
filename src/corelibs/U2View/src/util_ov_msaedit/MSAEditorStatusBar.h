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

class MSAEditorStatusWidget : public QWidget {
    Q_OBJECT
public:
    MSAEditorStatusWidget(MAlignmentObject* mobj, MSAEditorSequenceArea* seqArea);

    bool eventFilter(QObject* obj, QEvent* ev);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {updateCoords();}
    void sl_lockStateChanged() {updateLock();}
    void sl_cursorMoved(const QPoint& , const QPoint& ) {updateCoords();}
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
    QPoint                      lastSearchPos;
    QAction*                    findAction;

};


}//namespace;

#endif
