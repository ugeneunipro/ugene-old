/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include <QtCore/QMutex>
#include <U2Core/Task.h>

namespace U2 {

class GObject;
class GHints;
class AnnotationTableObject;
class U2SequenceObject;
class DNAAlphabet;
class StateLock;

class AutoAnnotationsUpdater;
class AutoAnnotationsSupport;

/**
 * This object represents in-memory AnnotationTableObject.
 * Auto-annotations are used to represent temporary algorithm results,
 * such as for example restriction sites or ORFS.
 * Auto-annotations are controlled by AnnotatedDNAView.
 */
class U2CORE_EXPORT AutoAnnotationObject : public QObject
{
    Q_OBJECT
public:
                                AutoAnnotationObject( U2SequenceObject *obj, QObject *parent );
                                ~AutoAnnotationObject( );
    AnnotationTableObject *       getAnnotationObject( ) const { return aobj; }
    U2SequenceObject *          getSeqObject( ) const { return dnaObj; }
    void                        setGroupEnabled( const QString &groupName, bool enabled );
    void                        update( );
    void                        updateGroup( const QString &groupName );
    void                        emitStateChange( bool started );

    static const QString        AUTO_ANNOTATION_HINT;

signals:
    void                        si_updateStarted( );
    void                        si_updateFinshed( );

private:
    void                        handleUpdate( QList<AutoAnnotationsUpdater *> updaters );

    U2SequenceObject *          dnaObj;
    AnnotationTableObject *       aobj;
    AutoAnnotationsSupport *    aaSupport;
    QSet<QString>               enabledGroups;
};

#define AUTO_ANNOTATION_SETTINGS "auto-annotations/"

struct U2CORE_EXPORT AutoAnnotationConstraints {
                            AutoAnnotationConstraints( );

    const DNAAlphabet *     alphabet;
    GHints *                hints;
};

class U2CORE_EXPORT AutoAnnotationsUpdater : public QObject {
    Q_OBJECT
public:
    AutoAnnotationsUpdater( const QString &nm, const QString &gName, bool offByDefault = false );
    virtual             ~AutoAnnotationsUpdater( );
    const QString &     getGroupName( ) { return groupName; }
    const QString &     getName( ) { return name; }
    bool                isCheckedByDefault( ) { return checkedByDefault; }
    void                setCheckedByDefault( bool checked )  { checkedByDefault = alwaysOffByDefault ? false : checked; }
    virtual bool        checkConstraints( const AutoAnnotationConstraints &constraints ) = 0;
    virtual Task *      createAutoAnnotationsUpdateTask( const AutoAnnotationObject *aa ) = 0;

private:
    QString             groupName;
    QString             name;
    bool                checkedByDefault;
    bool                alwaysOffByDefault;
};

class U2CORE_EXPORT AutoAnnotationsSupport : public QObject {
    Q_OBJECT
public:
                                        ~AutoAnnotationsSupport( );
    void                                registerAutoAnnotationsUpdater( AutoAnnotationsUpdater *updater );
    void                                unregisterAutoAnnotationsUpdater( AutoAnnotationsUpdater *updater );
    QList<AutoAnnotationsUpdater *>     getAutoAnnotationUpdaters( );
    AutoAnnotationsUpdater*             findUpdaterByGroupName( const QString& groupName );
    AutoAnnotationsUpdater*             findUpdaterByName( const QString &name );
    static bool                         isAutoAnnotation( const AnnotationTableObject *obj );
    static bool                         isAutoAnnotation( const GObject *obj );

private:
    QList<AutoAnnotationsUpdater *>     aaUpdaters;
};

class U2CORE_EXPORT AutoAnnotationsUpdateTask : public Task {
    Q_OBJECT
public:
                            AutoAnnotationsUpdateTask( AutoAnnotationObject *aaObj, QList<Task *> subtasks );
    virtual                 ~AutoAnnotationsUpdateTask( );
    virtual void            prepare( );
    virtual void            cleanup( );
    ReportResult            report( );
    void                    setAutoAnnotationInvalid() { aaObjectInvalid = true; }
    AutoAnnotationObject *  getAutoAnnotationObject( ) { return aa; }

    static const QString    NAME;

protected:
    AutoAnnotationObject *  aa;
    U2SequenceObject *      aaSeqObj;
    StateLock *             lock;
    QList<Task *>           subTasks;
    bool                    aaObjectInvalid;
};

} // namespace U2

#endif
