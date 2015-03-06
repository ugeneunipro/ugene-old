/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/AnnotationModification.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "AnnotationGroup.h"

const QString U2::AnnotationGroup::ROOT_GROUP_NAME("/");
const QChar U2::AnnotationGroup::GROUP_PATH_SEPARATOR('/');

namespace U2 {

AnnotationGroup::AnnotationGroup() :
    U2Entity(), parentObject(NULL)
{

}

AnnotationGroup::AnnotationGroup(const U2DataId &_featureId, AnnotationTableObject *_parentObject)
    : U2Entity(_featureId), parentObject(_parentObject)
{
    SAFE_POINT(NULL != parentObject && hasValidId(), "Invalid feature table detected!", );
}

AnnotationGroup::AnnotationGroup(const AnnotationGroup &other) :
    U2Entity(other),
    parentObject(other.parentObject)
{

}

bool AnnotationGroup::isValidGroupName(const QString &name, bool pathMode) {
    if (name.isEmpty()) {
        return false;
    }
    //todo: optimize
    QBitArray validChars = TextUtils::ALPHA_NUMS;
    validChars['_'] = true;
    validChars['-'] = true;
    validChars[' '] = true;
    validChars['\''] = true;

    if (pathMode) {
        validChars[GROUP_PATH_SEPARATOR.toLatin1()] = true;
    }
    QByteArray groupName = name.toLocal8Bit();
    if (!TextUtils::fits(validChars, groupName.constData(), groupName.size())) {
        return false;
    }
    if (' ' == groupName[0] || ' ' == groupName[groupName.size()-1]) {
        return false;
    }
    return true;
}

void AnnotationGroup::findAllAnnotationsInGroupSubTree(QList<Annotation> &set) const {
    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations(id, parentObject->getEntityRef().dbiRef, os, Recursive, Nonroot);
    SAFE_POINT_OP(os, );

    foreach (const U2Feature &feature, subfeatures) {
        if (feature.name.isEmpty()) { // part of joined annotation
            continue;
        }
        bool contains = false;
        foreach (const Annotation &a, set) {
            if (a.id == feature.id) {
                contains = true;
                break;
            }
        }
        if (!contains) {
            set << Annotation(feature.id, parentObject);
        }
    }
}

namespace {

ParentFeatureStatus isRootGroup(const AnnotationGroup &group, const U2DbiRef &dbiRef, U2OpStatus &os) {
    const U2Feature feature = U2FeatureUtils::getFeatureById(group.id, U2Feature::Group, dbiRef, os);
    CHECK_OP(os, Nonroot);
    return feature.rootFeatureId.isEmpty() ? Root : Nonroot;
}

}

QList<Annotation> AnnotationGroup::getAnnotations(bool recurcively) const {
    QList<Annotation> resultAnnotations;

    U2OpStatusImpl os;
    const U2DbiRef dbiRef = parentObject->getEntityRef().dbiRef;
    const OperationScope scope = recurcively ? Recursive : Nonrecursive;
    const ParentFeatureStatus parent = isRootGroup(*this, dbiRef, os);
    SAFE_POINT_OP(os, resultAnnotations);
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations(id, dbiRef, os, scope, parent);
    SAFE_POINT_OP(os, resultAnnotations);

    foreach (const U2Feature &feature, subfeatures) {
        if (!feature.name.isEmpty()) {
            resultAnnotations << Annotation(feature.id, parentObject);
        }
    }
    return resultAnnotations;
}

Annotation AnnotationGroup::addAnnotation(const AnnotationData &a) {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures(a, parentObject->getRootFeatureId(),
        id, parentObject->getEntityRef().dbiRef, os);

    Annotation result(feature.id, parentObject);
    SAFE_POINT_OP(os, result);

    parentObject->setModified(true);
    parentObject->emit_onAnnotationsAdded(QList<Annotation>() << result);

    return result;
}

QList<Annotation> AnnotationGroup::addFeatures(const QList<U2Feature> &features) {
    QList<Annotation> anns;

    foreach (const U2Feature &f, features) {
        if (f.parentFeatureId == id) {
            anns << Annotation(f.id, parentObject);
        }
    }

    parentObject->setModified(true);
    parentObject->emit_onAnnotationsAdded(anns);

    return anns;
}

void AnnotationGroup::addAnnotation(const Annotation &a) {
    SAFE_POINT(a.getGObject() == parentObject, "Illegal object!", );
    U2OpStatusImpl os;

    U2FeatureUtils::updateFeatureParent(a.id, id, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, );

    parentObject->setModified(true);
    AnnotationGroupModification md(AnnotationModification_AddedToGroup, a, *this);
    parentObject->emit_onAnnotationModified(md);
}

void AnnotationGroup::removeAnnotation(const Annotation &a) {
    SAFE_POINT(a.getGObject() == parentObject, "Attempting to remove annotation belonging to different object!", );
    parentObject->removeAnnotation(a);
}

void AnnotationGroup::removeAnnotations(const QList<Annotation> &annotations) {
    parentObject->removeAnnotations(annotations);
}

QList<AnnotationGroup> AnnotationGroup::getSubgroups() const {
    QList<AnnotationGroup> result;

    const U2DbiRef dbiRef = parentObject->getEntityRef().dbiRef;
    U2OpStatusImpl os;
    const ParentFeatureStatus parent = isRootGroup(*this, dbiRef, os);
    SAFE_POINT_OP(os, result);
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups(id, dbiRef, os, Nonrecursive, parent);
    SAFE_POINT_OP(os, result);

    foreach (const U2Feature &sub, subfeatures) {
        result << AnnotationGroup(sub.id, parentObject);
    }
    return result;
}

void AnnotationGroup::removeSubgroup(AnnotationGroup &g) {
    SAFE_POINT(g.getGObject() == parentObject, "Attempting to remove group belonging to different object!", );
    U2OpStatusImpl os;

    g.clear();

    U2FeatureUtils::removeFeature(g.id, U2Feature::Group, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, );

    parentObject->setModified(true);
    parentObject->emit_onGroupRemoved(*this, g);
}

QString AnnotationGroup::getName() const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById(id, U2Feature::Group, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, QString());
    return feature.name;
}

