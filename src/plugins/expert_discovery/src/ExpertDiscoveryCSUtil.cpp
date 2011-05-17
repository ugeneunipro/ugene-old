#include "ExpertDiscoveryCSUtil.h"

#include "DDisc/statmath.h"
#include "DDisc/Context.h"

namespace U2 {
    CSFolder::CSFolder(CSFolder* pParentFolder)
        : QObject(pParentFolder)
    {
        setName("");

    }

    CSFolder::~CSFolder(void)
    {
        /*CSelectedSignalsContainer& rSel = GlobalGetDocument()->GetSelectedSignalsContainer();
        for (int id=0; id<(int) m_Signals.GetSize(); id++)
        {
            if (rSel.IsSelected(m_Signals[id]))
                rSel.RemoveSignal(m_Signals[id]);
        }*/
    }

    void CSFolder::clear()
    {
        for(uint i = 0; i < signalsVect.size(); i++){
            delete signalsVect[i];
        }
        signalsVect.clear();

        for(int i = 0; i < folders.size(); i++){
            delete folders[i];
        }
        folders.clear();
    }

    QString CSFolder::getName() const
    {
        return strName;
    }

    void CSFolder::setName(QString strName)
    {
        this->strName = strName;
    }

    CSFolder* CSFolder::getParentFolder() const
    {
        return (CSFolder*)parent();
    }

    void CSFolder::setParentFolder(CSFolder* pParentFolder)
    {
        setParent(pParentFolder);
    }

    int CSFolder::getSignalNumber() const
    {
        return (int) signalsVect.size();
    }

    Signal* CSFolder::getSignal(int id)
    {
        return signalsVect[id];
    }

    const Signal* CSFolder::getSignal(int id) const
    {
        return signalsVect[id];
    }

    int CSFolder::addSignal(Signal *pSignal, bool bReplace)
    {
        int nIndex = getSignalIndexByName(pSignal->getName().c_str());
        if (nIndex>=0) {
            if (bReplace) {
                delete signalsVect[nIndex];
                signalsVect.erase(signalsVect.begin()+nIndex);
            }
            else {
                return -1;
            }
        }
        signalsVect.push_back(pSignal);
        return 0;
            
    }

    int	CSFolder::getSignalIndexByName(QString strName) const
    {
        int nSigNum = (int) signalsVect.size();
        for (int i=0; i<nSigNum; i++) {
            if (QString::compare(QString::fromStdString(signalsVect[i]->getName()), strName)==0)
                return i;
        }
        return -1;
    }

    void CSFolder::deleteSignal(int id)
    {
        delete signalsVect[id];
        signalsVect.erase(signalsVect.begin()+id);
    }

    int CSFolder::getFolderNumber() const
    {
        return folders.size();
    }

    CSFolder* CSFolder::getSubfolder(int id)
    {
        return folders[id];
    }

    const CSFolder* CSFolder::getSubfolder(int id) const
    {
        return folders[id];
    }

    int CSFolder::addFolder(CSFolder *pFolder, bool bReplace)
    {
        int nIndex = getFolderIndexByName(pFolder->getName());
        if (nIndex>=0) { 
            if (bReplace)
            {
                CSFolder *pDestFolder = getSubfolder(nIndex);
                for (int i=0; i<pFolder->getFolderNumber(); i++)
                    pDestFolder->addFolder(pFolder->getSubfolder(i), true);
                for (int i=0; i<pFolder->getSignalNumber(); i++)
                    pDestFolder->addSignal(pFolder->getSignal(i), true);
                return nIndex;
            }
            else return -1;
        }
        else {
            pFolder->setParentFolder(this);

            folders.append(pFolder);
            return 0;
        }
    }

    int	CSFolder::getFolderIndexByName(QString strName) const
    {
        int nFolderNum = (int) folders.size();
        for (int i=0; i<nFolderNum; i++) {
            if (QString::compare(folders[i]->getName(), strName)==0)
                return i;
        }
        return -1;
    }

    QString	CSFolder::makeUniqueSignalName() const
    {
        QString strPrefix = "NewSignal";
        //if (!strPrefix.LoadString(IDS_PFX_SIGNAL)) ASSERT(0);

        int n = 0;
        QString strResult = "NewSignal";
        while (getSignalIndexByName(strResult)>=0) {
            strResult = strPrefix + QString("%1").arg(n);
            n++;
        }
        return strResult;
    }

    QString	CSFolder::makeUniqueFolderName() const
    {
        QString strPrefix= "NewFolder";
        //if (!strPrefix.LoadString(IDS_PFX_FOLDER)) ASSERT(0);

        int n = 0;
        QString strResult = "NewFolder";
        while (getFolderIndexByName(strResult)>=0) {
            strResult = strPrefix + QString("%1").arg(n);
            n++;
        }
        return strResult;
    }

