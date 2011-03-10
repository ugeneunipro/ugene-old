#include "BlastTaskSettings.h"

namespace U2 {

BlastTaskSettings::BlastTaskSettings() {
    offsInGlobalSeq=0;
    aobj=NULL;
    alphabet=NULL;
    isNucleotideSeq=false;
    numberOfHits=0;
    isDefaultMatrix=true;
    isDefautScores=true;
    isDefaultCosts=true;
    reset();
}

void BlastTaskSettings::reset() {
    programName="";
    databaseNameAndPath="";
    expectValue=10.0;
    megablast=false;
    wordSize=11;
    gapOpenCost=0;
    gapExtendCost=0;
    mismatchPenalty=0;
    matchReward=0;
    matrix="";
    queryFile="";
    outputResFile="";
    numberOfProcessors=1;
    needCreateAnnotations=true;
    filter="";
}

}//namespace
