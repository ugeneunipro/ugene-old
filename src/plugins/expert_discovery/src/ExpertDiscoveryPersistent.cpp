#include "ExpertDiscoveryPersistent.h"


namespace U2 {

void EDPMSequence::save(QDataStream& ar, Sequence& rSequence)
{
    ar << QString::fromStdString(rSequence.getName());
    ar << QString::fromStdString(rSequence.getSequence());
    ar << rSequence.isHasScore();
    ar << rSequence.getScore();
}

void EDPMSequence::load(QDataStream& ar, Sequence& rSequence){
    QString strName;
    QString strSequence;
    bool bHasScore = false;
    double dScore = 0;
    ar >> strName;
    ar >> strSequence;
    ar >> bHasScore;
    ar >> dScore;

    rSequence.setName(strName.toStdString());
    rSequence.setSequence(strSequence.toStdString());
    rSequence.setHasScore(bHasScore);
    rSequence.setScore(dScore);
}

void EDPMSeqBase::save(QDataStream& ar, SequenceBase& rSeqBase)
{
    int nSize = rSeqBase.getSize();
    ar << nSize;
    for (int i=0; i<nSize; i++) 
        EDPMSequence::save(ar, rSeqBase.getSequence(i));
}

void EDPMSeqBase::load(QDataStream& ar, SequenceBase& rSeqBase){
    rSeqBase.clear();
    int nSize = 0;
    ar >> nSize;
    assert( nSize >= 0 );
    Sequence seq;
    for (int i=0; i<nSize; i++) {
        EDPMSequence::load(ar, seq);
        rSeqBase.addSequence(seq);
    }
}


void EDPMMrkSignal::save(QDataStream& ar, Marking::IntervalSet& rMrk)
{
    ar << rMrk.size();
    Marking::IntervalSet::iterator i = rMrk.begin();
    while (i!=rMrk.end()) {
        ar << (*i).getFrom();
        ar << (*i).getTo();
        i++;
    }
}

void EDPMMrkSignal::load(QDataStream& ar, Marking::IntervalSet& rMrk){
    int nSize = 0;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        int nFrom = 0;
        ar >> nFrom;
        int nTo = 0;
        ar >> nTo;
        rMrk.insert(DDisc::Interval(nFrom, nTo));
    }	
}



void EDPMMrkFamily::save(QDataStream& ar, Marking::FamilyMarking& rMrk)
{
    ar << rMrk.size();
    Marking::FamilyMarking::iterator i = rMrk.begin();
    while (i!=rMrk.end()) {
        ar << QString::fromStdString(i->first);
        EDPMMrkSignal::save(ar, i->second);
        i++;
    }
}

void EDPMMrkFamily::load(QDataStream& ar, Marking::FamilyMarking& rMrk){
    int nSize = 0;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        QString strSignalName;
        Marking::IntervalSet SignalMrk;
        ar >> strSignalName;
        EDPMMrkSignal::load(ar, SignalMrk);
        rMrk[strSignalName.toStdString()] = SignalMrk;
    }
}

void EDPMMrk::save(QDataStream& ar, Marking& rMrk)
{
    Marking::MarkingData& rData = rMrk.marking;
   
    ar << rData.size();
    Marking::MarkingData::iterator i = rData.begin();
    while (i!=rData.end()) {
        ar << QString::fromStdString(i->first);
        EDPMMrkFamily::save(ar, i->second);
        i++;
    }
}

void EDPMMrk::load(QDataStream& ar, Marking& rMrk){
    Marking::MarkingData& rData = rMrk.marking;
    int nSize = 0;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        QString strFamilyName;
        Marking::FamilyMarking FamilyMrk;
        ar >> strFamilyName;
        EDPMMrkFamily::load(ar, FamilyMrk);
        rData[strFamilyName.toStdString()] = FamilyMrk;
    }
}

void EDPMMrkBase::save(QDataStream& ar, MarkingBase& rMrkBase, int nSeqCount)
{
    ar << nSeqCount;
    for (int i=0; i<nSeqCount; i++) {
        try {
            EDPMMrk::save(ar, rMrkBase.getMarking(i));
        }
        catch (range_error) {
            EDPMMrk::save(ar, Marking());
        }
    }
}

void EDPMMrkBase::load(QDataStream& ar, MarkingBase& rMrkBase, int nSeqCount){
    rMrkBase.clear();
    int nSize = 0;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        Marking mrk;
        EDPMMrk::load(ar, mrk);		
        rMrkBase.setMarking(i, mrk);
    }
}

void EDPMDescInfo::save(QDataStream& ar, MetaInfo& rInfo)
{
    ar << rInfo.getNo();
    ar << QString::fromStdString(rInfo.getName());
    ar << QString::fromStdString(rInfo.getMethodName());
 
}

