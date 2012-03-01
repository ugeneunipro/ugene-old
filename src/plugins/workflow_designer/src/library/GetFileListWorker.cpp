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

#include <U2Core/AppContext.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Designer/DelegateEditors.h>
#include <U2Gui/GUIUtils.h>

#include "GetFileListWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString GetFileListWorkerFactory::ACTOR_ID("get-file-list");
static const QString OUT_PORT_ID("out-url");
static const QString INPUT_PATH("in-path");
static const QString ABSOLUTE_PATH("absolute");
static const QString RECURSIVE("recursive");
static const QString INCLUDE_NAME_FILTER("include-name-filter");
static const QString EXCLUDE_NAME_FILTER("exclude-name-filter");


/************************************************************************/
/* Worker */
/************************************************************************/
GetFileListWorker::GetFileListWorker(Actor *p)
: BaseWorker(p), outChannel(NULL)
{
    absolute = false;
    recursive = false;
}

void GetFileListWorker::init() {
    outChannel = ports.value(OUT_PORT_ID);
    mtype = outChannel->getBusType();

    dirUrls = WorkflowUtils::expandToUrls(actor->getParameter(INPUT_PATH)->getAttributeValue<QString>(context));
    absolute = actor->getParameter(ABSOLUTE_PATH)->getAttributeValue<bool>(context);
    recursive = actor->getParameter(RECURSIVE)->getAttributeValue<bool>(context);
    includeFilter = actor->getParameter(INCLUDE_NAME_FILTER)->getAttributeValue<QString>(context);
    excludeFilter = actor->getParameter(EXCLUDE_NAME_FILTER)->getAttributeValue<QString>(context);
}

Task *GetFileListWorker::tick() {
    if (cache.isEmpty() && !dirUrls.isEmpty()) {
        Task *t = new ScanDirectoryTask(dirUrls.takeFirst(), includeFilter, excludeFilter, absolute, recursive);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }
    while (!cache.isEmpty()) {
        outChannel->put(cache.takeFirst());
    }
    if (dirUrls.isEmpty()) {
        setDone();
        outChannel->setEnded();
    }
    return NULL;
}

bool GetFileListWorker::isDone() {
    return BaseWorker::isDone() && cache.isEmpty();
}

void GetFileListWorker::sl_taskFinished() {
    ScanDirectoryTask *t = qobject_cast<ScanDirectoryTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }
    foreach(const QString &url, t->getResults()) {
        QVariantMap m;
        m.insert(BaseSlots::URL_SLOT().getId(), url);
        cache.append(Message(mtype, m));
    }
}

/************************************************************************/
/* Task */
/************************************************************************/
ScanDirectoryTask::ScanDirectoryTask(const QString &dirPath, const QString &incFilter, const QString &excFilter, bool absolute, bool recursive)
: Task(tr("Scan directory %1").arg(dirPath), TaskFlag_None), dirPath(dirPath), includeFilter(incFilter),
excludeFilter(excFilter), absolute(absolute), recursive(recursive)
{

}

void ScanDirectoryTask::run() {
    QDir rootDir(dirPath);
    QStringList usedDirs;
    QFileInfoList unusedDirs; // it is needed for avoiding cycles because of symlinks
    unusedDirs << QFileInfo(dirPath);

    QString rootPath = rootDir.absolutePath();
    if (!rootPath.endsWith("/")) {
        rootPath += "/";
    }

    QRegExp incRx(includeFilter);
    QRegExp excRx(excludeFilter);
    incRx.setPatternSyntax(QRegExp::Wildcard);
    excRx.setPatternSyntax(QRegExp::Wildcard);


    while (!unusedDirs.isEmpty()) {
        QFileInfo entry = unusedDirs.takeFirst();
        if (usedDirs.contains(entry.absoluteFilePath())) {
            continue;
        }

        QDir dir(entry.absoluteFilePath());
        QFileInfoList nested;
        QFileInfoList files = scanDirectory(dir, nested);
        foreach (const QFileInfo &path, files) {
            bool matched = true;
            if (!includeFilter.isEmpty()) {
                matched = incRx.exactMatch(path.fileName());
            }
            if (!excludeFilter.isEmpty()) {
                matched = matched && !excRx.exactMatch(path.fileName());
            }
            if (matched) {
                QString absPath = path.absoluteFilePath();
                if (absolute) {
                    results << absPath;
                } else {
                    QString relPath = absPath.replace(rootPath, "");
                    results << relPath;
                }
            }
        }
        if (recursive) {
            unusedDirs << nested;
        }
        usedDirs << dir.absolutePath();
    }
}

