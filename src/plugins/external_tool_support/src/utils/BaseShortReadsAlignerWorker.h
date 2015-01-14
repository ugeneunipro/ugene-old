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

#ifndef _U2_BASE_SHORT_READS_ALIGNER_WORKER_H_
#define _U2_BASE_SHORT_READS_ALIGNER_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Core/GUrl.h>


namespace U2 {

class DnaAssemblyToRefTaskSettings;
class DnaAssemblyToReferenceTask;

namespace LocalWorkflow {

const QString IN_PORT_DESCR("in-data");
const QString REFERENCE_GENOME("reference");

class BaseShortReadsAlignerWorker: public BaseWorker {
    Q_OBJECT
public:
    BaseShortReadsAlignerWorker(Actor *a, const QString& algName);
    void init();
    Task * tick();
    void cleanup(){}
    virtual bool isReady();

protected:
    virtual QVariantMap getCustomParameters() const {return QVariantMap();}
    DnaAssemblyToRefTaskSettings getSettings(U2OpStatus &os);
    virtual void setGenomeIndex(DnaAssemblyToRefTaskSettings& settings) = 0;
    virtual QString getDefaultFileName() const = 0;
    virtual QString getBaseSubdir() const = 0;
    virtual DnaAssemblyToReferenceTask* getTask(const DnaAssemblyToRefTaskSettings &settings) const = 0;
protected:
    QString      algName;
    IntegralBus *inChannel;
    IntegralBus *inPairedChannel;
    IntegralBus *output;
    bool pairedReadsInput;

public slots:
    void sl_taskFinished();
};

class ShortReadsAlignerSlotsValidator : public PortValidator {
public:
    bool validate(const IntegralBusPort *port, ProblemList &problemList) const;
};

class BaseShortReadsAlignerWorkerFactory : public DomainFactory {
protected:
    BaseShortReadsAlignerWorkerFactory(const QString& actorId) : DomainFactory(actorId) {}

    static QList<PortDescriptor*> getPortDescriptors();

    static void addCommonAttributes(QList<Attribute*>& attrs, QMap<QString, PropertyDelegate*>& delegates);
};

class ShortReadsAlignerPrompter : public PrompterBase<ShortReadsAlignerPrompter> {
    Q_OBJECT
public:
    ShortReadsAlignerPrompter(Actor *p = NULL) : PrompterBase<ShortReadsAlignerPrompter>(p) {}

protected:
    QString composeRichDoc();
};

} //LocalWorkflowa
} //U2

#endif //_U2_BASE_SHORT_READS_ALIGNER_WORKER_H_
