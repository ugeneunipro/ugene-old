#ifndef _U2_SELECTION_MODEL_H_
#define _U2_SELECTION_MODEL_H_

#include <U2Core/global.h>

namespace U2 {

typedef QString GSelectionType;


class U2CORE_EXPORT GSelection : public QObject {
    Q_OBJECT
public:
    GSelection(GSelectionType _type, QObject* p = NULL) : QObject(p), type(_type) {}

    GSelectionType getSelectionType() const {return type;}

    virtual bool isEmpty() const = 0;

    virtual void clear() = 0;

signals:
    void si_onSelectionChanged(GSelection*);

private:
    GSelectionType type;
};

typedef QList<GSelection*> GSelections;

class U2CORE_EXPORT MultiGSelection {
public:
    MultiGSelection(){}
    //TODO: deallocation! -> use shared data

    void addSelection(const GSelection* s);

    void removeSelection(const GSelection* s);

    bool contains(GSelectionType t) const {return findSelectionByType(t) !=NULL;}

    const GSelection* findSelectionByType(GSelectionType t) const;

    const QList<const GSelection*>& getSelections() const {return selections;}

private:
    QList<const GSelection*> selections;
};

}//namespace
#endif
