#ifndef _U2_FIND_QUERY_H_
#define _U2_FIND_QUERY_H_

#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Algorithm/FindAlgorithmTask.h>


namespace U2 {

class QDFindActor : public QDActor {
    Q_OBJECT
public:
    QDFindActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff,0xf8,0); }
    bool hasStrand() const { return false; }
private slots:
    void sl_onFindTaskFinished(Task* t);
private:
    FindAlgorithmTaskSettings settings;
};

class QDFindActorPrototype : public QDActorPrototype {
public:
    QDFindActorPrototype();
    virtual QDActor* createInstance() const { return new QDFindActor(this); }
    virtual QIcon getIcon() const { return QIcon(":core/images/find_dialog.png"); }
};

}//namespace

#endif
