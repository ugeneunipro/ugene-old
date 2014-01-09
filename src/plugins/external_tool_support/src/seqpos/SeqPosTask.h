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

#ifndef _U2_SEQPOS_SUPPORT_TASK_
#define _U2_SEQPOS_SUPPORT_TASK_

#include <U2Core/AnnotationData.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/Task.h>

#include "SeqPosSettings.h"

namespace U2 {

class Document;
class SaveDocumentTask;
class LoadDocumentTask;

class SeqPosTask : public ExternalToolSupportTask {
    Q_OBJECT
public:
    SeqPosTask(const SeqPosSettings& _settings, const QList<AnnotationData>& _treatAnn);
    virtual ~SeqPosTask();

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    virtual void run();
    virtual void cleanup();

    const SeqPosSettings & getSettings();
    QStringList getOutputFiles();

private:
    SeqPosSettings settings;
    QString workingDir;

    QList<AnnotationData> treatAnn;

    Document *treatDoc;

    SaveDocumentTask *treatTask;

    ExternalToolRunTask *etTask;
    ExternalToolLogParser *logParser;

    static const QString BASE_DIR_NAME;
    static const QString BASE_SUBDIR_NAME;
    static const QString TREAT_NAME;

private:
    Document* createDoc(const QList<AnnotationData>& annData, const QString& name);
};

class SeqPosLogParser : public ExternalToolLogParser {
public:
    SeqPosLogParser();

private:
    QString lastErrLine;
    int     progress;
};

} // U2

#endif // _U2_SEQPOS_SUPPORT_TASK_
