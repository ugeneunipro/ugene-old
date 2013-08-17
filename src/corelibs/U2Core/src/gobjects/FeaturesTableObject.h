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

#ifndef _U2_FEATURES_TABLE_OBJECT_H_
#define _U2_FEATURES_TABLE_OBJECT_H_

#include <U2Core/AnnotationTableObject.h>


namespace U2 {

class U2CORE_EXPORT FeaturesTableObject: public GObject {
    Q_OBJECT
    friend class Annotation;
    friend class AnnotationGroup;
    friend class AnnotationTableObject;

public:
    FeaturesTableObject(const QString& objectName, const U2DbiRef& dbiRef, const QVariantMap& hintsMap = QVariantMap());
    ~FeaturesTableObject();

    const QList<Annotation*>& getAnnotations() const {return aObject->getAnnotations();}
    AnnotationGroup* getRootGroup() const {return rootGroup;}
    void addAnnotation(Annotation* a, const QString& groupName = QString());
    void addAnnotation(Annotation* a, const QList<QString>& groupsNames);
    void addAnnotations(const QList<Annotation*>& annotations, const QString& groupName = QString());
    void removeAnnotation(Annotation* a);
    void removeAnnotations(const QList<Annotation*>& annotations);
    virtual GObject* clone(const U2DbiRef& ref, U2OpStatus& os) const;
    void selectAnnotationsByName(const QString& name, QList<Annotation*>& res);
    bool checkConstraints(const GObjectConstraints* c) const;
    void removeAnnotationsInGroup(const QList<Annotation*>& _annotations, AnnotationGroup *group);
    void releaseLocker();
    bool isLocked() const;
    void cleanAnnotations();

    AnnotationTableObject*  getAnnotationTableObject() { return aObject; }

    //////////////////////////////////////////////////////////////////////////
    // Direct features interface (without sync with annotations)

    /** Get the root feature */
    U2Feature getRootFeature() const { return rootFeature; }

    /** Set sequence for the dummy root feature and therefore for all the features of the object */
    void setRootFeatureSequenceId (const U2DataId &id){ rootFeature.sequenceId = id; }

    /**
     * Add a new feature. Set its id to the correct value.
     * If not specified otherwise, parentFeatureId is set to the root feature.
     */
    void addFeature(U2Feature &f, U2OpStatus &os, bool create = false);

    /**
     * Add a new feature. Set its id to the correct value.
     * Also add the list of keys to the feature.
     * If not specified otherwise, parentFeatureId is set to the root feature.
     */
    void addFeature(U2Feature &f, QList<U2FeatureKey> keys, U2OpStatus &os, bool create = false);

    /** Retrieve a feature by its id */
    U2Feature getFeature(U2DataId id, U2OpStatus &os);

    /**
    * Find all child features of a given feature.
    * If recursive is false, returns only direct children.
    * Otherwise returns all features in the subtree.
    */
    QList<U2Feature> getSubfeatures(U2DataId parentFeatureId, U2OpStatus &os, bool recursive = false);

    /** Get features that intersects the specified region */
    U2DbiIterator<U2Feature>* getFeatures(const U2Region &range, U2OpStatus & os);

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

private:
    /** Import an annotation to the database */
    void importToDbi(Annotation* a);

    /** Wrapped annotations object */
    AnnotationTableObject*  aObject;

    /** Dummy root feature */
    U2Feature rootFeature;

signals:
    //annotations added to the object and have valid groups assigned
    //the same signal of ATO is ignored
    void si_onAnnotationsAdded(const QList<Annotation*>& a);
    //annotations removed from the object and will be deleted, but still keeps references to groups and object
    //the same signal of ATO is ignored
    void si_onAnnotationsRemoved(const QList<Annotation*>& a);
    void si_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup*);
    void si_onAnnotationModified(const AnnotationModification& md);

    void si_onGroupCreated(AnnotationGroup*);
    void si_onGroupRemoved(AnnotationGroup* p, AnnotationGroup* removed);
    void si_onGroupRenamed(AnnotationGroup*, const QString& oldName);

    friend class DeleteAnnotationsFromObjectTask;

protected slots:
    void sl_onAnnotationsRemoved(const QList<Annotation*>& a);
    void sl_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup*);
    void sl_onAnnotationModified(const AnnotationModification& md);

    void sl_onGroupCreated(AnnotationGroup*);
    void sl_onGroupRemoved(AnnotationGroup* p, AnnotationGroup* removed);
    void sl_onGroupRenamed(AnnotationGroup*, const QString& oldName);
};

} // namespace

#endif
