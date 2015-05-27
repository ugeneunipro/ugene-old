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

#include <QAction>
#include <QDir>
#include <QMessageBox>

#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Algorithm/DnaAssemblyMultiTask.h>
#include <U2Algorithm/GenomeAssemblyMultiTask.h>
#include <U2Algorithm/GenomeAssemblyRegistry.h>

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/MultiTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/ConvertAssemblyToSamTask.h>
#include <U2Formats/ConvertFileTask.h>

#include <U2Gui/OpenViewTask.h>
#include <U2Gui/ToolsMenu.h>
#include <U2Gui/QObjectScopedPointer.h>

#include "BuildIndexDialog.h"
#include "ConvertAssemblyToSamDialog.h"
#include "DnaAssemblyDialog.h"
#include "DnaAssemblyUtils.h"
#include "GenomeAssemblyDialog.h"

namespace U2 {

DnaAssemblySupport::DnaAssemblySupport()
{
    QAction* convertAssemblyToSamAction = new QAction( tr("Convert UGENE assembly database to SAM..."), this );
    convertAssemblyToSamAction->setObjectName(ToolsMenu::NGS_CONVERT_SAM);
    convertAssemblyToSamAction->setIcon(QIcon(":core/images/align.png"));
    connect( convertAssemblyToSamAction, SIGNAL( triggered() ), SLOT( sl_showConvertToSamDialog() ) );
    ToolsMenu::addAction(ToolsMenu::NGS_MENU, convertAssemblyToSamAction);

    QAction* genomeAssemblyAction = new QAction( tr("Genome de novo assembly..."), this );
    genomeAssemblyAction->setObjectName(ToolsMenu::NGS_DENOVO);
    genomeAssemblyAction->setIcon(QIcon(":core/images/align.png"));
    connect( genomeAssemblyAction, SIGNAL( triggered() ), SLOT( sl_showGenomeAssemblyDialog() ) );
    ToolsMenu::addAction(ToolsMenu::NGS_MENU, genomeAssemblyAction);

    QAction* dnaAssemblyAction = new QAction(tr("Map reads to reference..."), this );
    dnaAssemblyAction->setObjectName(ToolsMenu::NGS_MAP);
    dnaAssemblyAction->setIcon(QIcon(":core/images/align.png"));
    connect( dnaAssemblyAction, SIGNAL( triggered() ), SLOT( sl_showDnaAssemblyDialog() ) );
    ToolsMenu::addAction(ToolsMenu::NGS_MENU, dnaAssemblyAction);

    QAction* buildIndexAction = new QAction( tr("Build index for reads mapping..."), this );
    buildIndexAction->setObjectName(ToolsMenu::NGS_INDEX);
    buildIndexAction->setIcon(QIcon(":core/images/align.png"));
    connect( buildIndexAction, SIGNAL( triggered() ), SLOT( sl_showBuildIndexDialog() ) );
    ToolsMenu::addAction(ToolsMenu::NGS_MENU, buildIndexAction);
}

void DnaAssemblySupport::sl_showDnaAssemblyDialog()
{
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    if (registry->getRegisteredAlgorithmIds().isEmpty()) {
        QMessageBox::information(QApplication::activeWindow(), tr("DNA Assembly"),
            tr("There are no algorithms for DNA assembly available.\nPlease, check your plugin list.") );
        return;
    }

    QObjectScopedPointer<DnaAssemblyDialog> dlg = new DnaAssemblyDialog(QApplication::activeWindow());
    dlg->exec();
    CHECK(!dlg.isNull(), );

    if (QDialog::Accepted == dlg->result()) {
        DnaAssemblyToRefTaskSettings s;
        s.samOutput = dlg->isSamOutput();
        s.refSeqUrl = dlg->getRefSeqUrl();
        s.algName = dlg->getAlgorithmName();
        s.resultFileName = dlg->getResultFileName();
        s.setCustomSettings(dlg->getCustomSettings());
        s.shortReadSets = dlg->getShortReadSets();
        s.pairedReads = dlg->isPaired();
        s.openView = true;
        s.prebuiltIndex = dlg->isPrebuiltIndex();
        Task* assemblyTask = new DnaAssemblyTaskWithConversions(s, true);
        AppContext::getTaskScheduler()->registerTopLevelTask(assemblyTask);
    }
}

void DnaAssemblySupport::sl_showGenomeAssemblyDialog() {
    GenomeAssemblyAlgRegistry* registry = AppContext::getGenomeAssemblyAlgRegistry();
    if (registry->getRegisteredAlgorithmIds().isEmpty()) {
        QMessageBox::information(QApplication::activeWindow(), tr("Genome Assembly"),
            tr("There are no algorithms for genome assembly available.\nPlease, check external tools in the settings.") );
        return;
    }

    QObjectScopedPointer<GenomeAssemblyDialog> dlg = new GenomeAssemblyDialog(QApplication::activeWindow());
    dlg->exec();
    CHECK(!dlg.isNull(), );

    if (QDialog::Accepted == dlg->result()) {
        GenomeAssemblyTaskSettings s;
        s.algName = dlg->getAlgorithmName();
        s.outDir = dlg->getOutDir();
        s.setCustomSettings(dlg->getCustomSettings());
        s.reads = dlg->getReads();
        s.openView = true;
        Task* assemblyTask = new GenomeAssemblyMultiTask(s);
        AppContext::getTaskScheduler()->registerTopLevelTask(assemblyTask);
    }
}

void DnaAssemblySupport::sl_showBuildIndexDialog()
{
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    if (registry->getRegisteredAlgorithmIds().isEmpty()) {
        QMessageBox::information(QApplication::activeWindow(), tr("DNA Assembly"),
            tr("There are no algorithms for DNA assembly available.\nPlease, check your plugin list.") );
        return;
    }

    QObjectScopedPointer<BuildIndexDialog> dlg = new BuildIndexDialog(registry, QApplication::activeWindow());
    dlg->exec();
    CHECK(!dlg.isNull(), );

    if (QDialog::Accepted == dlg->result()) {
        DnaAssemblyToRefTaskSettings s;
        s.refSeqUrl = dlg->getRefSeqUrl();
        s.algName = dlg->getAlgorithmName();
        s.resultFileName = dlg->getIndexFileName();
        s.indexFileName = dlg->getIndexFileName();
        s.setCustomSettings(dlg->getCustomSettings());
        s.openView = false;
        s.prebuiltIndex = false;
        s.pairedReads = false;
        Task* assemblyTask = new DnaAssemblyTaskWithConversions(s, false, true);
        AppContext::getTaskScheduler()->registerTopLevelTask(assemblyTask);
    }
}

void DnaAssemblySupport::sl_showConvertToSamDialog() {
    QObjectScopedPointer<ConvertAssemblyToSamDialog> dlg = new ConvertAssemblyToSamDialog(QApplication::activeWindow());
    dlg->exec();
    CHECK(!dlg.isNull(), );

    if (QDialog::Accepted == dlg->result()) {
        Task *convertTask = new ConvertAssemblyToSamTask(dlg->getDbFileUrl(), dlg->getSamFileUrl());
        AppContext::getTaskScheduler()->registerTopLevelTask(convertTask);
    }
}

namespace {
enum Result {
    UNKNOWN,
    CORRECT,
    INCORRECT
};

static Result isCorrectFormat(const GUrl &url, const QStringList &targetFormats, QString &detectedFormat) {
    DocumentUtils::Detection r = DocumentUtils::detectFormat(url, detectedFormat);
    CHECK(DocumentUtils::UNKNOWN != r, UNKNOWN);

    bool correct = targetFormats.contains(detectedFormat);
    if (correct) {
        return CORRECT;
    }
    return INCORRECT;
}

ConvertFileTask * getConvertTask(const GUrl &url, const QStringList &targetFormats) {
    QString detectedFormat;
    Result r = isCorrectFormat(url, targetFormats, detectedFormat);
    if (UNKNOWN == r) {
        coreLog.info("Unknown file format: " + url.getURLString());
        return NULL;
    }

    if (INCORRECT == r) {
        QDir dir = QFileInfo(url.getURLString()).absoluteDir();
        return new DefaultConvertFileTask(url, detectedFormat, targetFormats.first(), dir.absolutePath());
    }
    return NULL;
}
}

#define CHECK_FILE(url, targetFormats) \
    QString format; \
    Result r = isCorrectFormat(url, targetFormats, format); \
    if (UNKNOWN == r) { \
        unknownFormatFiles << url; \
    } else if (INCORRECT == r) { \
        result[url.getURLString()] = format; \
    }

#define PREPARE_FILE(url, targetFormats) \
    if (!toConvert.contains(url.getURLString())) { \
        ConvertFileTask *task = getConvertTask(url, targetFormats); \
        if (NULL != task) { \
            addSubTask(task); \
            conversionTasksCount++; \
            toConvert << url.getURLString(); \
        } \
    }

QMap<QString, QString> DnaAssemblySupport::toConvert(const DnaAssemblyToRefTaskSettings &settings, QList<GUrl> &unknownFormatFiles) {
    QMap<QString, QString> result;
    DnaAssemblyAlgorithmEnv *env= AppContext::getDnaAssemblyAlgRegistry()->getAlgorithm(settings.algName);
    SAFE_POINT(NULL != env, "Unknown algorithm: " + settings.algName, result);

    foreach (const GUrl &url, settings.getShortReadUrls()) {
        CHECK_FILE(url, env->getReadsFormats());
    }

    if (!settings.prebuiltIndex) {
        CHECK_FILE(settings.refSeqUrl, env->getRefrerenceFormats());
    }
    return result;
}

QString DnaAssemblySupport::toConvertText(const QMap<QString, QString> &files) {
    QStringList strings;
    foreach (const QString &url, files.keys()) {
        QString format = files[url];
        strings << url + " [" + format + "]";
    }
    return strings.join("\n");
}

QString DnaAssemblySupport::unknownText(const QList<GUrl> &unknownFormatFiles) {
    QStringList strings;
    foreach (const GUrl &url, unknownFormatFiles) {
        strings << url.getURLString();
    }
    return strings.join("\n");
}

/************************************************************************/
/* DnaAssemblyTaskWithConversions */
/************************************************************************/
DnaAssemblyTaskWithConversions::DnaAssemblyTaskWithConversions(const DnaAssemblyToRefTaskSettings &settings, bool viewResult, bool justBuildIndex)
: Task("Dna assembly task", TaskFlags_NR_FOSCOE), settings(settings), viewResult(viewResult),
    justBuildIndex(justBuildIndex), conversionTasksCount(0), assemblyTask(NULL)
{

}

void DnaAssemblyTaskWithConversions::prepare() {
    DnaAssemblyAlgorithmEnv *env= AppContext::getDnaAssemblyAlgRegistry()->getAlgorithm(settings.algName);
    if (env == NULL) {
        setError(QString("Algorithm %1 is not found").arg(settings.algName));
        return;
    }

    QSet<QString> toConvert;
    Q_UNUSED(toConvert);
    foreach (const GUrl &url, settings.getShortReadUrls()) {
        PREPARE_FILE(url, env->getReadsFormats());
    }

    if (!settings.prebuiltIndex) {
        PREPARE_FILE(settings.refSeqUrl, env->getRefrerenceFormats());
    }

    if (0 == conversionTasksCount) {
        assemblyTask = new DnaAssemblyMultiTask(settings, viewResult, justBuildIndex);
        addSubTask(assemblyTask);
    }
}

QList<Task*> DnaAssemblyTaskWithConversions::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(!subTask->hasError(), result);
    CHECK(!hasError(), result);

    ConvertFileTask *convertTask = dynamic_cast<ConvertFileTask*>(subTask);
    if (NULL != convertTask) {
        SAFE_POINT_EXT(conversionTasksCount > 0, setError("Conversions task count error"), result);
        if (convertTask->getSourceURL() == settings.refSeqUrl) {
            settings.refSeqUrl = convertTask->getResult();
        }

        for (QList<ShortReadSet>::Iterator i=settings.shortReadSets.begin(); i != settings.shortReadSets.end(); i++) {
            if (convertTask->getSourceURL() == i->url) {
                i->url = convertTask->getResult();
            }
        }
        conversionTasksCount--;

        if (0 == conversionTasksCount) {
            assemblyTask = new DnaAssemblyMultiTask(settings, viewResult, justBuildIndex);
            result << assemblyTask;
        }
    }

    return result;
}

} // U2

