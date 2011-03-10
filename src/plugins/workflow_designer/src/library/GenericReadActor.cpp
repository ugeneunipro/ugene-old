#include "GenericReadActor.h"
#include "CoreLib.h"
#include "DocActors.h"

#include <U2Lang/WorkflowEnv.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseActorCategories.h>

#include <U2Core/DNAInfo.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectTypes.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/GUIUtils.h>

#include <QtCore/QFileInfo>
#include <limits.h>

/* TRANSLATOR U2::Workflow::SeqReadPrompter */
/* TRANSLATOR U2::Workflow::CoreLib */

namespace U2 {
namespace Workflow {

const QString GenericSeqActorProto::MODE_ATTR("mode");
const QString GenericSeqActorProto::GAP_ATTR("merge-gap");
const QString GenericSeqActorProto::ACC_ATTR("accept-accession");

const QString GenericSeqActorProto::TYPE("generic.seq");
const QString GenericMAActorProto::TYPE("generic.ma");

#define ICOLOR QColor(85,85,255)

GenericSeqActorProto::GenericSeqActorProto() : IntegralBusActorPrototype(CoreLibConstants::GENERIC_READ_SEQ_PROTO_ID)
{    
    setDisplayName(U2::Workflow::CoreLib::tr("Read sequence"));
    desc = U2::Workflow::CoreLib::tr("Reads sequences and annotations if any from local or remote files.");
    QMap<Descriptor, DataTypePtr> m;
    m[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
    m[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    m[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();

    DataTypePtr seqTypeset(new MapDataType(Descriptor(TYPE), m));
    bool treg = WorkflowEnv::getDataTypeRegistry()->registerEntry(seqTypeset);
    Q_UNUSED(treg);assert(treg);

    ports << new PortDescriptor(Descriptor(BasePorts::OUT_SEQ_PORT_ID(), U2::Workflow::CoreLib::tr("Sequence"), 
                                            U2::Workflow::CoreLib::tr("A sequence of any type (nucleotide, protein).")), 
                                seqTypeset, false, true);
    {
        Descriptor md(GenericSeqActorProto::MODE_ATTR, SeqReadPrompter::tr("Mode"), 
            SeqReadPrompter::tr("If the file contains more than one sequence, \"split\" mode sends them as is to output," 
            "while \"merge\" appends all the sequences and outputs the sole merged sequence."));
        Descriptor gd(GenericSeqActorProto::GAP_ATTR, SeqReadPrompter::tr("Merging gap"), 
            SeqReadPrompter::tr("In the \"merge\" mode, insert the specified number of gaps between original sequences."
            "This is helpful e.g. to avoid finding false positives at the merge boundaries."));
        Descriptor acd(GenericSeqActorProto::ACC_ATTR, SeqReadPrompter::tr("Accession filter"), 
            SeqReadPrompter::tr("Only report sequence with the specified accession (id)."));

        attrs << new Attribute(BaseAttributes::URL_IN_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true);
        attrs << new Attribute(md, BaseTypes::NUM_TYPE(), true, SPLIT);
        attrs << new Attribute(gd, BaseTypes::NUM_TYPE(), false, 10);
        attrs << new Attribute(acd, BaseTypes::STRING_TYPE(), false);
    }

    QMap<QString, PropertyDelegate*> delegates;
    QVariantMap modeMap; 
    modeMap[SeqReadPrompter::tr("Split")] = SPLIT;
    modeMap[SeqReadPrompter::tr("Merge")] = MERGE;
    delegates[MODE_ATTR] = new ComboBoxDelegate(modeMap);
    {
        QVariantMap m; m["minimum"] = 0; m["maximum"] = INT_MAX;
        delegates[GAP_ATTR] = new SpinBoxDelegate(m);
    }
    delegates[BaseAttributes::URL_IN_ATTRIBUTE().getId()] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true);
    setEditor(new DelegateEditor(delegates));
    setPrompter(new ReadDocPrompter(U2::Workflow::CoreLib::tr("Reads sequence(s) from <u>%1</u>.")));
    
    if(AppContext::isGUIMode()) {
        setIcon( GUIUtils::createRoundIcon(ICOLOR, 22) );
    }
}

GenericMAActorProto::GenericMAActorProto() : IntegralBusActorPrototype(CoreLibConstants::GENERIC_READ_MA_PROTO_ID) 
{
    setDisplayName(U2::Workflow::CoreLib::tr("Read alignment"));
    desc = U2::Workflow::CoreLib::tr("Reads multiple sequence alignments (MSAs) from local or remote files."
        "<p>Besides the known alignment formats, it supports composing an alignment from a set of sequences in a corresponding file (e.g. FASTA or Genbank).");

    QMap<Descriptor, DataTypePtr> m;
    m[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
    m[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    DataTypePtr blockTypeset(new MapDataType(Descriptor(TYPE), m));
    bool treg = WorkflowEnv::getDataTypeRegistry()->registerEntry(blockTypeset);
    Q_UNUSED(treg);assert(treg);

    ports << new PortDescriptor(Descriptor(BasePorts::OUT_MSA_PORT_ID(), U2::Workflow::CoreLib::tr("Multiple sequence alignment"), ""), 
                                blockTypeset, false, true);
    
    attrs << new Attribute(BaseAttributes::URL_IN_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true);
    setEditor(new DelegateEditor(BaseAttributes::URL_IN_ATTRIBUTE().getId(), new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(),true)));
    setPrompter(new ReadDocPrompter(U2::Workflow::CoreLib::tr("Reads MSA(s) from <u>%1</u>.")));
    
    if( AppContext::isGUIMode() ) {
        setIcon( GUIUtils::createRoundIcon(ICOLOR, 22) );
    }
}

bool GenericMAActorProto::isAcceptableDrop(const QMimeData* md, QVariantMap* params) const {
    QList<DocumentFormat*> fs;
    QString url = WorkflowUtils::getDropUrl(fs, md);
    foreach(DocumentFormat* f, fs) {
        if (f->getSupportedObjectTypes().contains(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            if (params != NULL) {
                params->insert(BaseAttributes::URL_IN_ATTRIBUTE().getId(), url);
            }
            return true;
        }
    }
    /*DocumentFormatConstraints sc;
    sc.supportedObjectTypes.append(GObjectTypes::SEQUENCE);
    foreach(DocumentFormat* f, fs) {
        if (f->checkConstraints(sc)) {
            if (params) {
                params->insert(CoreLib::URL_ATTR_ID, url);
            }
            return true;
        }
    }*/
    if (QFileInfo(url).isDir()) {
        if (params) {
            params->insert(BaseAttributes::URL_IN_ATTRIBUTE().getId(), url + "/*");
        }
        return true;
    }

    return false;
}

bool GenericSeqActorProto::isAcceptableDrop(const QMimeData* md, QVariantMap* params) const {
    QList<DocumentFormat*> fs;
    const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
    if (gomd && params) {
        const DNASequenceObject* obj = qobject_cast<const DNASequenceObject*>(gomd->objPtr.data());
        if (obj) {
            params->insert(BaseAttributes::URL_IN_ATTRIBUTE().getId(), obj->getDocument()->getURLString());
            QString acc = DNAInfo::getPrimaryAccession(obj->getDNASequence().info);
            if (acc.isEmpty()) {
                acc = obj->getDNASequence().getName();
            }
            params->insert(ACC_ATTR, acc);
        }
    }

    QString url = WorkflowUtils::getDropUrl(fs, md);
    foreach(DocumentFormat* f, fs) {
        if (f->getSupportedObjectTypes().contains(GObjectTypes::SEQUENCE)) {
            if (params) {
                params->insert(BaseAttributes::URL_IN_ATTRIBUTE().getId(), url);
            }
            return true;
        }
    }
    if (QFileInfo(url).isDir()) {
        if (params) {
            params->insert(BaseAttributes::URL_IN_ATTRIBUTE().getId(), url + "/*");
        }
        return true;
    }

    return false;
}

QString SeqReadPrompter::composeRichDoc() {
    //TODO finish
    //BusPort* input = qobject_cast<BusPort*>(target->getPort(CoreLib::DATA_PORT_ID));
    QString url = getURL(BaseAttributes::URL_IN_ATTRIBUTE().getId());
    return QString("%1").arg(url);
}

}//namespace Workflow
}//namespace U2