void EDPMDescInfo::load(QDataStream& ar, MetaInfo& rInfo){
    int nNo = 0;
    ar >> nNo;
    QString strName;
    QString strMethodName;
    ar >> strName;
    ar >> strMethodName;
    rInfo.setNo(nNo);
    rInfo.setName(strName.toStdString());
    rInfo.setMethodName(strMethodName.toStdString());
}

void EDPMDescFamily::save(QDataStream& ar, Family& rFamily)
{
    
    ar << QString::fromStdString(rFamily.getName());
    int nSize = rFamily.getSignalNumber();
    ar << nSize;
    for (int i=0; i<nSize; i++)
        EDPMDescInfo::save(ar, rFamily.getMetaInfo(i));
}

void EDPMDescFamily::load(QDataStream& ar, Family& rFamily){
    QString strName;
    ar >> strName;
    rFamily.setName(strName.toStdString());
    int nSize = 0;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        MetaInfo info;
        EDPMDescInfo::load(ar, info);
        rFamily.AddInfo(info);
    }
}

void EDPMDescription::save(QDataStream& ar, MetaInfoBase& rDesc)
{
    int nSize = rDesc.getFamilyNumber();
    ar << nSize;
    for (int i=0; i<nSize; i++)
        EDPMDescFamily::save(ar, rDesc.getSignalFamily(i));
}

void EDPMDescription::load(QDataStream& ar, MetaInfoBase& rDesc){
    rDesc.clear();
    int nSize = 0;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        Family family;
        EDPMDescFamily::load(ar, family);
        rDesc.addFamily(family);
    }
}


void EDPMCS::save(QDataStream& ar, Signal* pSignal)
{
    ar << QString::fromStdString(pSignal->getName());
    ar << QString::fromStdString(pSignal->getDescription());
    ar << bool(pSignal->isPriorParamsDefined());
    if (pSignal->isPriorParamsDefined()) {
        ar << pSignal->getPriorProbability();
        ar << pSignal->getPriorFisher();
        ar << pSignal->getPriorPosCoverage();
        ar << pSignal->getPriorNegCoverage();
    }
    Operation *pOp = pSignal->getSignal();
    EDPMOperation::save(ar, pOp);
  
}

void EDPMCS::load(QDataStream& ar, Signal* pSignal){
    QString strName;
    QString strDescription;
    bool bPriorParamsDefined;
    double dPriorProb = 0;
    double dPriorFisher = 1;
    double dPriorPosCoverage = 0;
    double dPriorNegCoverage = 0;

    ar >> strName;
    ar >> strDescription;
    ar >> bPriorParamsDefined;

    if (bPriorParamsDefined) {
        ar >> dPriorProb;
        ar >> dPriorFisher;
        ar >> dPriorPosCoverage;
        ar >> dPriorNegCoverage;
    }

    pSignal->setName(strName.toStdString());
    pSignal->setDescription(strDescription.toStdString());
    Operation *pOp = 0;
    EDPMOperation::load(ar, pOp);

    pSignal->attach(pOp);
    pSignal->setPriorParamsDefined(bPriorParamsDefined != 0);
    if (pSignal->isPriorParamsDefined()) {
        pSignal->setPriorProbability(dPriorProb);
        pSignal->setPriorFisher(dPriorFisher);
        pSignal->setPriorPosCoverage(dPriorPosCoverage);
        pSignal->setPriorNegCoverage(dPriorNegCoverage);
    }
}

void EDPMCSFolder::save(QDataStream& ar, CSFolder* pFolder)
{
    ar << pFolder->getName();
    
    int nFolderNumber = pFolder->getFolderNumber();
    ar << nFolderNumber;
    for (int i=0; i<nFolderNumber; i++)
        EDPMCSFolder::save(ar, pFolder->getSubfolder(i));
    int nSignalNumber = pFolder->getSignalNumber();
    ar << nSignalNumber;
    for (int i=0; i<nSignalNumber; i++)
        EDPMCS::save(ar, pFolder->getSignal(i));
 
}

void EDPMCSFolder::load(QDataStream& ar, CSFolder* pFolder){
    QString strName;
    ar >> strName;
    pFolder->setName(strName);
    int nFolderNumber = 0;
    ar >> nFolderNumber;
    for (int i=0; i<nFolderNumber; i++) {
        CSFolder *pSubfolder = new CSFolder;
        EDPMCSFolder::load(ar, pSubfolder);
        pFolder->addFolder(pSubfolder);
    }
    int nSignalNumber = 0;
    ar >> nSignalNumber;
    for (int i=0; i<nSignalNumber; i++) {
        Signal *pSignal = new Signal;
        EDPMCS::load(ar, pSignal);
        pFolder->addSignal(pSignal);
    }
}

