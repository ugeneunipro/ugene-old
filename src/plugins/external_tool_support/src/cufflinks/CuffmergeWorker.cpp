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

#include "CuffmergeWorker.h"

#include <U2Core/L10n.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>


namespace U2 {
namespace LocalWorkflow {

/*****************************
 * CuffmergeWorkerFactory
 *****************************/
const QString CuffmergeWorkerFactory::ACTOR_ID("cuffmerge");

const QString REF_ANNOTATION("ref-annotation");
const QString REF_SEQ("ref-seq");
const QString CUFFCOMPARE_TOOL_PATH("cuffcompare-tool-path");
const QString EXT_TOOL_PATH("path");
const QString TMP_DIR_PATH("tmp-dir");


void CuffmergeWorkerFactory::init()
{
    QList<PortDescriptor*> portDescriptors;
    QList<Attribute*> attributes;

    // Description of the element
    Descriptor cuffmergeDescriptor(ACTOR_ID,
        CuffmergeWorker::tr("Merge Assemblies with Cuffmerge"),
        CuffmergeWorker::tr("Cuffmerge merges together several assemblies."
        " It also handles running Cuffcompare for you, and automatically"
        " filters a number of transfrags that are probably artifacts."
        " If you have a reference file available, you can provide it"
        " to Cuffmerge in order to gracefully merge input (e.g. novel) isoforms and"
        " known isoforms and maximize overall assembly quality."));

    // Define parameters of the element
    Descriptor refAnnotation(REF_ANNOTATION,
        CuffmergeWorker::tr("Reference annotation"),
        CuffmergeWorker::tr("Merge the input assemblies together with"
        " this reference annotation"));

    Descriptor refSeq(REF_SEQ,
        CuffmergeWorker::tr("Reference sequence"),
        CuffmergeWorker::tr("The genomic DNA sequences for the reference."
        " It is used to assist in classifying transfrags and excluding"
        " artifacts (e.g. repeats). For example, transcripts consisting"
        " mostly of lower-case bases are classified as repeats."));

    Descriptor cuffcompareToolPath(CUFFCOMPARE_TOOL_PATH,
        CuffmergeWorker::tr("Cuffcompare tool path"),
        CuffmergeWorker::tr("The path to the Cuffcompare external tool in UGENE"));

    Descriptor extToolPath(EXT_TOOL_PATH,
        CuffmergeWorker::tr("Cuffmerge tool path"),
        CuffmergeWorker::tr("The path to the Cuffmerge external tool in UGENE"));

    Descriptor tmpDir(TMP_DIR_PATH,
        CuffmergeWorker::tr("Temporary directory"),
        CuffmergeWorker::tr("The directory for temporary files"));

    attributes << new Attribute(refAnnotation, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(refSeq, BaseTypes::STRING_TYPE(), false, QVariant(""));
    attributes << new Attribute(cuffcompareToolPath, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
    attributes << new Attribute(extToolPath, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));
    attributes << new Attribute(tmpDir, BaseTypes::STRING_TYPE(), true, QVariant(L10N::defaultStr()));

    // Create the actor prototype
    ActorPrototype* proto = new IntegralBusActorPrototype(cuffmergeDescriptor,
        portDescriptors,
        attributes);

    // Values range of some parameters
    QMap<QString, PropertyDelegate*> delegates;

    delegates[REF_ANNOTATION] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[REF_SEQ] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), "", false);
    delegates[CUFFCOMPARE_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);

    // Init and register the actor prototype
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new CuffmergePrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(
        BaseActorCategories::CATEGORY_RNA_SEQ(),
        proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new CuffmergeWorkerFactory());
}


/*****************************
 * CuffmergePrompter
 *****************************/
CuffmergePrompter::CuffmergePrompter(Actor* parent)
    : PrompterBase<CuffmergePrompter>(parent)
{
}


QString CuffmergePrompter::composeRichDoc()
{
    QString result = CuffmergeWorker::tr("Merges together supplied assemblies.");

    return result;
}


/*****************************
 * CuffmergeWorker
 *****************************/
CuffmergeWorker::CuffmergeWorker(Actor* actor)
    : BaseWorker(actor),
      input(NULL),
      output(NULL)
{
}


void CuffmergeWorker::init()
{
}


Task* CuffmergeWorker::tick()
{
    return NULL;
}


void CuffmergeWorker::sl_taskFinished()
{
}


void CuffmergeWorker::cleanup()
{
}

} // namespace LocalWorkflow
} // namespace U2
