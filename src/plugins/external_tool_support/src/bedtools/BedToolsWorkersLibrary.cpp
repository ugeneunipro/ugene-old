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

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Formats/FileAndDirectoryUtils.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>

#include "BedtoolsSupport.h"
#include "BedToolsWorkersLibrary.h"

namespace U2 {
namespace LocalWorkflow {

const QString SlopbedWorkerFactory::ACTOR_ID("slopbed");
static const QString GENOME_ID( "genome-id" );
static const QString B_ID( "b-id" );
static const QString L_ID( "l-id" );
static const QString R_ID( "r-id" );
static const QString S_ID( "s-id" );
static const QString PCT_ID( "pct-id" );
static const QString HEADER_ID( "header-id" );

/************************************************************************/
/* SlopbedPrompter */
/************************************************************************/
QString SlopbedPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BaseBedToolsWorker::INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Increases the size of each feature in files from %1 with bedtool slop.").arg(producerName);
    return doc;
}

/************************************************************************/
/* SlopbedWorkerFactory */
/************************************************************************/
void SlopbedWorkerFactory::init() {
    //init data path
    U2DataPath* dataPath = NULL;
    U2DataPathRegistry* dpr =  AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = dpr->getDataPathByName(GENOMES_DATA_NAME);
        if (dp && dp->isValid()){
            dataPath = dp;
        }
    }
    Descriptor desc( ACTOR_ID, SlopbedWorker::tr("Slopbed"),
        SlopbedWorker::tr("Increases the size of each feature in files using bedtools slop.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(BaseBedToolsWorker::INPUT_PORT, SlopbedWorker::tr("Input File"),
            SlopbedWorker::tr("Set of files to bedtools slop"));
        Descriptor outD(BaseBedToolsWorker::OUTPUT_PORT, SlopbedWorker::tr("Output File"),
            SlopbedWorker::tr("Output file"));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(inD, DataTypePtr(new MapDataType("sb.input-url", inM)), true);

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType("sb.output-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor outDir(BaseBedToolsWorker::OUT_MODE_ID, SlopbedWorker::tr("Output directory"),
            SlopbedWorker::tr("Select an output directory. <b>Custom</b> - specify the output directory in the 'Custom directory' parameter. "
            "<b>Workflow</b> - internal workflow directory. "
            "<b>Input file</b> - the directory of the input file."));

        Descriptor customDir(BaseBedToolsWorker::CUSTOM_DIR_ID, SlopbedWorker::tr("Custom directory"),
            SlopbedWorker::tr("Select the custom output directory."));

        Descriptor outName(BaseBedToolsWorker::OUT_NAME_ID, SlopbedWorker::tr("Output file name"),
            SlopbedWorker::tr("A name of an output file. If default of empty value is provided the output name is the name of the first file with additional extention."));

        Descriptor genomeAttrDesc(GENOME_ID, SlopbedWorker::tr("Genome"),
            SlopbedWorker::tr("In order to prevent the extension of intervals beyond chromosome boundaries, bedtools slop requires a genome file defining the length of each chromosome or contig. The format of the file is: <chromName><TAB><chromSize>. (-g)"));

        Descriptor bAttr(B_ID, SlopbedWorker::tr("Each direction increase"),
            SlopbedWorker::tr("Increase the BED/GFF/VCF entry by the same number base pairs in each direction. If this parameter is used -l and -l are ignored. Enter 0 to disable. (-b)"));

        Descriptor lAttr(L_ID, SlopbedWorker::tr("Substract from start"),
            SlopbedWorker::tr("The number of base pairs to subtract from the start coordinate. Enter 0 to disable. (-l)"));

        Descriptor rAttr(R_ID, SlopbedWorker::tr("Add to end"),
            SlopbedWorker::tr("The number of base pairs to add to the end coordinate. Enter 0 to disable. (-r)"));

        Descriptor sAttr(S_ID, SlopbedWorker::tr("Strand-based"),
            SlopbedWorker::tr("Define -l and -r based on strand. For example. if used, -l 500 for a negative-stranded feature, it will add 500 bp to the end coordinate. (-s)"));

        Descriptor pctAttr(PCT_ID, SlopbedWorker::tr("As fraction"),
            SlopbedWorker::tr("Define -l and -r as a fraction of the feature’s length. E.g. if used on a 1000bp feature, -l 0.50, will add 500 bp “upstream”. (-pct)"));

        Descriptor headerAttr(HEADER_ID, SlopbedWorker::tr("Print header"),
            SlopbedWorker::tr("Print the header from the input file prior to results. (-header)"));

        a << new Attribute( outDir, BaseTypes::NUM_TYPE(), false, QVariant(FileAndDirectoryUtils::FILE_DIRECTORY));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(BaseBedToolsWorker::OUT_MODE_ID, SlopbedWorker::tr("Custom")));
        a << customDirAttr;
        a << new Attribute( outName, BaseTypes::STRING_TYPE(), false, QVariant(BaseBedToolsWorker::DEFAULT_NAME));