void AnnotationGroup::setName(const QString &newName) {
    SAFE_POINT(!newName.isEmpty(), "Attempting to set an empty name for a group!", );
    U2OpStatusImpl os;
    U2FeatureUtils::updateFeatureName(id, U2Feature::Group, newName, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, );

    parentObject->setModified(true);
    parentObject->emit_onGroupRenamed(*this);
}

QString AnnotationGroup::getGroupPath() const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById(id, U2Feature::Group, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, QString());

    if (feature.parentFeatureId.isEmpty()) {
        return QString();
    }

    const U2Feature parentFeature = U2FeatureUtils::getFeatureById(feature.parentFeatureId, U2Feature::Group,
        parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, QString());

    if (parentFeature.parentFeatureId.isEmpty()) {
        return feature.name;
    } else {
        AnnotationGroup parentGroup(parentFeature.id, parentObject);
        return parentGroup.getGroupPath() + GROUP_PATH_SEPARATOR + feature.name;
    }
}

AnnotationTableObject * AnnotationGroup::getGObject() const {
    return parentObject;
}

AnnotationGroup AnnotationGroup::getParentGroup() const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById(id, U2Feature::Group, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, *this);

    if (0 == U2DbiUtils::toDbiId(feature.parentFeatureId)) {
        return *this;
    }

    const U2Feature parentFeature = U2FeatureUtils::getFeatureById(feature.parentFeatureId, U2Feature::Group,
        parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, *this);
    return AnnotationGroup(parentFeature.id, parentObject);
}

