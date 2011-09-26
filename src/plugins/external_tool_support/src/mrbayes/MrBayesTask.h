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

#ifndef _U2_MRBAYES_TASK_H
#define _U2_MRBAYES_TASK_H

#include "MrBayesSupport.h"
#include "ExternalToolRunTask.h"
#include "utils/ExportTasks.h"

#include <U2View/CreatePhyTreeDialogController.h>
#include <U2Algorithm/PhyTreeGenerator.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/PhyTreeObject.h>


namespace U2 {

class MrBayesPrepareDataForCalculation :public Task{
public:
    MrBayesPrepareDataForCalculation(const MAlignment& _ma, const CreatePhyTreeSettings& s, const QString& url);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
    QString getInputFileUrl() {return inputFileForMrBayes;}
private:
    const MAlignment&           ma;
    CreatePhyTreeSettings       settings;
    QString                     tmpDirUrl;
    SaveAlignmentTask*          saveDocumentTask;
    QString                     inputFileForMrBayes;
};

class MrBayesLogParser : public ExternalToolLogParser {
public:
    MrBayesLogParser(int _nchains);
    int getProgress();
    void parseErrOutput(const QString& partOfLog);
private:
    QString lastErrLine; 
    int nchains;
    bool isMCMCRunning;
    int curProgress;

};

class MrBayesGetCalculatedTreeTask: public Task{
public:
    MrBayesGetCalculatedTreeTask(const QString& url);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
    PhyTreeObject* getPhyObject(){return phyObject;}
private:
    QString                     baseFileName;
    LoadDocumentTask*           loadTmpDocumentTask;
    PhyTreeObject*              phyObject;

};

class MrBayesSupportTask : public PhyTreeGeneratorTask{
public:
    MrBayesSupportTask(const MAlignment& _ma, const CreatePhyTreeSettings& s);
    void prepare();
    Task::ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QString                             tmpDirUrl;
    QString                             tmpNexusFile;
    MrBayesPrepareDataForCalculation*   prepareDataTask;
    ExternalToolRunTask*                mrBayesTask;
    MrBayesGetCalculatedTreeTask*       getTreeTask;
    MrBayesLogParser*                   logParser;

};



}//namespace

#endif // _U2_MRBAYES_TASK_H