        Attribute* genomeAttr = NULL;
        if (dataPath){
            const QList<QString>& dataNames = dataPath->getDataNames();
            if (!dataNames.isEmpty()){
                genomeAttr = new Attribute(genomeAttrDesc, BaseTypes::STRING_TYPE(), true, dataPath->getPathByName(dataNames.first()));
            }else{
                genomeAttr = new Attribute(genomeAttrDesc, BaseTypes::STRING_TYPE(), true);
            }
        }else{
            genomeAttr = new Attribute(genomeAttrDesc, BaseTypes::STRING_TYPE(), true);
        }
        a << genomeAttr;

        a << new Attribute( bAttr, BaseTypes::NUM_TYPE(), false, QVariant(0));
        a << new Attribute( lAttr, BaseTypes::NUM_TYPE(), false, QVariant(0));
        a << new Attribute( rAttr, BaseTypes::NUM_TYPE(), false, QVariant(0));
        a << new Attribute( sAttr, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        a << new Attribute( pctAttr, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        a << new Attribute( headerAttr, BaseTypes::BOOL_TYPE(), false, QVariant(false));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap directoryMap;
        QString fileDir = SlopbedWorker::tr("Input file");
        QString workflowDir = SlopbedWorker::tr("Workflow");
        QString customD = SlopbedWorker::tr("Custom");
        directoryMap[fileDir] = FileAndDirectoryUtils::FILE_DIRECTORY;
        directoryMap[workflowDir] = FileAndDirectoryUtils::WORKFLOW_INTERNAL;
        directoryMap[customD] = FileAndDirectoryUtils::CUSTOM;
        delegates[BaseBedToolsWorker::OUT_MODE_ID] = new ComboBoxDelegate(directoryMap);

        delegates[BaseBedToolsWorker::CUSTOM_DIR_ID] = new URLDelegate("", "", false, true);

        QVariantMap vm;
        if (dataPath){
            vm = dataPath->getDataItemsVariantMap();
        }
        delegates[GENOME_ID] = new ComboBoxWithUrlsDelegate(vm);

        QVariantMap lenMap; lenMap["minimum"] = QVariant(0); lenMap["maximum"] = QVariant(INT_MAX);
        delegates[B_ID] = new SpinBoxDelegate(lenMap);
        delegates[L_ID] = new SpinBoxDelegate(lenMap);
        delegates[R_ID] = new SpinBoxDelegate(lenMap);
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new SlopbedPrompter());
    proto->addExternalTool(ET_BEDTOOLS);

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CONVERTERS(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new SlopbedWorkerFactory());
}

/************************************************************************/
/* SlopbedWorker */
/************************************************************************/
SlopbedWorker::SlopbedWorker(Actor *a)
:BaseBedToolsWorker(a)
{

}

QVariantMap SlopbedWorker::getCustomParameters() const{
    QVariantMap res;
    const QString genomePath = getValue<QString>(GENOME_ID);
    if(!genomePath.isEmpty()){
        res["-g"] = GUrlUtils::getQuotedString(genomePath);
    }
    const int b = getValue<int>(B_ID);
    if(b!=0){
        res["-b"] = b;
    }else{
        const int l = getValue<int>(L_ID);
        const int r = getValue<int>(R_ID);
        res["-l"] = l;
        res["-r"] = r;
}
    const bool s = getValue<bool>(S_ID);
    if(s){
        res["-s"] = "";
    }
    const int pct = getValue<bool>(PCT_ID);
    if(pct){
        res["-pct"] = "";
    }
    const int header = getValue<bool>(HEADER_ID);
    if(header){
        res["-header"] = "";
    }
    return res;
}

QString SlopbedWorker::getDefaultFileName() const{
    return ".sb.bed";
}

Task *SlopbedWorker::getTask(const BedToolsSetting &settings) const{
    return new SlopbedTask(settings);
}

//////////////////////////////////////////////////////
//SlopbedTask
SlopbedTask::SlopbedTask(const BedToolsSetting &settings)
    :BaseBedToolsTask(settings){

}

QStringList SlopbedTask::getParameters(U2OpStatus &os){
    QStringList res;
    res << "slop";
    res << "-i";
    res << GUrlUtils::getQuotedString(settings.inputUrl);



    if(!settings.customParameters.contains("-g")){
        os.setError("No genome file");
        return res;
    }else{
        res << "-g";
        res << settings.customParameters["-g"].toString();
    }

    if(!(settings.customParameters.contains("-b") || (settings.customParameters.contains("-r") && settings.customParameters.contains("-l")))){
        os.setError("Need -l and -r together or -b alone.");
        return res;
    }

    if(settings.customParameters.contains("-b")){
        res << "-b";
        res << settings.customParameters["-b"].toString();
    }else{
        if(settings.customParameters.contains("-r")){
            res << "-r";
            res << settings.customParameters["-r"].toString();
        }

        if(settings.customParameters.contains("-l")){
            res << "-l";
            res << settings.customParameters["-l"].toString();
        }
    }

    if(settings.customParameters.contains("-s")){
        res << "-s";
    }

    if(settings.customParameters.contains("-pct")){
        res << "-pct";
    }

    if(settings.customParameters.contains("-header")){
        res << "-header";
    }

    return res;
}

} //LocalWorkflow
} //U2
