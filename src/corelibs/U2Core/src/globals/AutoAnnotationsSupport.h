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

#ifndef _U2_AUTO_ANNOTATIONS_SUPPORT_H_
#define _U2_AUTO_ANNOTATIONS_SUPPORT_H_

#include <QtCore/QSet>
#include <U2Core/Task.h>

namespace U2 {

class AnnotationTableObject;
class DNASequenceObject;
class DNAAlphabet;
class StateLock;

// This object represents in-memory AnnotationTableObject.
// Auto-annotations are used to represent temporary algorithm results,
// such as for example restriction sites or ORFS.
// Auto-annotations are controlled by AnnotatedDNAView.

class AutoAnnotationsUpdater;
class AutoAnnotationsSupport;

class U2CORE_EXPORT AutoAnnotationObject : public QObject 
{
    Q_OBJECT
public:
    AutoAnnotationObject(DNASequenceObject* obj);
    ~AutoAnnotationObject();
    AnnotationTableObject* getAnnotationObject() const { return aobj; }
    DNASequenceObject* getSeqObject() const {return dnaObj; }
    void setGroupEnabled(const QString& groupName, bool enabled);
    void lock();
    void unlock();
    void update();
    void update(AutoAnnotationsUpdater* updater);
public slots:
    void updateGroup(const QString& groupName);
private:
    DNASequenceObject* dnaObj;
    AnnotationTableObject*  aobj;
    AutoAnnotationsSupport* aaSupport;
    QSet<QString> enabledGroups;
    StateLock* stateLock;
};

#define AUTO_ANNOTATION_SETTINGS "auto-annotations/"

struct AutoAnnotationConstraints {
    AutoAnnotationConstraints() : alphabet(NULL) {}
    DNAAlphabet* alphabet;
};

class U2CORE_EXPORT AutoAnnotationsUpdater : public QObject {
    Q_OBJECT
private:
    QString groupName;
    QString name;
    bool checkedByDefault;
public:
    AutoAnnotationsUpdater(const QString& nm, const QString& gName);
    virtual ~AutoAnnotationsUpdater();
    const QString& getGroupName() { return groupName; }
    const QString& getName() { return name; }
    bool isCheckedByDefault() { return checkedByDefault; }
    void setCheckedByDefault(bool checked) { checkedByDefault = checked; }
    virtual bool checkConstraints(const AutoAnnotationConstraints& constraints) = 0;   
    virtual Task* createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa) = 0;
};

class U2CORE_EXPORT AutoAnnotationsSupport : public QObject
{
    Q_OBJECT 
public:
    ~AutoAnnotationsSupport();
    void registerAutoAnnotationsUpdater(AutoAnnotationsUpdater* updater);
    QList<AutoAnnotationsUpdater*> getAutoAnnotationUpdaters();
    AutoAnnotationsUpdater* findUpdaterByGroupName(const QString& groupName);
    AutoAnnotationsUpdater* findUpdaterByName(const QString& name);
    void updateAnnotationsByGroup(const QString& groupName);
signals:
    void si_updateAutoAnnotationsGroupRequired(const QString& groupName);
private:
    QList<AutoAnnotationsUpdater*> aaUpdaters;
};

class U2CORE_EXPORT AutoAnnotationsUpdateTask : public Task {
public:
    AutoAnnotationsUpdateTask(AutoAnnotationObject* aaObj, Task* subtask); 
    virtual void prepare();
    virtual ~AutoAnnotationsUpdateTask(); 
protected:
    AutoAnnotationObject* aa;
    Task* subtask;
};


} // namespace

#endif
