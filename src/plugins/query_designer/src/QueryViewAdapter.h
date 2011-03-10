#ifndef _U2_QUERY_VIEW_ADAPTER_H_
#define _U2_QUERY_VIEW_ADAPTER_H_

#include <QtCore/QList>
#include <QtCore/QPointF>


namespace U2 {

class QDScheme;
class QDSchemeUnit;
class QDElement;
class Footnote;
class QueryViewAdapter {
public:
    QueryViewAdapter(QDScheme* scheme, const QPointF& topLeftCorner = QPointF(0,0));
    const QList<QDElement*>& getElements() const { return createdElements; }
    const QList<Footnote*>& getFootnotes() const { return createdFootnotes; }
    void placeElement(QDElement* uv);
    void sortVertically();

    //void scheme2graph();

private:
    void moveElement(QDElement* uv, int dx);
    //returns false if recursive
    bool needToMove(QDElement* current, int dx, QList<QDElement*>& res);

    QList<Footnote*> getSharedFootnotes(QDElement* uv1, QDElement* uv2) const;
private:
    QDScheme* scheme;
    QList<QDElement*> createdElements;
    QList<Footnote*> createdFootnotes;
    QList<QDElement*> currentPath;
};

}//namespace

#endif
