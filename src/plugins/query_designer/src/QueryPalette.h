#ifndef _U2_QUERY_PALETTE_H_
#define _U2_QUERY_PALETTE_H_

#include <QtGui/QAction>
#include <QtGui/QTreeWidget>

namespace U2 {

class QDDistanceIds {
public:
    static const QString E2S;
    static const QString S2E;
    static const QString E2E;
    static const QString S2S;
};

class QDActorPrototype;
class QueryPalette : public QTreeWidget {
    Q_OBJECT
public:
    static const QString MIME_TYPE;
    QueryPalette(QWidget* parent=NULL);

    QVariant saveState() const;
    void restoreState(const QVariant&);
signals:
    void processSelected(QDActorPrototype*);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);
private slots:
    void sl_selectProcess(bool checked=false);
private:
    void setContent();
    QAction* createItemAction(QDActorPrototype* item);
    QAction* createItemAction(const QString& constraintId);
private:
    QTreeWidgetItem* overItem;
    QAction* currentAction;
    QPoint  dragStartPosition;
    //  < category id, item ids > ids r used as display names
    QMap< QString, QList<QString> > categoryMap;
    QMap<QAction*, QTreeWidgetItem*> actionMap;
    friend class PaletteDelegate;
};

}//namespace

Q_DECLARE_METATYPE(QAction *)
Q_DECLARE_METATYPE(U2::QDActorPrototype *)

#endif
