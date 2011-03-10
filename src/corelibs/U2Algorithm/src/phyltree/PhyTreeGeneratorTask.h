#ifndef  __PHYTREEGENERATORTASK_H
#define  __PHYTREEGENERATORTASK_H

#include <U2Core/Task.h>
#include "PhyTreeGenerator.h"
#include <U2Algorithm/CreatePhyTreeSettings.h>

namespace U2{

class U2ALGORITHM_EXPORT PhyTreeGeneratorTask: public Task{
    Q_OBJECT
public:
    PhyTreeGeneratorTask(const MAlignment& ma, const CreatePhyTreeSettings& _settings);
    ~PhyTreeGeneratorTask(){};
    void run();
    PhyTree getResult() { return result; }
    const CreatePhyTreeSettings& getSettings() { return settings; } 
    ReportResult report();
private:
    void calculateTree();
    PhyTreeGenerator*           generator;
    const MAlignment&           inputMA;
    PhyTree                     result;
    CreatePhyTreeSettings       settings;

};

} //namespace

#endif