    void CSFolder::deleteFolder(int id) 
    {
        delete folders[id];
        folders.erase(folders.begin()+id);

    }

    bool CSFolder::doConstructPath(QString& strPath, const Signal* pSignal) const
    {
        for (int i=0; i<getSignalNumber(); i++) {
            if (getSignal(i) == pSignal) {
                strPath += QString("\\") + QString::fromStdString(pSignal->getName());
                return true;
            }
        }
        for (int i=0; i<getFolderNumber(); i++) {
            const CSFolder* pFolder = getSubfolder(i);
            QString strLocalPath = strPath + "\\" + pFolder->getName();
            if (pFolder->doConstructPath(strLocalPath, pSignal)) {
                strPath = strLocalPath;
                return true;
            }
        }
        return false;
    }

    QString CSFolder::getPathToSignal(const Signal* pSignal) const
    {
        QString strPath;
        doConstructPath(strPath, pSignal);
        return strPath;
    }

    const Signal* CSFolder::getSignalByPath(QString strPath) const
    {
        int nPos = strPath.indexOf("\\");
        if (nPos == 0) {
            strPath = strPath.right(strPath.length()-1);
            nPos = strPath.indexOf("\\");
        }
        if (nPos <= 0) {
            int nIndex = getSignalIndexByName(strPath);
            if (nIndex < 0)
                return NULL;
            else 
                return getSignal(nIndex);
        }

        QString strFolderName(strPath.left(nPos));
        QString strNewPath(strPath.right(strPath.length() - nPos - 1));
        int nIndex = getFolderIndexByName(strFolderName);
        if (nIndex < 0)
            return NULL;
        return getSubfolder(nIndex)->getSignalByPath(strNewPath);
    }

