#ifndef _U2_GOBJECT_H_
#define _U2_GOBJECT_H_

#include <U2Core/global.h>
#include "StateLockableDataModel.h"
#include "GObjectReference.h"

#include <QtCore/QMimeData>
#include <QtCore/QPointer>

#define RELATED_OBJECTS_KEY "related_objs"

namespace U2 {

class Document;
class GObjectConstraints;
class GHints;

class U2CORE_EXPORT GObject : public StateLockableTreeItem {
    friend class DocumentFormat;
    friend class Document;
    
    Q_OBJECT
public:
    GObject(QString _type, const QString& _name, const QVariantMap& hints = QVariantMap());
    virtual ~GObject();

    virtual Document* getDocument() const;

    const QString& getGObjectName() const {return name;}

    virtual void setGObjectName(const QString& newName);

    GObjectType getGObjectType() const {return type;}

    virtual GObject* clone() const =  0;

    virtual bool checkConstraints(const GObjectConstraints* c) const {Q_UNUSED(c); return true;}

    GHints* getGHints() const {return hints;}
    
    QVariantMap getGHintsMap() const;
    
    void setGHints(GHints*);

    QList<GObjectRelation> getObjectRelations() const;

    void setObjectRelations(const QList<GObjectRelation>& obj);

    QList<GObjectRelation> findRelatedObjectsByRole(const QString& role);
    
    QList<GObjectRelation> findRelatedObjectsByType(const GObjectType& objType);
    
    void addObjectRelation(const GObjectRelation& ref);
    
    void addObjectRelation(const GObject* obj, const QString& role);
    
    void removeObjectRelation(const GObjectRelation& ref);

    bool hasObjectRelation(const GObject* obj, const QString& role) const;
    
    bool hasObjectRelation(const GObjectRelation& r) const;

    QHash< QString, QString > getIndexInfo() const {return indexInfo;}
    
    void setIndexInfo( const QHash<QString, QString>& ii) {indexInfo = ii;}

    bool isUnloaded() const;
    
signals:
    void si_nameChanged(const QString& oldName);

private:
    GObjectType                 type;
    QString                     name;
    GHints*                     hints;
    QHash<QString, QString>     indexInfo;
};

class GObjectConstraints : public QObject {
    Q_OBJECT
public:
    GObjectConstraints(const GObjectType& t, QObject* parent = NULL) : QObject(parent), objectType(t){}
    const GObjectType objectType;

};

class U2CORE_EXPORT GObjectMimeData : public QMimeData {
    Q_OBJECT
public:
    static const QString MIME_TYPE;
    GObjectMimeData(GObject* obj) : objPtr(obj){};
    ~GObjectMimeData();
    QPointer<GObject> objPtr;
};

}//namespace

#endif
