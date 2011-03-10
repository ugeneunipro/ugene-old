#ifndef _U2_PRIMER_QUERY_H_
#define _U2_PRIMER_QUERY_H_

#include "Primer3TaskSettings.h"

#include "U2Lang/QDScheme.h"
#include "U2Lang/QueryDesignerRegistry.h"


namespace U2 {

class QDPrimerActor : public QDActor {
    Q_OBJECT
public:
    QDPrimerActor(QDActorPrototype const* proto);
    int getMinResultLen() const { return 1; }
    int getMaxResultLen() const { return 1000; }
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0x88, 0x78, 0xEC); }
private:
    void setDefaultSettings();
private slots:
    void sl_onAlgorithmTaskFinished(Task* t);
private:
    Primer3TaskSettings settings;
};

class QDPrimerActorPrototype : public QDActorPrototype {
public:
    QDPrimerActorPrototype();
    QIcon getIcon() const { return QIcon(":primer3/images/primer3.png"); }
    QDActor* createInstance() const { return new QDPrimerActor(this); }
};

}//namespace

#endif
