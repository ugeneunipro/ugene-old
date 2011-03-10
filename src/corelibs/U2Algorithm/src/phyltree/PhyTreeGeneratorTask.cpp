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