AnnotationGroup AnnotationGroup::getSubgroup(const QString &path, bool create) {
    if (path.isEmpty()) {
        return *this;
    }
    const int separatorFirstPosition = path.indexOf(GROUP_PATH_SEPARATOR);
    const QString subgroupName = (0 > separatorFirstPosition) ? path
        : ((0 == separatorFirstPosition) ? path.mid(1) : path.left(separatorFirstPosition));

    const U2DbiRef dbiRef = parentObject->getEntityRef().dbiRef;
    AnnotationGroup subgroup(*this);
    U2OpStatusImpl os;
    const ParentFeatureStatus parent = isRootGroup(*this, dbiRef, os);
    SAFE_POINT_OP(os, subgroup);

    const QList<U2Feature> subfeatures = U2FeatureUtils::getFeaturesByName(parentObject->getRootFeatureId(),
        subgroupName, U2Feature::Group, dbiRef, os);
    SAFE_POINT_OP(os, subgroup);
    foreach (const U2Feature &feature, subfeatures) {
        if (feature.parentFeatureId == id) {
            subgroup = AnnotationGroup(feature.id, parentObject);
            break;
        }
    }

    if (id == subgroup.id && create) {
        const U2Feature subgroupFeature = U2FeatureUtils::exportAnnotationGroupToFeature(subgroupName,
            parentObject->getRootFeatureId(), id, dbiRef, os);
        SAFE_POINT_OP(os, subgroup);
        subgroup = AnnotationGroup(subgroupFeature.id, parentObject);

        parentObject->emit_onGroupCreated(subgroup);
    }
    if (0 >= separatorFirstPosition || id == subgroup.id) {
        return subgroup;
    }
    AnnotationGroup result = subgroup.getSubgroup(path.mid(separatorFirstPosition + 1), create);
    return result;
}

void AnnotationGroup::getSubgroupPaths(QStringList &res) const {
    if (getParentGroup().id != id) {
        res << getGroupPath();
    }

    U2OpStatusImpl os;
    const QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups(id, parentObject->getEntityRef().dbiRef, os, Nonrecursive);
    SAFE_POINT_OP(os,);

    foreach (const U2Feature &sub, subfeatures) {
        AnnotationGroup subgroup(sub.id, parentObject);
        subgroup.getSubgroupPaths(res);
    }
}

void AnnotationGroup::clear() {
    const QList<Annotation> subAnns = getAnnotations();
    if (!subAnns.isEmpty()) {
        removeAnnotations(subAnns);
    }

    const QList<AnnotationGroup> subGroups = getSubgroups();
    foreach (AnnotationGroup sub, subGroups) {
        removeSubgroup(sub);
    }
}

int AnnotationGroup::getGroupDepth() const {
    const AnnotationGroup parentGroup = getParentGroup();
    return 1 + ((parentGroup.id == id) ? 0 : parentGroup.getGroupDepth());
}

bool AnnotationGroup::isParentOf(const AnnotationGroup &g) const {
    if (g.getGObject() != parentObject || g.id == id) {
        return false;
    }
    U2OpStatusImpl os;
    bool result = U2FeatureUtils::isChild(g.id, id, parentObject->getEntityRef().dbiRef, os);
    SAFE_POINT_OP(os, false);
    return result;
}

bool AnnotationGroup::isTopLevelGroup() const {
    const AnnotationGroup parentGroup = getParentGroup();
    return (parentGroup.id != id && parentGroup.getParentGroup().id == parentGroup.id);
}

bool AnnotationGroup::operator ==(const AnnotationGroup &other) const {
    return id == other.id && parentObject == other.getGObject();
}

namespace {

bool registerAnnotationGroupMeta() {
    qRegisterMetaType<AnnotationGroup>("AnnotationGroup");
    return true;
}
static const bool annotationGroupMetaRegistered = registerAnnotationGroupMeta();

}

} // namespace U2
