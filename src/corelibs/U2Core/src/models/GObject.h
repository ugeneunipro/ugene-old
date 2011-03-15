/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