    CSFolder* CSFolder::clone() const
    {
        CSFolder* pFolder = new CSFolder;
        pFolder->setName(getName());
        for (int i=0; i<getFolderNumber(); i++)
        {
            pFolder->addFolder(getSubfolder(i)->clone());
        }
        for (int i=0; i<getSignalNumber(); i++)
        {
            pFolder->addSignal(getSignal(i)->clone());
        }
        return pFolder;
    }

EDProcessedSignal::EDProcessedSignal()
    : m_dProbability		(UNDEFINED_VALUE)
    , m_dFisher				(UNDEFINED_VALUE)
    , m_dUl					(UNDEFINED_VALUE)
    , m_dPosCoverage		(UNDEFINED_VALUE)
    , m_dNegCoverage		(UNDEFINED_VALUE)

{
}

EDProcessedSignal* EDProcessedSignal::processSignal(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase) {
    EDProcessedSignal* pPS = NULL;
    if (pOp==NULL || !pOp->check())
        return NULL;
    switch (pOp->getType()) {
    case OP_INTERVAL	: pPS = new EDProcessedInterval(); break;
    case OP_REITERATION	: pPS = new EDProcessedReiteration(); break;
    case OP_DISTANCE	: pPS = new EDProcessedDistance(); break;
    case OP_TS			: pPS = new EDProcessedTS(); break;
    };
    if (pPS != NULL) {
        pPS->setYesSequenceNumber(pYesBase->getSize());
        pPS->setNoSequenceNumber(pNoBase->getSize());
        pPS->setTextDescription(pOp->getDescription().c_str());
        //	for (int i=0; i<pOp->getArgumentNumber(); i++)
        //		pPS->AddChild(ProcessSignal(pOp->getArgument(i), pYesBase, pNoBase));
        pPS->makeStandardProcessing(pOp, pYesBase, pNoBase);
        pPS->process(pOp, pYesBase, pNoBase);
    }
    return pPS;
}

void EDProcessedSignal::makeStandardProcessing(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase) 
{
    bool first;
    int a[2][2] = {{0,0},{0,0}};
    int aseq[2][2] = {{0,0},{0,0}};
    int nSeqNum = pYesBase->getSize();
    Signal sig(pOp);
    Context& rContext = sig.createCompartibleContext();
    for (int i=0; i<nSeqNum; i++) {
        first = true;
        const Sequence& rSeq = pYesBase->getSequence(i);
        Set set;
        set.init((unsigned)rSeq.getSize());
        while (sig.find(rSeq, rContext)) {
            if (first) {
                first = false;
                aseq[1][1]++;
            }
            a[1][1]++;
            int nPos = rContext.getPosition();
            int nLen = rContext.getLength();
            for (int k=nPos; k<nPos+nLen; k++) { 
                if (rContext.isSignalPart(k)){
                    set.set(k);
                    set.associate(k, rContext.getTSName(k));
                }
            }
        }
        if (first) {
            a[0][1]++;
            aseq[0][1]++;
        }
        setYesRealizations(i, set);
        rContext.reset();
    }

    nSeqNum = pNoBase->getSize();
    for (int i=0; i<nSeqNum; i++) {
        first = true;
        const Sequence& rSeq = pNoBase->getSequence(i);
        Set set;
        set.init((unsigned)rSeq.getSize());
        while (sig.find(rSeq, rContext)) {
            if (first) {
                first = false;
                aseq[1][0]++;
            }
            a[1][0]++;
            int nPos = rContext.getPosition();
            int nLen = rContext.getLength();
            for (int k=nPos; k<nPos+nLen; k++) {
                if (rContext.isSignalPart(k)) {
                    set.set(k);
                    set.associate(k, rContext.getTSName(k));
                }
            }
        }
        if (first) {
            a[0][0]++;
            aseq[0][0]++;
        }
        setNoRealizations(i, set);
        rContext.reset();
    }

    QString str;

    str = "Undefined ( ";
    if (a[1][1]+a[1][0] != 0) {
        m_dProbability = 100*a[1][1]/double(a[1][1]+a[1][0]);
        str = QString("%1").arg(m_dProbability) + "% ( ";
        //str = (to_string(m_dProbability)+"% ( ").c_str();
    }
    //str += (to_string(a[1][1]) + " / " + to_string(a[1][1]+a[1][0]) + " )").c_str();
    str += QString("%1").arg(a[1][1]) + " / " + QString("%1").arg(a[1][1]+a[1][0]) + " )";
    addProperty("Probability", str);

    str = "Undefined ( ";
    if (aseq[1][1]+aseq[0][1]!= 0) {
        m_dPosCoverage = 100*aseq[1][1]/double(aseq[1][1]+aseq[0][1]);
        //str = (to_string(m_dPosCoverage)+"% ( ").c_str();
        str = QString("%1").arg(m_dPosCoverage)+"% ( ";
    }
    //str += (to_string(aseq[1][1]) + " / " + to_string(aseq[1][1]+aseq[0][1]) + " )").c_str();
    str += QString("%1").arg(aseq[1][1]) + " / " + QString("%1").arg(aseq[1][1]+aseq[0][1]) + " )";
    addProperty("Pos. coverage", str);

    str = "Undefined ( ";
    if (aseq[1][0]+aseq[0][0]!= 0) {
        m_dNegCoverage = 100*aseq[1][0]/double(aseq[1][0]+aseq[0][0]);
        str = QString("%1").arg(m_dNegCoverage)+"% ( ";
    }
    str += QString("%1").arg(aseq[1][0]) + " / " + QString("%1").arg(aseq[1][0]+aseq[0][0]) + " )";
    addProperty("Neg. coverage", str);


    m_dFisher = fisher(a[0][0], a[0][1], a[1][0], a[1][1]);
    //str.Format("%E", m_dFisher);
    
    addProperty("Fisher", QString("%1").arg(m_dFisher));

    m_dUl = ul(a[0][0], a[0][1], a[1][0], a[1][1]);
    //str.Format("%E", m_dUl);
    addProperty("Ul", QString("%1").arg(m_dUl));

    rContext.destroy();
    sig.detach();
}

QString EDProcessedSignal::getPropertyValue(QString name) const
{
    for (int i=0; i<(int)m_arNames.size(); i++)
    {
        if (!m_arNames[i].compare(name, Qt::CaseInsensitive))
            return m_arValues[i];
    }
    return QString();
}

void EDProcessedInterval::process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase) 
{

}

void EDProcessedDistance::process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase)
{
    OpDistance *pDist = dynamic_cast<OpDistance*>(pOp);
    assert(pDist != 0);
    int ai[2][2][2] = {{{0,0},{0,0}},{{0,0},{0,0}}};
    const SequenceBase *pBasei[2] = {pYesBase, pNoBase};

    Signal sig1(pDist->getArgument(0));
    Signal sig2(pDist->getArgument(1));
    Context& rContext1 = sig1.createCompartibleContext();
    Context& rContext2 = sig2.createCompartibleContext();
    for (int b=0; b<2; b++) {
        int (&a)[2][2] = ai[b];
        const SequenceBase* pBase = pBasei[b];
        int nSeqNum = pBase->getSize();
        int r1 = 0, r2 = 0;
        for (int i=0; i<nSeqNum; i++) {
            const Sequence& rSeq = pBase->getSequence(i);
            if (sig1.find(rSeq, rContext1)) r1 = 1;
            else r1 = 0;
            if (sig2.find(rSeq, rContext2)) r2 = 1;
            else r2 = 0;
            a[r1][r2]++;
            rContext1.reset();
            rContext2.reset();
        }
    }
    rContext1.destroy();
    rContext2.destroy();
    sig1.detach();
    sig2.detach();

    QString str;
    double dCC = corelation(ai[0][0][0], ai[0][0][1], ai[0][1][0], ai[0][1][1]);
    if (dCC == CORELATION_UNDEFINED)
        str = "Undefined";
    else 
        str =QString("%1").arg(dCC);
    addProperty("Param. corelation on pos.", str);

    dCC = corelation(ai[1][0][0], ai[1][0][1], ai[1][1][0], ai[1][1][1]);
    if (dCC == CORELATION_UNDEFINED)
        str = "Undefined";
    else 
        //str = to_string(dCC);
        str = QString("%1").arg(dCC);
    addProperty("Param. corelation on neg.", str);
}

