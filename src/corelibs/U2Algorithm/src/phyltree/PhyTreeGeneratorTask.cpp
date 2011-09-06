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

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>

#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>

namespace U2 {

PhyTreeGeneratorTask::PhyTreeGeneratorTask(const MAlignment& ma, const CreatePhyTreeSettings& _settings) 
   : Task(tr("Calculating Phylogenetic Tree"), TaskFlag_FailOnSubtaskError), inputMA(ma), settings(_settings)
{
    tpm = Task::Progress_Manual;
}

void PhyTreeGeneratorTask::run() {
}

Task::ReportResult PhyTreeGeneratorTask::report() {
    return ReportResult_Finished; 
}

PhyTreeGeneratorLauncherTask::PhyTreeGeneratorLauncherTask(const MAlignment& ma, const CreatePhyTreeSettings& _settings)
:Task(tr("Calculating Phylogenetic Tree"), TaskFlag_FailOnSubtaskError), inputMA(ma), settings(_settings), task(NULL){
    tpm = Task::Progress_SubTasksBased;
}
void PhyTreeGeneratorLauncherTask::prepare(){
    QString algId = settings.algorithmId;
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    PhyTreeGenerator* generator = registry->getGenerator(algId);
    assert(generator!=NULL);
    if (generator == NULL) {
        stateInfo.setError(tr("Tree construction algorithm %1 not found").arg(algId));
    }else{
        task = dynamic_cast<PhyTreeGeneratorTask*>(generator->createCalculatePhyTreeTask(inputMA,settings));
        addSubTask(task);
    }
    
}
Task::ReportResult PhyTreeGeneratorLauncherTask::report(){
    if(task){
        result = task->getResult();
    }
    return ReportResult_Finished;
}

} //namespace
