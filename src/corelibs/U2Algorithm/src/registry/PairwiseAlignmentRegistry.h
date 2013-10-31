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

#ifndef _U2_PAIRWISE_ALIGNMENT_REGISTRY_H_
#define _U2_PAIRWISE_ALIGNMENT_REGISTRY_H_

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#include "U2Core/global.h"

namespace U2 {

class PairwiseAlignmentAlgorithm;
class PairwiseAlignmentTaskFactory;
class PairwiseAlignmentGUIExtensionFactory;
class AlgorithmRealization;

class U2ALGORITHM_EXPORT PairwiseAlignmentRegistry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PairwiseAlignmentRegistry)

public:
    PairwiseAlignmentRegistry(QObject* pOwn = 0);
    ~PairwiseAlignmentRegistry();

    bool registerAlgorithm(PairwiseAlignmentAlgorithm* _alg);
    PairwiseAlignmentAlgorithm* unregisterAlgorithm(const QString& id);
    PairwiseAlignmentAlgorithm* getAlgorithm(const QString& id) const;

    QStringList getRegisteredAlgorithmIds() const;
private:
    mutable QMutex mutex;
    QMap<QString, PairwiseAlignmentAlgorithm*> algorithms;
};

class U2ALGORITHM_EXPORT PairwiseAlignmentAlgorithm {
    Q_DISABLE_COPY(PairwiseAlignmentAlgorithm)

public:
    PairwiseAlignmentAlgorithm(const QString& _id, PairwiseAlignmentTaskFactory* tf, PairwiseAlignmentGUIExtensionFactory* guif, const QString& _realizationId = QString("default"));
    virtual ~PairwiseAlignmentAlgorithm();
    virtual PairwiseAlignmentTaskFactory* getFactory(const QString& _realizationId = QString("default")) const;
    virtual PairwiseAlignmentGUIExtensionFactory* getGUIExtFactory(const QString& realizationId = QString("default")) const;

    QString getId() const;
    QStringList getRealizationsList() const;
    bool addAlgorithmRealization(PairwiseAlignmentTaskFactory* tf, PairwiseAlignmentGUIExtensionFactory* guif, const QString& _realizationId);
    AlgorithmRealization* getAlgorithmRealization(QString _realizationId) const;
protected:
    mutable QMutex mutex;
    QString id;
    QMap<QString, AlgorithmRealization*> realizations;
};

class AlgorithmRealization {
public:
    AlgorithmRealization(QString _realizationId, PairwiseAlignmentTaskFactory* tf, PairwiseAlignmentGUIExtensionFactory* guif);
    ~AlgorithmRealization();

    QString getRealizationId() const;
    PairwiseAlignmentTaskFactory* getTaskFactory() const;
    PairwiseAlignmentGUIExtensionFactory* getGUIExtFactory() const;

private:
    QString realizationId;
    PairwiseAlignmentTaskFactory* pairwiseAlignmentTaskFactory;
    PairwiseAlignmentGUIExtensionFactory* pairwiseAlignmentGUIExtensionsFactory;
};

}   //namespace

#endif // _U2_PAIRWISE_ALIGNMENT_REGISTRY_H_