void EDProcessedReiteration::process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase)
{

}

void EDProcessedTS::process(Operation *pOp, const SequenceBase *pYesBase, const SequenceBase *pNoBase)
{

}

//////////////////////////////////////////////////////////////////////////////////

SelectedSignalsContainer::SelectedSignalsContainer(void)
{
}

SelectedSignalsContainer::~SelectedSignalsContainer(void)
{
}

void SelectedSignalsContainer::AddSignal(const Signal* pSignal)
{
    m_SelectedSignals.insert(pSignal);
}

void SelectedSignalsContainer::RemoveSignal(const Signal* pSignal)
{
    SignalList::iterator it = m_SelectedSignals.find(pSignal);
    if (m_SelectedSignals.end() != it)
        m_SelectedSignals.erase(it);
}

const SignalList& SelectedSignalsContainer::GetSelectedSignals() const
{
    return m_SelectedSignals;
}

bool SelectedSignalsContainer::IsSelected(const Signal *pSignal) const
{
    SignalList::const_iterator it = m_SelectedSignals.find(pSignal);
    if (m_SelectedSignals.end() != it)
        return true;
    else 
        return false;
}

void  SelectedSignalsContainer::save(QDataStream& ar, CSFolder& rootF){
    SignalList::iterator it = m_SelectedSignals.begin();
    int nSize = (int) m_SelectedSignals.size();
    ar << nSize;
    while (m_SelectedSignals.end() != it) {
        QString strPath = rootF.getPathToSignal((*it));
        assert(!strPath.isEmpty());
        ar << strPath;
        it++;
    }
}
void  SelectedSignalsContainer::load(QDataStream& ar, CSFolder& rootF){
    int nSize;
    ar >> nSize;
    for (int i=0; i<nSize; i++) {
        QString strPath;
        ar >> strPath;
        const Signal* pSignal = rootF.getSignalByPath(strPath);
        assert(pSignal != NULL);
        if (pSignal)
            AddSignal(pSignal);
    }
}

RecognizationDataStorage::~RecognizationDataStorage(){
    clear();
}
void RecognizationDataStorage::clear(){
    foreach(RecognizationData* d, recMap){
        if (d){
            delete d;
        }
    }

    recMap.clear();

}
void RecognizationDataStorage::addSequence(QString& seqName){
    if (recMap.contains(seqName)){
        RecognizationData *d = recMap.value(seqName);
        if(d){
            delete d;
        }
    }
    recMap.insert(seqName, NULL);
}
bool RecognizationDataStorage::getRecognizationData(RecognizationData& data, const Sequence* seq, const SelectedSignalsContainer& rSe){
 
    if (seq->isHasScore() && !(getRecData(seq) == NULL)){
        data = *getRecData(seq);
        return !data.empty();

    }
  
    const SignalList& rSelList = rSe.GetSelectedSignals();
    if (rSelList.size() == 0)
        return false;

    data.resize(seq->getSize());
    fill(data.begin(), data.end(), 0);

    SignalList::const_iterator iter = rSelList.begin();
    while (iter != rSelList.end()) {
        const Signal* pSignal = (*iter);
        Context& context = pSignal->createCompartibleContext();
        while (pSignal->find(*seq,context)) {
            double t = pSignal->getPriorProbability()/100;
            if (t>=1) t = 0.999999;
            int nPos = context.getPosition();
            int not_null_length = 0;
            double value = -log(1-t);
            for (int i=0; i<context.getLength(); i++)
            {
                if (context.isSignalPart(nPos+i))
                    not_null_length++;
            }
            value /= not_null_length;
            for (int i=0; i<context.getLength(); i++)
            {
                if (context.isSignalPart(nPos+i))
                    data[nPos + i] += value;
            }
        }
        context.destroy();
        iter++;
    }
    RecognizationData* d = recMap.value(QString::fromStdString(seq->getName()));
    if(d != NULL){
        delete d;
    }
    d = new RecognizationData(data);
   
    return true;
       
}
RecognizationData* RecognizationDataStorage::getRecData(const Sequence* seq){
    if(!recMap.contains(QString::fromStdString(seq->getName())) ){
        return NULL;
    }else{
        return recMap.value(QString::fromStdString(seq->getName()));
    }
}


} //namespace
