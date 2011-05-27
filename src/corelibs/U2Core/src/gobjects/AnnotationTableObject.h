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

#ifndef _U2_ANNOTATION_TABLE_OBJECT_H_
#define _U2_ANNOTATION_TABLE_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationData.h>

#include <QtCore/QSet>
#include <QtCore/QTimer>

namespace U2 {

class Annotation;
class U2Qualifier;
class AnnotationGroup;
class AnnotationTableObject;
class DNASequenceObject;
class DNATranslation;

enum AnnotationModificationType {
    AnnotationModification_NameChanged, 
    AnnotationModification_QualifierAdded,
    AnnotationModification_QualifierRemoved,
    AnnotationModification_LocationChanged,
    AnnotationModification_AddedToGroup, // Emitted when annotation is added to a group. 
                                         // Note that first time annotation added to the object it automatically 
                                         // has a group assigned and this signal is not emitted
    AnnotationModification_RemovedFromGroup
};


class U2CORE_EXPORT AnnotationModification {
public:
    AnnotationModification(AnnotationModificationType _type, Annotation* a) : type(_type), annotation(a) {}
    AnnotationModificationType type;
    Annotation* annotation;
};

class  U2CORE_EXPORT QualifierModification : public AnnotationModification {
public:
    QualifierModification(AnnotationModificationType t, Annotation* a, const U2Qualifier& q) 
        : AnnotationModification(t, a), qualifier(q) {}

    U2Qualifier qualifier;
};

class  U2CORE_EXPORT AnnotationGroupModification : public AnnotationModification {
public:
    AnnotationGroupModification(AnnotationModificationType t, Annotation* a, AnnotationGroup* g) 
        : AnnotationModification(t, a), group(g) {}

    AnnotationGroup* group;
};


class  U2CORE_EXPORT Annotation {
    friend class AnnotationGroup;
    friend class AnnotationTableObject;

public:
    Annotation(SharedAnnotationData data); 

    virtual ~Annotation();

    static bool isValidAnnotationName(const QString& n);
    static bool isValidQualifierName(const QString& s);
    static bool isValidQualifierValue(const QString& s);


    AnnotationTableObject* getGObject() const { return obj;}

    const QString& getAnnotationName() const  {return d->name;}
    
    void setAnnotationName(const QString& name);

    bool isOrder() const {return d->isOrder();}

    bool isJoin() const {return d->isJoin();}
    
    U2Strand getStrand() const {return d->getStrand();}

    void setStrand(U2Strand strand);

    U2LocationOperator getLocationOperator() {return d->getLocationOperator();}

    void setLocationOperator(U2LocationOperator op);

    const U2Location& getLocation() const {return d->location;}
    
    void setLocation(const U2Location& location );
    
    const QVector<U2Region>& getRegions() const {return d->getRegions();}
    
    void replaceRegions(const QVector<U2Region>& regions);

    void addLocationRegion(const U2Region& reg);

    const QVector<U2Qualifier>& getQualifiers() const;

    void addQualifier(const U2Qualifier& q);
    
    void addQualifier(const QString& name, const QString& val) {addQualifier(U2Qualifier(name, val));}

    void removeQualifier(const U2Qualifier& q);
    
    void removeQualifier(const QString& name, const QString& val) {removeQualifier(U2Qualifier(name, val));}

    const QList<AnnotationGroup*>& getGroups() const {return groups;}

    void findQualifiers(const QString& name, QVector<U2Qualifier>& res) const {return d->findQualifiers(name, res);}
    
    QString findFirstQualifierValue(const QString& name) const {return d->findFirstQualifierValue(name);}

    SharedAnnotationData data() const {return d;}
    
    QString getQualifiersTip(int maxRows, DNASequenceObject* = NULL, DNATranslation* complTT = NULL, DNATranslation* aminoTT = NULL) const;
    
    static bool annotationLessThan(Annotation *first, Annotation *second);
    
private:
    AnnotationTableObject*                  obj;
    SharedAnnotationData                    d;
    QList<AnnotationGroup*>                 groups;
};


class U2CORE_EXPORT AnnotationGroup {
    friend class AnnotationTableObject;
public:
    AnnotationGroup(AnnotationTableObject* p, const QString& _name, AnnotationGroup* parentGrp);
    ~AnnotationGroup();
    
    static const QString ROOT_GROUP_NAME;

    static bool isValidGroupName(const QString& n, bool pathMode);

    void findAllAnnotationsInGroupSubTree(QSet<Annotation*>& set) const;

    const QList<Annotation*>& getAnnotations() const {return annotations;}

    void addAnnotation(Annotation* a);
    
    void removeAnnotation(Annotation* a);

    void removeAnnotations(const QList<Annotation*>& annotations);

    const QList<AnnotationGroup*>& getSubgroups() const {return subgroups;}

    void removeSubgroup(AnnotationGroup* g);

    const QString& getGroupName() const {return name;}
    
    QString getGroupPath() const; 

    void setGroupName(const QString& newName);

    AnnotationTableObject* getGObject() const {return obj;}

    AnnotationGroup* getParentGroup() const {return parentGroup;}
    
    AnnotationGroup* getSubgroup(const QString& path, bool create);

    void getSubgroupPaths(QStringList& res) const;

    /// remove all subgroups and annotation refs
    void clear();