void EDPMOperation::save(QDataStream& ar, Operation*& pOp)
{
    EOpType eType;
    if (pOp) eType = pOp->getType();
    else eType = OP_UNDEFINED;
    ar << (int)eType;
}

void EDPMOperation::saveDistance(QDataStream& ar, OpDistance* pOp)
{
    ar << pOp->getDistance().getFrom();
    ar << pOp->getDistance().getTo();
    ar << pOp->getDistanceType();
    ar << pOp->isOrderImportant();

}

void EDPMOperation::saveReiteration(QDataStream& ar, OpReiteration* pOp)
{
    ar << pOp->getDistance().getFrom();
    ar << pOp->getDistance().getTo();
    ar << pOp->getCount().getFrom();
    ar << pOp->getCount().getTo();
    ar << pOp->getDistanceType();
}

void EDPMOperation::saveInterval(QDataStream& ar, OpInterval* pOp)
{
    ar << pOp->getInt().getFrom();
    ar << pOp->getInt().getTo();
 }

void EDPMOperation::saveTS(QDataStream& ar, TS* pOp)
{
    ar << pOp->isFromMarking();
    ar << QString::fromStdString(pOp->getWord());
    ar << QString::fromStdString(pOp->getName());
    ar << QString::fromStdString(pOp->getFamily());
}

void EDPMOperation::load(QDataStream& ar, Operation*& pOp){
    EOpType eType;
    int tmp;

    ar>>tmp;
    eType = (EOpType)tmp;
    switch (eType) {
        case OP_DISTANCE	:	pOp = new OpDistance(); break;
        case OP_REITERATION	:	pOp = new OpReiteration(); break;
        case OP_INTERVAL	:	pOp = new OpInterval(); break;
        case OP_TS			:	pOp = new TS(); break;
        case OP_UNDEFINED	:	pOp = NULL; break;
    }

    switch (eType) {
        case OP_DISTANCE	:	loadDistance(ar, dynamic_cast<OpDistance*>(pOp)); break;
        case OP_REITERATION	:	loadReiteration(ar, dynamic_cast<OpReiteration*>(pOp)); break;
        case OP_INTERVAL	:	loadInterval(ar, dynamic_cast<OpInterval*>(pOp)); break;
        case OP_TS			:	loadTS(ar, dynamic_cast<TS*>(pOp)); break;
        case OP_UNDEFINED	:	return;
        default: assert(0);
    }

    for (int i=0; i<pOp->getArgumentNumber(); i++) {
        Operation *pArg = pOp->getArgument(i);
        EDPMOperation::load(ar,pArg);
        pOp->setArgument(pArg,i);
    }
}

void EDPMOperation::loadDistance(QDataStream& ar, OpDistance* pOp){
    DDisc::Interval dist;
    int tmp = 0;
    ar >> tmp;
    dist.setFrom(tmp);
    ar >> tmp;
    dist.setTo(tmp);
    pOp->setDistance(dist);
    EDistType type;
    ar >> tmp;
    type = (EDistType)tmp;
    pOp->setDistanceType( type);
    bool bOrder;
    ar >> bOrder;
    pOp->setOrderImportant(bOrder);
}
void EDPMOperation::loadReiteration(QDataStream& ar, OpReiteration* pOp){
    DDisc::Interval dist;
    DDisc::Interval count;
    int tmp = 0;
    ar >> tmp;
    dist.setFrom(tmp);
    ar >> tmp;
    dist.setTo(tmp);
    ar >> tmp;
    count.setFrom(tmp);
    ar >> tmp;
    count.setTo(tmp);

    pOp->setDistance(dist);
    EDistType type;
    ar >> tmp;
    type = (EDistType)tmp;
    pOp->setDistanceType( type );
    pOp->setCount(count);
}
void EDPMOperation::loadInterval(QDataStream& ar, OpInterval* pOp){
    DDisc::Interval interval;
    int tmp = 0;
    ar >> tmp;
    interval.setFrom(tmp);
     ar >> tmp;
    interval.setTo(tmp);
    pOp->setInt(interval);
}
void EDPMOperation::loadTS(QDataStream& ar, TS* pOp){
    bool bFromMarking;
    ar >> bFromMarking;
    pOp->setFromMarking(bFromMarking);
    QString strWord;
    QString strName;
    QString strFamily;

    ar >> strWord;
    ar >> strName;
    ar >> strFamily;

    pOp->setWord(strWord.toStdString());
    pOp->setName(strName.toStdString());
    pOp->setFamily(strFamily.toStdString());
}

} //namespace
