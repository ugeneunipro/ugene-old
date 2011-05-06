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

#ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_
#define _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_

#include <U2Core/global.h>
#include <U2Core/Matrix44.h>
#include <U2Core/Task.h>
#include <U2Core/BioStruct3DObject.h>

#include <memory>

namespace U2 {

class BioStruct3D;

/** Reference to a part of BioStruct3D */
// Maybe this class shoud be merged with BioStruct3DChainSelection
class U2ALGORITHM_EXPORT BioStruct3DReference {
public:
    BioStruct3DReference(const BioStruct3DObject *_obj, const QList<int> &_chains, int _modelId = -1)
            : obj(_obj), chains(_chains), modelId(_modelId)
    {
        if (modelId == -1) {
            modelId = obj->getBioStruct3D().modelMap.keys().first();
        }
    }

    /** Pretty print structure reference description */
    QString print() const;

    const BioStruct3DObject *obj;
    QList<int> chains;

    // -1 means all models
    int modelId;
};  // class BioStruct3DReference

struct U2ALGORITHM_EXPORT StructuralAlignmentTaskSettings {
    BioStruct3DReference ref, alt;
};  // struct StructuralAlignmentTaskSettings

/** Structural alignment algorithm result */
class U2ALGORITHM_EXPORT StructuralAlignment {
public:
    double rmsd;
    Matrix44 transform;
};  // class StructuralAlignment

/** Structural alignment algorithm abstract interface */
class U2ALGORITHM_EXPORT StructuralAlignmentAlgorithm {
public:
    virtual StructuralAlignment align(const StructuralAlignmentTaskSettings &settings) = 0;
};  // class StructuralAlignmentAlgorithm

/** Task wrapper for structural alignment algorithm */
class U2ALGORITHM_EXPORT StructuralAlignmentTask : public Task {
    Q_OBJECT

public:
    StructuralAlignmentTask(StructuralAlignmentAlgorithm *algorithm, const StructuralAlignmentTaskSettings &settings);

    virtual void run();
    Task::ReportResult report();
    QString generateReport() const;

    StructuralAlignment getResult() const { return result; }
    StructuralAlignmentTaskSettings getSettings() const { return settings; }

private:
    std::auto_ptr<StructuralAlignmentAlgorithm> algorithm;
    StructuralAlignmentTaskSettings settings;

    StructuralAlignment result;
};  // class StructuralAlignmentTask

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_
