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

#include <U2Core/MAlignmentObject.h>

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/IOAdapter.h>

#include <U2Formats/SAMFormat.h>

#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2Algorithm/DnaAssemblyMultiTask.h>

#include <U2View/DnaAssemblyUtils.h>


#include "DnaAssemblyTests.h"

namespace U2 {

#define REF_SEQ_ATTR "ref-seq"
#define INDEX_FILE_ATTR "index-file"
#define METHOD_NAME_ATTR "assembly-method"
#define SHORT_READS_ATTR "shortreads"
#define RES_OBJ_NAME "res-index"
#define CUSTOM_ATTR "custom-options"

void GTest_DnaAssemblyToReferenceTask::init(XMLTestFormat*, const QDomElement& el)  {
    
    refSeqUrl = el.attribute(REF_SEQ_ATTR);
    if (refSeqUrl.isEmpty()) {
        failMissingValue(REF_SEQ_ATTR);
        return;
    }
    refSeqUrl.prepend(env->getVar("COMMON_DATA_DIR") + "/");

    indexFileName = el.attribute(INDEX_FILE_ATTR);
    if (!indexFileName.isEmpty()) {
        indexFileName.prepend(env->getVar("COMMON_DATA_DIR") + "/");
    }

    algName = el.attribute(METHOD_NAME_ATTR);
    if (algName.isEmpty()) {
        failMissingValue(METHOD_NAME_ATTR);
        return;
    }

    QString shortReads = el.attribute(SHORT_READS_ATTR);
    if (shortReads.isEmpty()) {
        failMissingValue(SHORT_READS_ATTR);
        return;
    }
    
    objName = el.attribute(RES_OBJ_NAME);
    if (objName.isEmpty()) {
        failMissingValue(RES_OBJ_NAME);
        return;
    }
    
    QStringList shortReadList = shortReads.split(";");
    if (shortReadList.isEmpty()) {
        setError("No short reads urls are found in test");
        return;
    }


   QString buf = el.attribute(CUSTOM_ATTR);
   QStringList customAttrs = buf.split(",");
   foreach (const QString& attr, customAttrs) {
       QStringList keyValPair = attr.split('=');
       if (keyValPair.size() == 2) {
           QString optName = keyValPair[0];
           QString optVal = keyValPair[1];
           customOptions.insert(optName, optVal);
       }
   }

    foreach (const QString& url, shortReadList) {
        shortReadUrls.append(GUrl(env->getVar("COMMON_DATA_DIR") + "/" + url));
    }
    loadResultTask = NULL;
    assemblyMultiTask = NULL;
}

void GTest_DnaAssemblyToReferenceTask::prepare()
{
    expectedObj = getContext<MAlignmentObject>(this, objName);
    if(expectedObj == NULL){
        stateInfo.setError(  QString("Error can't cast to malignment object from GObject %1").arg(objName) );
        return;
    }
    
    QString dir(env->getVar("TEMP_DATA_DIR"));
    if (!QDir(dir).exists()) {
        bool ok = QDir::root().mkpath(dir);
        if (!ok) {
            return;
        }
    }
    QString id = QString::number(rand());
    resultFileName = GUrlUtils::rollFileName(dir+"/"+GUrl(refSeqUrl).baseFileName() + "_" + id + "_aligned.sam",DocumentUtils::getNewDocFileNameExcludesHint());
    DnaAssemblyToRefTaskSettings settings;
    if (indexFileName.isEmpty()) {
        indexFileName = GUrlUtils::rollFileName(dir+"/"+GUrl(refSeqUrl).baseFileName() + "_index_" + id,DocumentUtils::getNewDocFileNameExcludesHint());
        settings.prebuiltIndex = false;
    } else {
        settings.prebuiltIndex = true;
    }

    settings.openView = false;
    settings.algName = algName;
    settings.refSeqUrl = refSeqUrl;
    settings.indexFileName = indexFileName;
    settings.resultFileName = resultFileName;
    foreach (const GUrl& url, shortReadUrls) {
        settings.shortReadSets.append( url );
    }
    foreach (const QString& optionName, customOptions.keys()) {
        settings.setCustomValue(optionName, customOptions.value(optionName));
    }

    assemblyMultiTask = new DnaAssemblyMultiTask( settings,  false);
    addSubTask(assemblyMultiTask);
    
    SAMFormat* samFormat = new SAMFormat();
    loadResultTask = new LoadDocumentTask(samFormat, settings.resultFileName, AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
    samFormat->setParent(loadResultTask);
    addSubTask(loadResultTask);
}

Task::ReportResult GTest_DnaAssemblyToReferenceTask::report() {
    if (hasError() || isCanceled()) {
        return ReportResult_Finished;
    }
    
    Document* resultDoc = loadResultTask->getDocument();
    QList<GObject*> resObjects = resultDoc->getObjects();
    if (resObjects.size() != 1) {
        setError("Invalid number of objects in result!");
        return ReportResult_Finished;
    }
    MAlignmentObject* obj = qobject_cast<MAlignmentObject*>(resObjects.first());
    if (obj == NULL) {
        setError("Failed to load result alignment");
        return ReportResult_Finished;
    }
    const MAlignment& aln = obj->getMAlignment();
    const_cast<MAlignment&>(aln).sortRowsByName();
    const_cast<MAlignment&>(expectedObj->getMAlignment()).sortRowsByName();
    
    if (aln != expectedObj->getMAlignment()) {
        setError("Expected and result alignments don't match");
    }


    return ReportResult_Finished;
}

void GTest_DnaAssemblyToReferenceTask::cleanup()
{
    // cleanup temporary files

    QDir dir(env->getVar("TEMP_DATA_DIR"));
    dir.remove(resultFileName.getURLString());
    QDir::root().rmpath(env->getVar("TEMP_DATA_DIR"));
}

///////////////////////////////////////////////////////////////////////////////////////////

QList<XMLTestFactory*> DnaAssemblyTests::createTestFactories()
{
    QList<XMLTestFactory*> res;
    res.append(GTest_DnaAssemblyToReferenceTask::createFactory());
    
    return res;
}

} // U2
