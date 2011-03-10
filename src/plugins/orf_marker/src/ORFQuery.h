#ifndef _U2_ORF_QUERY_H_
#define _U2_ORF_QUERY_H_

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include "U2Algorithm/ORFFinder.h"


namespace U2 {

class ORFFindTask;
class QDORFActor : public QDActor {
    Q_OBJECT
public:
    QDORFActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff, 0xc6, 0); }
private slots:
    void sl_onAlgorithmTaskFinished(Task*);
private:
    ORFAlgorithmSettings settings;
    QList<ORFFindTask*> orfTasks;
};

class QDORFActorPrototype : public QDActorPrototype {
public:
    QDORFActorPrototype();
    QIcon getIcon() const { return QIcon(":orf_marker/images/orf_marker.png"); }
    virtual QDActor* createInstance() const { return new QDORFActor(this); }
};

}//namespace

#endif
