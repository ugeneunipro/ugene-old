/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_BOWTIE2_SUPPORT_WORKER_
#define _U2_BOWTIE2_SUPPORT_WORKER_

#include "utils/BaseShortReadsAlignerWorker.h"

namespace U2 {

class DnaAssemblyToRefTaskSettings;
class U2OpStatus;

namespace LocalWorkflow {

class Bowtie2Worker : public BaseShortReadsAlignerWorker {
    Q_OBJECT
public:
    Bowtie2Worker(Actor *p);

protected :
    QVariantMap getCustomParameters() const;
    QString getDefaultFileName() const;
    QString getBaseSubdir() const;
    DnaAssemblyToReferenceTask* getTask(const DnaAssemblyToRefTaskSettings &settings) const;
    void setGenomeIndex(DnaAssemblyToRefTaskSettings& settings);
}; // Bowtie2Worker

class Bowtie2WorkerFactory : public BaseShortReadsAlignerWorkerFactory {
public:
    static const QString ACTOR_ID;

    Bowtie2WorkerFactory() : BaseShortReadsAlignerWorkerFactory(ACTOR_ID) {}
    static void init();
    virtual Worker *createWorker(Actor *a);
}; // Bowtie2WorkerFactory

} // LocalWorkflow
} // U2

#endif // _U2_BOWTIE2_SUPPORT_WORKER_
