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

#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>

namespace U2 {

PhyTreeGeneratorTask::PhyTreeGeneratorTask(const MAlignment& ma, const CreatePhyTreeSettings& _settings) 
   : Task(tr("Calculating Phylogenetic Tree"), TaskFlags_FOSCOE), inputMA(ma), settings(_settings)
{
    tpm = Task::Progress_Manual;
    TaskResourceUsage tru(RESOURCE_PHYTREE, 1, true);
    taskResources.append(tru);
    QString algId = settings.algorithmId;
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    generator = registry->getGenerator(algId);
    if (generator == NULL) {
        stateInfo.setError(QString("Tree construction algorithm %1 not found").arg(algId));
    }
}

void PhyTreeGeneratorTask::run() {
    assert(!hasErrors());
    if (!hasErrors()) {
        calculateTree();
    }
}

void PhyTreeGeneratorTask::calculateTree() {
    stateInfo.progress = 0;
    stateInfo.setStateDesc(tr("Calculating phylogenetic tree"));
    result = generator->calculatePhyTree(inputMA, settings, stateInfo);
    stateInfo.progress = 100;
}

Task::ReportResult PhyTreeGeneratorTask::report() {
    return ReportResult_Finished; 
}

} //namespace
