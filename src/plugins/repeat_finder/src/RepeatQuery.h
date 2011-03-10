#ifndef _U2_REPEAT_QUERY_H_
#define _U2_REPEAT_QUERY_H_

#include "FindRepeatsTask.h"

#include "U2Lang/QDScheme.h"
#include <U2Lang/QueryDesignerRegistry.h>


namespace U2 {

class QDRepeatActor : public QDActor {
    Q_OBJECT
public:
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QList< QPair<QString,QString> > saveConfiguration() const;
    void loadConfiguration(const QList< QPair<QString,QString> >& strMap);
    QColor defaultColor() const { return QColor(0x66,0xa3,0xd2); }
    virtual bool hasStrand() const { return false; }
protected:
    QDRepeatActor(QDActorPrototype const* proto);
    friend class QDRepeatActorPrototype;
private slots:
    void sl_onAlgorithmTaskFinished();
private:
    //void addResults( const SharedAnnotationData& ad, bool complement );

    FindRepeatsTaskSettings settings;
    QList<FindRepeatsToAnnotationsTask*> repTasks;
};

class QDRepeatActorPrototype : public QDActorPrototype {
public:
    QDRepeatActorPrototype();
    QIcon getIcon() const { return QIcon(":repeat_finder/images/repeats.png"); }
    QDActor* createInstance() const { return new QDRepeatActor(this); }
};

}//namespace

#endif
