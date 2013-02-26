/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "PairwiseAlignmentRegistry.h"

#include "U2Algorithm/PairwiseAlignmentTask.h"
#include "U2View/PairwiseAlignmentGUIExtension.h"

#include <QtCore/QMutexLocker>

namespace U2 {

PairwiseAlignmentRegistry::PairwiseAlignmentRegistry(QObject *parent) : QObject(parent) {
}

PairwiseAlignmentRegistry::~PairwiseAlignmentRegistry() {
    foreach( PairwiseAlignmentAlgorithm* alg, algorithms.values()) {
        delete alg;
    }
}

bool PairwiseAlignmentRegistry::registerAlgorithm(PairwiseAlignmentAlgorithm* alg) {
    QMutexLocker locker(&mutex);

    if (algorithms.contains(alg->getId())){
        return false;
    }
    algorithms.insert(alg->getId(), alg);
    return true;

}

PairwiseAlignmentAlgorithm* PairwiseAlignmentRegistry::unregisterAlgorithm(const QString& id) {
    QMutexLocker locker(&mutex);

    if (!algorithms.contains(id)) {
        return NULL;
    }
    PairwiseAlignmentAlgorithm* res = algorithms.value(id);
    algorithms.remove(id);
    return res;
}

PairwiseAlignmentAlgorithm* PairwiseAlignmentRegistry::getAlgorithm(const QString& id) const {
    QMutexLocker locker(&mutex);
    return algorithms.value(id, 0);
}

QStringList PairwiseAlignmentRegistry::getRegisteredAlgorithmIds() const {
    return algorithms.keys();
}


PairwiseAlignmentAlgorithm::PairwiseAlignmentAlgorithm(const QString& _id, PairwiseAlignmentTaskFactory* tf,
                                                       PairwiseAlignmentGUIExtensionFactory* guif,
                                                       const QString& _realizationId) : id(_id) {
    realizations.insert(_realizationId, new AlgorithmRealization(_realizationId, tf, guif));
}

PairwiseAlignmentAlgorithm::~PairwiseAlignmentAlgorithm() {
    foreach(AlgorithmRealization* algReal, realizations) {
        delete algReal;
    }
}

PairwiseAlignmentTaskFactory* PairwiseAlignmentAlgorithm::getFactory(const QString& _realizationId) const {
    QMutexLocker locker(&mutex);
    return ((AlgorithmRealization*)realizations.value(_realizationId))->getTaskFactory();
}

PairwiseAlignmentGUIExtensionFactory* PairwiseAlignmentAlgorithm::getGUIExtFactory(const QString& _realizationId) const {
    QMutexLocker locker(&mutex);
    AlgorithmRealization* algReal = (AlgorithmRealization*)realizations.value(_realizationId, 0);
    assert(algReal);
    return algReal->getGUIExtFactory();
}

QString PairwiseAlignmentAlgorithm::getId() const {
    return id;
}

QStringList PairwiseAlignmentAlgorithm::getRealizationsList() const {
    QMutexLocker locker(&mutex);
    return realizations.keys();
}

bool PairwiseAlignmentAlgorithm::addAlgorithmRealization(PairwiseAlignmentTaskFactory* tf, PairwiseAlignmentGUIExtensionFactory* guif, const QString& _realizationId) {
    QMutexLocker locker(&mutex);
    if (realizations.keys().contains(_realizationId)) {
        return false;
    }
    realizations.insert(_realizationId, new AlgorithmRealization(_realizationId, tf, guif));
    return true;
}

AlgorithmRealization* PairwiseAlignmentAlgorithm::getAlgorithmRealization(QString _realizationId) const {
    QMutexLocker locker(&mutex);
    return realizations.value(_realizationId, 0);
}


AlgorithmRealization::AlgorithmRealization(QString _realizationId, PairwiseAlignmentTaskFactory* tf, PairwiseAlignmentGUIExtensionFactory* guif) :
    realizationId(_realizationId), pairwiseAlignmentTaskFactory(tf), pairwiseAlignmentGUIExtensionsFactory(guif) {
}

AlgorithmRealization::~AlgorithmRealization() {
    delete pairwiseAlignmentTaskFactory;
    delete pairwiseAlignmentGUIExtensionsFactory;
}

QString AlgorithmRealization::getRealizationId() const {
    return realizationId;
}

PairwiseAlignmentTaskFactory* AlgorithmRealization::getTaskFactory() const {
    return pairwiseAlignmentTaskFactory;
}

PairwiseAlignmentGUIExtensionFactory* AlgorithmRealization::getGUIExtFactory() const {
    return pairwiseAlignmentGUIExtensionsFactory;
}

}   //namespace