    int getGroupDepth() const {return 1 + (parentGroup == NULL ? 0 : parentGroup->getGroupDepth());}

    bool isParentOf(AnnotationGroup* g) const;

    bool isRootGroup() const {return parentGroup == NULL;}
    
    //true if the parent of the group is root group
    bool isTopLevelGroup() const {return parentGroup!=NULL && parentGroup->isRootGroup();}

private:
    friend U2CORE_EXPORT QDataStream& operator>>(QDataStream& dataStream, AnnotationGroup* parentGroup);
    friend U2CORE_EXPORT QDataStream& operator<<(QDataStream& dataStream, const AnnotationGroup& group);

    QString                 name;
    AnnotationTableObject*  obj;
    AnnotationGroup*        parentGroup;
    QList<Annotation*>      annotations;
    QList<AnnotationGroup*> subgroups;
};

U2CORE_EXPORT QDataStream& operator>>(QDataStream& dataStream, AnnotationGroup* parentGroup);
U2CORE_EXPORT QDataStream& operator<<(QDataStream& dataStream, const AnnotationGroup& group);

class AnnotationsLocker: public QObject {
    Q_OBJECT
public:
    void setToDelete(const QList<Annotation*>& _anns, AnnotationGroup *_parentGroup, int counter);
    void releaseLocker();
    bool isLocked() const;
    void sl_Clean();
    
private:
    QList<Annotation*> anns;
    AnnotationGroup *parentGroup;
    int deleteCounter;
};

class DeleteAnnotationsFromObjectTask: public Task {
public:
    DeleteAnnotationsFromObjectTask(const QList<Annotation *> _anns, AnnotationTableObject *_aobj, AnnotationGroup *_gr): 
      Task(tr("Delete annotations from object task"), TaskFlag_None), anns(_anns), aobj(_aobj), group(_gr){}
    void run();
    Task::ReportResult report();

private:
    QList<Annotation *> anns;
    AnnotationTableObject *aobj;
    AnnotationGroup *group;
};

class U2CORE_EXPORT AnnotationTableObject: public GObject {
    Q_OBJECT
    friend class Annotation;
    friend class AnnotationGroup;

public:
    AnnotationTableObject(const QString& objectName, const QVariantMap& hintsMap = QVariantMap());
    ~AnnotationTableObject();
    
    const QList<Annotation*>& getAnnotations() const {return annotations;}
    
    AnnotationGroup* getRootGroup() const {return rootGroup;}

    void addAnnotation(Annotation* a, const QString& groupName = QString());

    void addAnnotation(Annotation* a, QStringList& groupsNames);

    void addAnnotations(const QList<Annotation*>& annotations, const QString& groupName = QString());

    void removeAnnotation(Annotation* a);
    
    void removeAnnotations(const QList<Annotation*>& annotations);

    virtual GObject* clone() const;

    void selectAnnotationsByName(const QString& name, QList<Annotation*>& res);

    bool checkConstraints(const GObjectConstraints* c) const;

    void removeAnnotationsInGroup(const QList<Annotation*>& _annotations, AnnotationGroup *group);
    void releaseLocker();
    bool isLocked() const;
    void cleanAnnotations();

protected:

    void emit_onAnnotationModified(const AnnotationModification& md) {emit si_onAnnotationModified(md);}

    void emit_onGroupCreated(AnnotationGroup* g) {emit si_onGroupCreated(g);}
    void emit_onGroupRemoved(AnnotationGroup* p, AnnotationGroup* g) {emit si_onGroupRemoved(p, g);}
    void emit_onGroupRenamed(AnnotationGroup* g, const QString& oldName) {emit si_onGroupRenamed(g, oldName);}
    void emit_onAnnotationsInGroupRemoved(const QList<Annotation*>& l, AnnotationGroup* gr) {emit si_onAnnotationsInGroupRemoved(l, gr);}

    void _removeAnnotation(Annotation* a);

    QList<Annotation*>      annotations;
    AnnotationGroup*        rootGroup;
    AnnotationsLocker       annLocker;

signals:
    //annotations added to the object and have valid groups assigned
    void si_onAnnotationsAdded(const QList<Annotation*>& a);
    //annotations removed from the object and will be deleted, but still keeps references to groups and object
    void si_onAnnotationsRemoved(const QList<Annotation*>& a);
    void si_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup*);
    void si_onAnnotationModified(const AnnotationModification& md);

    void si_onGroupCreated(AnnotationGroup*);
    void si_onGroupRemoved(AnnotationGroup* p, AnnotationGroup* removed);
    void si_onGroupRenamed(AnnotationGroup*, const QString& oldName);

    friend class DeleteAnnotationsFromObjectTask;
};


class U2CORE_EXPORT AnnotationTableObjectConstraints : public GObjectConstraints {
    Q_OBJECT
public:
    AnnotationTableObjectConstraints(QObject* p = NULL);
    AnnotationTableObjectConstraints(const AnnotationTableObjectConstraints& c, QObject* p = NULL);
    int sequenceSizeToFit;
};

bool U2CORE_EXPORT annotationLessThanByRegion(const Annotation* a1, const Annotation* a2);
bool U2CORE_EXPORT annotationGreaterThanByRegion(const Annotation* a1, const Annotation* a2);


}//namespace


#endif