QFileInfoList ScanDirectoryTask::scanDirectory(const QDir &dir, QFileInfoList &nestedDirs) {
    QFileInfoList result;
    if (!dir.exists()) {
        return result;
    }

    QFileInfoList entries = dir.entryInfoList();
    foreach (const QFileInfo &entry, entries) {
        if (entry.isDir()) {
            if ("." == entry.fileName() || ".." == entry.fileName()) {
                continue;
            }
            if (entry.isSymLink()) {
                nestedDirs << QFileInfo(entry.symLinkTarget());
            } else {
                nestedDirs << entry;
            }
        } else if (entry.isFile()) {
            result << entry;
        }
    }

    return result;
}

/************************************************************************/
/* Factory */
/************************************************************************/
void GetFileListWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> outTypeMap;
        outTypeMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr outTypeSet(new MapDataType(BasePorts::OUT_TEXT_PORT_ID(), outTypeMap));

        portDescs << new PortDescriptor(OUT_PORT_ID, outTypeSet, false);
    }

    QList<Attribute*> attrs;
    {
        Descriptor inPath(INPUT_PATH,
            GetFileListWorker::tr("Input directory"),
            GetFileListWorker::tr("Input directory"));
        Descriptor isAbsolute(ABSOLUTE_PATH,
            GetFileListWorker::tr("Absolute output paths"),
            GetFileListWorker::tr("Produce absolute or relative paths"));
        Descriptor isRecursive(RECURSIVE,
            GetFileListWorker::tr("Recursive reading"),
            GetFileListWorker::tr("Get files from all nested directories or just from the current one"));
        Descriptor includeFilter(INCLUDE_NAME_FILTER,
            GetFileListWorker::tr("Include name filter"),
            GetFileListWorker::tr("Filter files by name using this name or this regular expression filter"));
        Descriptor excludeFilter(EXCLUDE_NAME_FILTER,
            GetFileListWorker::tr("Exclude name filter"),
            GetFileListWorker::tr("Exclude files with this name or with this regular expression"));

        attrs << new Attribute(inPath, BaseTypes::STRING_TYPE(), true);
        attrs << new Attribute(isAbsolute, BaseTypes::BOOL_TYPE(), false, true);
        attrs << new Attribute(isRecursive, BaseTypes::BOOL_TYPE(), false, false);
        attrs << new Attribute(includeFilter, BaseTypes::STRING_TYPE());
        attrs << new Attribute(excludeFilter, BaseTypes::STRING_TYPE());
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[INPUT_PATH] = new URLDelegate(QString(), QString(), true, true);
    }

    Descriptor protoDesc(GetFileListWorkerFactory::ACTOR_ID,
        GetFileListWorker::tr("File List"),
        GetFileListWorker::tr("Produces ulrs to files from specified directories"));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new GetFileListPrompter());
    if(AppContext::isGUIMode()) {
        proto->setIcon( GUIUtils::createRoundIcon(QColor(85,85,255), 22));
    }

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new GetFileListWorkerFactory());
}

Worker *GetFileListWorkerFactory::createWorker(Actor *a) {
    return new GetFileListWorker(a);
}

/************************************************************************/
/* Prompter */
/************************************************************************/
QString GetFileListPrompter::composeRichDoc() {
    bool absolute = getParameter(ABSOLUTE_PATH).toBool();
    QString relativity;
    if (absolute) {
        relativity = tr("<u>absolute</u>");
    } else {
        relativity = tr("<u>relative</u>");
    }

    bool recursive = getParameter(RECURSIVE).toBool();
    QString recursivity;
    if (recursive) {
        recursivity = tr(" <u>recursively</u> ");
    } else {
        recursivity = tr(" ");
    }

    QString url = getHyperlink(INPUT_PATH, getURL(INPUT_PATH));

    return tr("Gets%1the %2 paths of files from the <u>%3</u> directory")
        .arg(recursivity).arg(relativity).arg(url);
}

} // LocalWorkflow
} // U2
