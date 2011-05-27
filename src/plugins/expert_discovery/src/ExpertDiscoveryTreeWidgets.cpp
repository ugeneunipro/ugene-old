#include "ExpertDiscoveryTreeWidgets.h"



#include<QMessageBox>
#include <QtGui/QMouseEvent>

#include <strstream>
using std::strstream;
using std::ws;

namespace U2 {
 

////////////////////////////////////////////////////////////
EDPIPropertyType::EDPIPropertyType(){

}
EDPIPropertyType::~EDPIPropertyType(){

}
//////////////////////////////////////////////////////////////////
EDPIProperty::EDPIProperty(const EDPIProperty& rProperty) 
    : m_pType(NULL)
    , m_pCallback(NULL)
{
    *this = rProperty;
}

EDPIProperty::EDPIProperty(QString strName /* ="" */) 
    : m_pType(NULL)
    , m_pCallback(NULL)
{
    setType(EDPIPropertyTypeStaticString::getInstance());
    setName( strName );
}

EDPIProperty::~EDPIProperty() {
    delete m_pCallback;
}

const EDPIProperty& EDPIProperty::operator =(const EDPIProperty& rProperty) {
    setType( rProperty.m_pType);
    setName( rProperty.getName() );
    if (rProperty.m_pCallback)
        m_pCallback = rProperty.m_pCallback->clone();
    else 
        m_pCallback = NULL;
    return *this;
}

QString EDPIProperty::getName() const {
    return m_strName;
}

void EDPIProperty::setName(QString strName) {
    m_strName = strName;
}

QString	EDPIProperty::getValue() const {
    if (m_pCallback)
        return m_pCallback->call();
    else 
    {
        QString strUndefined = "Undefined";
        return strUndefined;
    }
}

void EDPIProperty::setCallback(ICallback* pCallback)
{
    m_pCallback = pCallback;
}
const EDPIPropertyType* EDPIProperty::getType() const
{
    return m_pType;
}

void EDPIProperty::setType(EDPIPropertyType* pType)
{
    m_pType = pType;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////


EDPIPropertyGroup::EDPIPropertyGroup(QString strName /*=""*/)
{
    setName(strName);
}

EDPIPropertyGroup::EDPIPropertyGroup(const EDPIPropertyGroup& rGroup)
{
    *this = rGroup;
}

EDPIPropertyGroup::~EDPIPropertyGroup()
{
}

const EDPIPropertyGroup& EDPIPropertyGroup::operator=(const EDPIPropertyGroup& rGroup)
{
    m_strName = rGroup.getName();
    m_arProperties = rGroup.m_arProperties;
    return *this;
}

QString EDPIPropertyGroup::getName() const
{
    return m_strName;
}

void EDPIPropertyGroup::setName(QString strName)
{
    m_strName = strName;
}

int	EDPIPropertyGroup::getPropertiesNumber() const {
    return  m_arProperties.size();
}

EDPIProperty& EDPIPropertyGroup::getProperty(int nProp) {
    assert( nProp>=0 && nProp<m_arProperties.size());
    return m_arProperties[nProp];
}

const EDPIProperty& EDPIPropertyGroup::getProperty(int nProp) const {
    assert( nProp>=0 && nProp<m_arProperties.size());
    return m_arProperties[nProp];
}

void EDPIPropertyGroup::addProperty(EDPIProperty &rProperty) {
    m_arProperties.append(rProperty);
}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeStaticString)

EDPIPropertyTypeStaticString::EDPIPropertyTypeStaticString()
{
}

EDPIPropertyTypeStaticString::~EDPIPropertyTypeStaticString()
{
}

bool EDPIPropertyTypeStaticString::isNumber() const 
{
    return false;
}

bool EDPIPropertyTypeStaticString::hasEdit() const 
{
    return false;
}

bool EDPIPropertyTypeStaticString::hasPredefinedValues() const
{
    return false;
}

int	EDPIPropertyTypeStaticString::getValueNumber() const
{
    return 0;
}

QString EDPIPropertyTypeStaticString::getValue(int i) const
{
    assert(0);
    return "";
}

bool EDPIPropertyTypeStaticString::isValidValue(QString strValue) const
{
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeString)

EDPIPropertyTypeString::EDPIPropertyTypeString()
{
}

EDPIPropertyTypeString::~EDPIPropertyTypeString()
{
}

bool EDPIPropertyTypeString::hasEdit() const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeUnsignedInt)

EDPIPropertyTypeUnsignedInt::EDPIPropertyTypeUnsignedInt()
{
}

EDPIPropertyTypeUnsignedInt::~EDPIPropertyTypeUnsignedInt()
{
}

bool EDPIPropertyTypeUnsignedInt::isNumber() const
{
    return true;
}

bool EDPIPropertyTypeUnsignedInt::hasEdit() const
{
    return true;
}

bool EDPIPropertyTypeUnsignedInt::hasPredefinedValues() const
{
    return false;
}

int EDPIPropertyTypeUnsignedInt::getValueNumber() const
{
    return 0;
}

QString EDPIPropertyTypeUnsignedInt::getValue(int i) const
{
    assert(0);
    return "";
}

bool EDPIPropertyTypeUnsignedInt::isValidValue(QString strValue) const
{
    strstream str;
    str << strValue.toStdString();
    int value = 0;
    str >> value;
    str >> ws;
    if (value>=0 && str.eof())
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeUnsignedIntWithUnl)

EDPIPropertyTypeUnsignedIntWithUnl::EDPIPropertyTypeUnsignedIntWithUnl()
: EDPIPropertyTypeListEdit()
{
   addValue("Unlimited");
}

EDPIPropertyTypeUnsignedIntWithUnl::~EDPIPropertyTypeUnsignedIntWithUnl()
{
}

bool EDPIPropertyTypeUnsignedIntWithUnl::isNumber() const
{
    return true;
}

bool EDPIPropertyTypeUnsignedIntWithUnl::isValidValue(QString strValue) const
{
    int nId = getValueId(strValue);
    if (nId >= 0) 
        return true;

    int nValue = 0;
    strstream str;
    str << strValue.toStdString();
    str >> nValue;
    str >> ws;
    if (str.eof() && nValue>=0)
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////

EDPIPropertyTypeList::EDPIPropertyTypeList() 
{
}

EDPIPropertyTypeList::~EDPIPropertyTypeList() 
{
}

bool EDPIPropertyTypeList::isNumber() const 
{
    return false;
}

bool EDPIPropertyTypeList::hasEdit() const 
{
    return false;
}

bool EDPIPropertyTypeList::hasPredefinedValues() const
{
    return true;
}

int EDPIPropertyTypeList::getValueNumber() const 
{
    return m_arValues.size();
}

QString EDPIPropertyTypeList::getValue(int i) const
{
    return m_arValues[i];
}

bool EDPIPropertyTypeList::isValidValue(QString strValue) const
{
    return getValueId(strValue) >= 0;
}


void EDPIPropertyTypeList::addValue(QString strValue) 
{
    m_arValues.append(strValue);
}

int	EDPIPropertyTypeList::getValueId(QString strValue) const
{
    int nValNum = getValueNumber();
    for (int i=0; i<nValNum; i++) {
        if (m_arValues[i].compare(strValue,Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}


//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeListCSNodeTypes);

EDPIPropertyTypeListCSNodeTypes::EDPIPropertyTypeListCSNodeTypes()
{
   addValue("Distance");
   addValue("Repetition");
   addValue("Interval");
   addValue("Word");
   addValue("Markup item");
}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeBool);

EDPIPropertyTypeBool::EDPIPropertyTypeBool()
{
   addValue("False");
   addValue("True");
}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_GETINSTANCE(EDPIPropertyTypeDistType);

EDPIPropertyTypeDistType::EDPIPropertyTypeDistType()
{
   addValue("finish to start");
   addValue("start to start");
   addValue("middle to start");
}

////////////////////////////////////////////////////////////
EDProjectItem::EDProjectItem():QTreeWidgetItem(), mInf(NULL){
    setName("");
    sortField = ED_FIELD_UNDEFINED;
    sortOrd = ED_ORDER_DECREASING;
}
EDProjectItem::~EDProjectItem(){
    clearGroups();
}
int	EDProjectItem::getGroupNumber() const{
    return m_arGroups.size();
}
EDPIPropertyGroup& EDProjectItem::getGroup(int nGroup){
    assert(nGroup >=0 && nGroup<getGroupNumber());
    return m_arGroups[nGroup];
}
const EDPIPropertyGroup& EDProjectItem::getGroup(int nGroup) const{
    assert(nGroup >=0 && nGroup<getGroupNumber());
    return m_arGroups[nGroup];
}
int	EDProjectItem::addGroup(const EDPIPropertyGroup& rGroup){
    m_arGroups.append(rGroup);
    return m_arGroups.size();
}
void EDProjectItem::clearGroups(){
    m_arGroups.clear();
}
EItemType EDProjectItem::getType() const{
    return PIT_NONE;
}

const EDProjectItem* EDProjectItem::findItemConnectedTo(void *pData) const{
    if (isConnectedTo(pData))
        return this;
 
    for (int i = 0; i < childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(child(i));
        if(item){
            const EDProjectItem* cItem = item->findItemConnectedTo(pData);
            if(cItem){
                return cItem;
            }

        }
    }
    return NULL;

}

bool EDProjectItem::operator<(const QTreeWidgetItem &other) const{
    const EDProjectItem* pItem1 = dynamic_cast<const EDProjectItem*>(this);
    const EDProjectItem* pItem2 = dynamic_cast<const EDProjectItem*>(&other);

    if(!pItem2 || !pItem1){
         return QTreeWidgetItem::operator<(other);
    }
    EItemType type1 = pItem1->getType();
    EItemType type2 = pItem2->getType();

    if(sortOrd == ED_ORDER_DECREASING){
       
        if (type1 == PIT_CS && type2 == PIT_CS){
            const EDProjectItem* pItem3;
            pItem3 = pItem1;
            pItem1 = pItem2;
            pItem2 = pItem3;

        }
    }
    if (type1 == PIT_CS && type2 == PIT_CS){
        const Signal* pSnl1 = ((EDPICS*) pItem1)->getSignal();
        const Signal* pSnl2 = ((EDPICS*) pItem2)->getSignal();
        switch (sortField) {
         case ED_FIELD_UNDEFINED: return QTreeWidgetItem::operator<(other);
         case ED_FIELD_PROBABILITY: return (pSnl1->getPriorProbability()<pSnl2->getPriorProbability());
         case ED_FIELD_COVERAGE: return (pSnl1->getPriorPosCoverage()<pSnl2->getPriorPosCoverage());
         case ED_FIELD_FISHER: return (pSnl1->getPriorFisher()<pSnl2->getPriorFisher());
         case ED_FIELD_NAME:
         default: return pItem1->getName() < pItem2->getName();

        }
    }else if ( (type1 == PIT_CS_FOLDER || type1 == PIT_CS || type1 == PIT_MRK_ITEM) &&
        (type2 == PIT_CS_FOLDER || type2 == PIT_CS || type2 == PIT_MRK_ITEM)) 
    {
        if ((int)type1 < (int)type2)
            return false;
        else 
            if ((int)type1 > (int)type2)
                return true;
            else 
                return pItem1->getName() < pItem2->getName();
    }else{ 
        const EDProjectItem *pParent = dynamic_cast<const EDProjectItem*>(dynamic_cast<const QTreeWidgetItem*>(pItem1)->parent());
        if (pParent != NULL && pParent->getType() == PIT_CSN_DISTANCE) {
            return (pParent->child(0) == pItem1) ? true : false;
        }
        else{
            return QTreeWidgetItem::operator<(other);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////

EDPISequenceRoot::EDPISequenceRoot(ExpertDiscoveryData& edD) 
:edData(edD)
{
    update(true);
}

EDPISequenceRoot::~EDPISequenceRoot() 
{
}

EItemType EDPISequenceRoot::getType() const {
    return PIT_SEQUENCEROOT;
}

void EDPISequenceRoot::update(bool bUpdateChildren)
{
    QString strName = "Sequences";
    setName(strName);
    clearGroups();

    if (bUpdateChildren) {
        takeChildren();
        
        addChild(new EDPIPosSequenceBase(edData.getPosSeqBase(), edData));
        addChild(new EDPINegSequenceBase(edData.getNegSeqBase(), edData));	
        //if (pDoc->GetControlSeqBase().getSize() != 0)
            addChild(new EDPIControlSequenceBase(edData.getConSeqBase(), edData));
    }
}



bool EDPISequenceRoot::isConnectedTo(void *pData) const
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////


EDPISequenceBase::EDPISequenceBase(QString strName, const SequenceBase& rSeqBase, ExpertDiscoveryData& edD)
: m_rSeqBase (rSeqBase)
,edData(edD)
{
    //assert(strName != NULL);
    setName(strName);
    update(true);
}

void EDPISequenceBase::update(bool bUpdateChildren) 
{
    clearGroups();
    takeChildren();

    int nSize = m_rSeqBase.getSize();

    QString strSize = "Size";
    QString strGenInfo = "General information";
    

    EDPIProperty PropSize(strSize);
    PropSize.setCallback(new Callback<const DDisc::SequenceBase, int>(&m_rSeqBase, &DDisc::SequenceBase::getSize));

    EDPIPropertyGroup GenInfo(strGenInfo);
    GenInfo.addProperty(PropSize);
    addGroup(GenInfo);

    if (bUpdateChildren) {
        for (int id=0; id<nSize; id++)
            addChild(new EDPISequence(m_rSeqBase, id, edData));
    }
}

EDPISequenceBase::~EDPISequenceBase() 
{
}

const SequenceBase& EDPISequenceBase::getSequenceBase() const 
{
    return m_rSeqBase;
}

bool EDPISequenceBase::isConnectedTo(void *pData) const
{
    return ((void*) &m_rSeqBase) == pData;
}

/////////////////////////////////////////////////////////////////////////////////////

EDPIPosSequenceBase::EDPIPosSequenceBase(const SequenceBase& rBase, ExpertDiscoveryData& edD) 
: EDPISequenceBase("", rBase, edD) 
{
    QString strPosName = "Positive";
    setName(strPosName);
}

EDPIPosSequenceBase::~EDPIPosSequenceBase() 
{
}

void EDPIPosSequenceBase::update(bool bUpdateChildren) 
{
    EDPISequenceBase::update(bUpdateChildren);
}

EItemType EDPIPosSequenceBase::getType() const
{
    return PIT_POSSEQUENCEBASE;
}


/////////////////////////////////////////////////////////////////////////////////////

EDPINegSequenceBase::EDPINegSequenceBase(const SequenceBase& rBase, ExpertDiscoveryData& edD) 
: EDPISequenceBase("", rBase, edD) 
{
    QString strNegName = "Negative";
    setName(strNegName);
}

EDPINegSequenceBase::~EDPINegSequenceBase() 
{
}

void EDPINegSequenceBase::update(bool bUpdateChildren) 
{
    EDPISequenceBase::update(bUpdateChildren);
}

EItemType EDPINegSequenceBase::getType() const
{
    return PIT_NEGSEQUENCEBASE;
}

/////////////////////////////////////////////////////////////////////////////////////

EDPIControlSequenceBase::EDPIControlSequenceBase(const SequenceBase& rBase, ExpertDiscoveryData& edD) 
: EDPISequenceBase("", rBase, edD) 
{
    QString strControlName = "Control";
    setName(strControlName);
    update(true);
}

EDPIControlSequenceBase::~EDPIControlSequenceBase() 
{
}

void EDPIControlSequenceBase::update(bool bUpdateChildren) 
{
    clearGroups();

    int nSize = m_rSeqBase.getSize();

    QString strSize = "Size";
    QString strGenInfo = "General information";

    EDPIProperty PropSize(strSize);
    PropSize.setCallback(new Callback<const DDisc::SequenceBase, int>(&m_rSeqBase, &DDisc::SequenceBase::getSize));
    EDPIPropertyGroup GenInfo(strGenInfo);
    GenInfo.addProperty(PropSize);
    addGroup(GenInfo);

    if (bUpdateChildren) {
        takeChildren();
        for (int id=0; id<nSize; id++)
            addChild(new EDPIControlSequence(m_rSeqBase, id, edData));
    }
}

EItemType EDPIControlSequenceBase::getType() const
{
    return PIT_CONTROLSEQUENCEBASE;
}

////////////////////////////////////////////////////////

EDPISequence::EDPISequence(const SequenceBase& rSeqBase, int id, ExpertDiscoveryData& edD)
: m_id (id)
, m_rSeq (rSeqBase.getSequence(id))
, m_firstCall(true)
,edData(edD)
{
    assert(id>=0 && id<rSeqBase.getSize());
    update(true);
}

void EDPISequence::update(bool bUpdateChildren)
{
    setName( m_rSeq.getName().c_str() );
    clearGroups();
    takeChildren();

    QString strName = "Name";
    QString strLength = "Length";
    QString strGenInfo = "Sequence info";
    QString strRecognizationInfo = "Sequence recogn. data";
    QString strScore = "Score";
    QString strBound = "Bound";
    QString strRecResult = "Result";
    

    EDPIProperty PropName(strName);
    EDPIProperty PropSize(strLength);
    PropName.setCallback(new Callback<const DDisc::Sequence, const std::string>(&m_rSeq, &DDisc::Sequence::getName));
    PropSize.setCallback(new Callback<const DDisc::Sequence, size_t>(&m_rSeq, &DDisc::Sequence::getSize));
    EDPIPropertyGroup GenInfo(strGenInfo);
    GenInfo.addProperty(PropName);
    GenInfo.addProperty(PropSize);
    addGroup(GenInfo);

    double dScore = m_rSeq.getScore();
    double dBound = edData.getRecognizationBound();

    EDPIProperty PropScore(strScore);
    PropScore.setCallback(new Callback<EDPISequence, QString>(this, &EDPISequence::getScore));
    EDPIProperty PropBound(strBound);
    PropBound.setCallback(new Callback<ExpertDiscoveryData, double>(&edData, &ExpertDiscoveryData::getRecognizationBound));
    EDPIProperty PropResult(strRecResult);
    PropResult.setCallback(new Callback<EDPISequence, QString>(this, &EDPISequence::getResult));
    EDPIPropertyGroup RecInfo(strRecognizationInfo);
    RecInfo.addProperty(PropScore);
    RecInfo.addProperty(PropBound);
    RecInfo.addProperty(PropResult);
    addGroup(RecInfo);
    //}
}

QString EDPISequence::getScore() const
{
    if (getType() == PIT_CONTROLSEQUENCE)
        return "0";
    if (m_rSeq.isHasScore() || edData.updateScore(const_cast<Sequence&>(m_rSeq)))
        return toString(m_rSeq.getScore());
    return "0";
}

QString EDPISequence::getResult() const
{
    if (getType() == PIT_CONTROLSEQUENCE)
        return "False";

    if ((m_rSeq.isHasScore() || edData.updateScore(const_cast<Sequence&>(m_rSeq))) &&
        m_rSeq.getScore() > edData.getRecognizationBound())
        return "True";
    else
        return "False";
}


EDPISequence::~EDPISequence()
{

}

EItemType EDPISequence::getType() const
{
    return PIT_SEQUENCE;
}

QString EDPISequence::getSequenceCode()
{
    return QString::fromStdString(m_rSeq.getSequence());
}

QString EDPISequence::getSequenceName()
{
    return QString::fromStdString(m_rSeq.getName());
}

bool EDPISequence::isConnectedTo(void *pData) const
{
    return ((void*) &m_rSeq) == pData;
}
////////////////////////////////////////////////////////////

const CSFolder* EDPICSDirectory::getFolder() const{
    return folder;
}
void EDPICSDirectory::setFolder(const CSFolder* pFolder){
    folder = pFolder;
}
void EDPICSDirectory::update(bool bupdateChildren){
    if (bupdateChildren) {
        takeChildren();

        const CSFolder* pFolder = getFolder();
        int nSubfolderNum = pFolder->getFolderNumber();
        for (int nSubfolder=0; nSubfolder<nSubfolderNum; nSubfolder++){
            EDPICSFolder* nFol = new EDPICSFolder(pFolder->getSubfolder(nSubfolder));
            addChild(dynamic_cast<EDProjectItem*>(nFol));
        }
                        

        int nSignalNum = pFolder->getSignalNumber();
        for (int nSignal=0; nSignal<nSignalNum; nSignal++){
            EDPICS* nSig = new EDPICS( pFolder->getSignal(nSignal) );
            addChild(dynamic_cast<EDProjectItem*>(nSig));
        }
            
    }      
}

bool EDPICSDirectory::isConnectedTo(void *pData) const{
    return ((void*) folder) == pData;
}

////////////////////////////////////////////////
EDPICSFolder::EDPICSFolder(const CSFolder *pFolder)
: EDPICSDirectory(pFolder){
    update(true);
}
EDPICSFolder::~EDPICSFolder(){

}
EItemType EDPICSFolder::getType() const{
    return PIT_CS_FOLDER;
}
void EDPICSFolder::update(bool bupdateChildren){
    clearGroups();

    QString strName = "Name";
    QString strEditor = "Editor";

    EDPIProperty propName(strName);
    propName.setCallback(new Callback<const CSFolder, QString>(getFolder(), &CSFolder::getName));
    propName.setType(EDPIPropertyTypeString::getInstance());

    EDPIPropertyGroup editor(strEditor);
    editor.addProperty(propName);
    addGroup(editor);

    EDPICSDirectory::update(bupdateChildren);
}
//virtual CExtPopupMenuWnd*	CreatePopupMenu(HWND hWndCmdRecieve) const;
QString EDPICSFolder::getName() const{
    return getFolder()->getName();
}
////////////////////////////////////////////////

EDPICSRoot::EDPICSRoot(CSFolder& rootF)
: EDPICSDirectory(NULL)
{
    setFolder(&rootF);
    update(true);
}

void EDPICSRoot::update(bool bUpdateChildren)
{
    QString strCSRoot = "Complex signals";
    
    setName(strCSRoot);
    EDPICSDirectory::update(bUpdateChildren);
}

EDPICSRoot::~EDPICSRoot()
{
}

EItemType EDPICSRoot::getType() const
{
    return PIT_CS_ROOT;
}


///////////////////////////////////////////////////

EDPICSNode::EDPICSNode(Operation *pOp1)
:pOp(pOp1)
,m_pPS(NULL)
{

}
EDPICSNode::~EDPICSNode(){
    delete m_pPS;
}
Operation* EDPICSNode::getOperation(){
   return pOp;
}

void EDPICSNode::setOperation(Operation *pOp){
    this->pOp = pOp;
}
const Operation* EDPICSNode::getOperation() const{
   return pOp;
}
void EDPICSNode::update(bool bupdateChildren){
    delete m_pPS;
    m_pPS = NULL;

    if (pOp == NULL) {
        setName("Undefined");
    }
    else  {
        setName(QString::fromStdString(pOp->getDescription()));	
        if (bupdateChildren) {
            takeChildren();
            int nArgNum = pOp->getArgumentNumber();
            for (int nArg=0; nArg<nArgNum; nArg++){
                EDProjectItem* ch = dynamic_cast<EDProjectItem*>(EDPICSNode::createCSN(pOp->getArgument(nArg)));
                emit si_getMetaInfoBase();
                ch->setMetainfoBase(getMinfo());
                addChild( ch );
                if(ch->getType() != PIT_CSN_UNDEFINED){
                    ch->update(true);
                }
                //addChild(dynamic_cast<EDProjectItem*>( EDPICSNode::createCSN( pOp->getArgument(nArg) )));
            }
        }
    }
}

EDPICSNode* EDPICSNode::createCSN( Operation *pOp )
{
    if (pOp == NULL) return new EDPICSNUndefined();
    switch (pOp->getType()) {
    case OP_DISTANCE	:	return new EDPICSNDistance(dynamic_cast<OpDistance*>(pOp));
    case OP_REITERATION	:	return new EDPICSNRepetition(dynamic_cast<OpReiteration*>(pOp));
    case OP_INTERVAL	:	return new EDPICSNInterval(dynamic_cast<OpInterval*>(pOp));
    case OP_TS			:	
        {	
            TS* pTS = dynamic_cast<TS*>(pOp);
            if (!pTS->isFromMarking())
                return new EDPICSNTSWord(pTS);
            else{
                return new EDPICSNTSMrkItem(pTS);
            }
        }
    }
    assert(0);
    return NULL;
}
bool EDPICSNode::isConnectedTo(void *pData) const{
    return ((void*) pOp) == pData;
}
const EDProcessedSignal* EDPICSNode::getProcessedSignal(ExpertDiscoveryData& edData){
    if (m_pPS == NULL) {
        m_pPS = EDProcessedSignal::processSignal(pOp, &edData.getPosSeqBase(), &edData.getNegSeqBase());
        if (m_pPS != NULL) {	
            QString strGenInfo = "General information";

            EDPIPropertyGroup GenInfo(strGenInfo);
            EDPIProperty prob("Probability");
            EDPIProperty posCoverage("Pos. coverage");
            EDPIProperty negCoverage("Neg. coverage");
            EDPIProperty fisher("Fisher");
            //EDPIProperty ul("ul");

            prob.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getProbability));
            posCoverage.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getPosCoverage));
            negCoverage.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getNegCoverage));
            fisher.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getFisher));
            //ul.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getUl));

            GenInfo.addProperty(prob);
            GenInfo.addProperty(posCoverage);
            GenInfo.addProperty(negCoverage);
            GenInfo.addProperty(fisher);
//            GenInfo.addProperty(ul);
            addGroup(GenInfo);
        }
    }
    return m_pPS;
}

QString EDPICSNode::getProbability() const{
    if (m_pPS)
        return m_pPS->getPropertyValue("Probability");
    return QString();
}
QString EDPICSNode::getFisher() const{
    if (m_pPS)
        return m_pPS->getPropertyValue("Fisher");
    return QString();
}
QString EDPICSNode::getUl() const{
    if (m_pPS)
        return m_pPS->getPropertyValue("Ul");
    return QString();
}
QString EDPICSNode::getPosCoverage() const{
    if (m_pPS)
        return m_pPS->getPropertyValue("Pos. coverage");
    return QString();
}
QString EDPICSNode::getNegCoverage() const{
    if (m_pPS)
        return m_pPS->getPropertyValue("Neg. coverage");
    return QString();
}
QString EDPICSNode::getTypeAsString() const{
    QString str;
    EDPIPropertyTypeList *pTList = EDPIPropertyTypeListCSNodeTypes::getInstance();
    switch (getType()) {
        case PIT_CSN_UNDEFINED: str = "Undefined"; break;
        case PIT_CSN_DISTANCE:  str = pTList->getValue(EDPIPropertyTypeListCSNodeTypes::DISTANCE); break;
        case PIT_CSN_REPETITION:str = pTList->getValue(EDPIPropertyTypeListCSNodeTypes::REPETITION); break;
        case PIT_CSN_INTERVAL:  str = pTList->getValue(EDPIPropertyTypeListCSNodeTypes::INTERVAL); break;
        case PIT_CSN_WORD:      str = pTList->getValue(EDPIPropertyTypeListCSNodeTypes::WORD); break;
        case PIT_CSN_MRK_ITEM:  str = pTList->getValue(EDPIPropertyTypeListCSNodeTypes::MRK_ITEM); break;
    }
    return str;
}

EDPICS::EDPICS(const Signal *pSignal)
: EDPICSNode(pSignal->getSignal())
, m_pSignal(pSignal)
{
    update(true);
}
EDPICS::~EDPICS(){
  pOp = NULL;  
}
EItemType EDPICS::getType() const{
    return PIT_CS;
}
const Signal* EDPICS::getSignal() const{
    return m_pSignal;
}

void EDPICS::update(bool bupdateChildren){
    clearGroups();

    QString strName = "Name";
    QString strEditor = "Editor";
    QString strDescription = "Description";
    QString strPriorParams = "Prior parameter";
    QString strPriorProb = "Probability";
    QString strPriorFisher = "Fisher criteria";
    QString strPriorPosCoverage = "Pos. Coverage";
    QString strPriorNegCoverage = "Neg. Coverage";
   

    EDPIProperty PropName(strName);
    PropName.setCallback(new Callback<const DDisc::Signal, const std::string>(m_pSignal, &DDisc::Signal::getName));
    PropName.setType(EDPIPropertyTypeString::getInstance());

    EDPIProperty PropDesc(strDescription);
    PropDesc.setCallback(new Callback<const DDisc::Signal, const std::string>(m_pSignal, &DDisc::Signal::getDescription));
    PropDesc.setType(EDPIPropertyTypeString::getInstance());

    EDPIPropertyGroup Editor(strEditor);
    Editor.addProperty(PropName);
    Editor.addProperty(PropDesc);
    addGroup(Editor);

    EDPICSNode::update(false);
    setName(m_pSignal->getName().c_str());

//     if (m_pSignal->isPriorParamsDefined()) {
//         EDPIProperty PropPriorProb(strPriorProb);
//         PropPriorProb.setCallback(new Callback<EDPICS, QString>(this, &EDPICS::getPriorProbability));
//         PropPriorProb.setType(EDPIPropertyTypeStaticString::getInstance());
// 
//         EDPIProperty PropPriorPosCoverage(strPriorPosCoverage);
//         PropPriorPosCoverage.setCallback(new Callback<EDPICS, QString>(this, &EDPICS::getPriorPosCoverage));
//         PropPriorPosCoverage.setType(EDPIPropertyTypeStaticString::getInstance());
// 
//         EDPIProperty PropPriorNegCoverage(strPriorNegCoverage);
//         PropPriorNegCoverage.setCallback(new Callback<EDPICS, QString>(this, &EDPICS::getPriorNegCoverage));
//         PropPriorNegCoverage.setType(EDPIPropertyTypeStaticString::getInstance());
// 
//         EDPIProperty PropPriorFisher(strPriorFisher);
//         PropPriorFisher.setCallback(new Callback<EDPICS, QString>(this, &EDPICS::getPriorFisher));
//         PropPriorFisher.setType(EDPIPropertyTypeStaticString::getInstance());
// 
//         EDPIPropertyGroup PriorParams(strPriorParams);
//         PriorParams.addProperty(PropPriorProb);
//         PriorParams.addProperty(PropPriorFisher);
//         PriorParams.addProperty(PropPriorPosCoverage);
//         PriorParams.addProperty(PropPriorNegCoverage);
//        addGroup(PriorParams);
//     }

    if (bupdateChildren) {
        takeChildren();
        if (m_pSignal->getSignal() == NULL) {
            addChild(dynamic_cast<EDProjectItem*>( new EDPICSNUndefined() ));
        }
        else {
            EDProjectItem* ch = dynamic_cast<EDProjectItem*>(EDPICSNode::createCSN(m_pSignal->getSignal()));
            emit si_getMetaInfoBase();
            ch->setMetainfoBase(getMinfo());
            addChild( ch );
            ch->update(true);
        }
    }
}
bool EDPICS::isConnectedTo(void *pData) const{
    return ((void*) m_pSignal) == pData;
}
//virtual CExtPopupMenuWnd*	CreatePopupMenu(HWND hWndCmdRecieve) const;
//virtual bool isSelected() const;

QString EDPICS::getPriorProbability() const{
    QString str = "%1";
    if (m_pSignal->getPriorProbability() != UNDEFINED_VALUE)
        str = str.arg(m_pSignal->getPriorProbability());
    else 
        str = "Undefined"; 
    return str;    
}
QString EDPICS::getPriorFisher() const{
    QString str = "%1";
    if (m_pSignal->getPriorFisher() != UNDEFINED_VALUE)
        str = str.arg(m_pSignal->getPriorFisher());
    else 
        str = "Undefined"; 
    return str;    
}
QString EDPICS::getPriorPosCoverage() const{
    QString str = "%1";
    if (m_pSignal->getPriorPosCoverage() != UNDEFINED_VALUE)
        str = str.arg(m_pSignal->getPriorPosCoverage());
    else 
        str = "Undefined"; 
    return str;    
}
QString EDPICS::getPriorNegCoverage() const{
    QString str = "%1";
    if (m_pSignal->getPriorNegCoverage() != UNDEFINED_VALUE)
        str = str.arg(m_pSignal->getPriorNegCoverage());
    else 
        str = "Undefined"; 
    return str;    
}
////////////////////////////////////////////////////////////

EDPICSNUndefined::EDPICSNUndefined()
: EDPICSNode(NULL)
{
    update(true);
}


void EDPICSNUndefined::update(bool bupdateChildren)
{
    QString strType = "Type";
    QString strEditor = "Editor";


    EDPIProperty PropType(strType);
    PropType.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getTypeAsString));
    PropType.setType(EDPIPropertyTypeListCSNodeTypes::getInstance());
    EDPIPropertyGroup Editor(strEditor);
    Editor.addProperty(PropType);
   addGroup(Editor);

    EDPICSNode::update(bupdateChildren);
}

EDPICSNUndefined::~EDPICSNUndefined()
{
}

EItemType EDPICSNUndefined::getType() const
{
    return PIT_CSN_UNDEFINED;
}

///////////////////////////////////////////////////////////////////////////////////////

EDPICSNDistance::EDPICSNDistance(OpDistance *pOp)
: EDPICSNode(pOp)
{
    update(true);
}	

void EDPICSNDistance::update(bool bupdateChildren) {
    clearGroups();

    QString strType = "Type";
    QString strFrom = "Distance from";
    QString strTo = "Distance to";
    QString strEditor = "Editor";
    QString strOrder = "Order";
    QString strDistanceType = "Distance type";


    OpDistance* pOp = dynamic_cast<OpDistance*>(getOperation());
    assert(pOp != NULL);

    EDPIPropertyTypeList *pTList = EDPIPropertyTypeListCSNodeTypes::getInstance();
    EDPIProperty PropType(strType);
    PropType.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getTypeAsString));
    PropType.setType(pTList);

    EDPIProperty PropFrom(strFrom);
    PropFrom.setCallback(new Callback<EDPICSNDistance, QString>(this, &EDPICSNDistance::getPropFrom));
    PropFrom.setType(EDPIPropertyTypeUnsignedInt::getInstance());

    EDPIProperty PropTo(strTo);
    PropTo.setCallback(new Callback<EDPICSNDistance, QString>(this, &EDPICSNDistance::getPropTo));
    PropTo.setType(EDPIPropertyTypeUnsignedIntWithUnl::getInstance());

    EDPIProperty PropOrder(strOrder);
    PropOrder.setType(EDPIPropertyTypeBool::getInstance());
    PropOrder.setCallback(new Callback<EDPICSNDistance, QString>(this, &EDPICSNDistance::getPropOrder));

    EDPIProperty PropDistType(strDistanceType);
    PropDistType.setType(EDPIPropertyTypeDistType::getInstance());
    PropDistType.setCallback(new Callback<EDPICSNDistance, QString>(this, &EDPICSNDistance::getPropDistType));

    EDPIPropertyGroup Editor(strEditor);
    Editor.addProperty(PropType);
    Editor.addProperty(PropDistType);
    Editor.addProperty(PropFrom);
    Editor.addProperty(PropTo);
    Editor.addProperty(PropOrder);
   addGroup(Editor);	

    emit si_getMetaInfoBase();

    EDPICSNode::update(bupdateChildren);
}

EDPICSNDistance::~EDPICSNDistance()
{
}

EItemType EDPICSNDistance::getType() const
{
    return PIT_CSN_DISTANCE;
}

QString EDPICSNDistance::getPropDistType() const
{
    const OpDistance* pOp = dynamic_cast<const OpDistance*>(getOperation());
    assert(pOp != NULL);
    int nType = (int) pOp->getDistanceType();
    return EDPIPropertyTypeDistType::getInstance()->getValue(nType);
}

QString EDPICSNDistance::getPropFrom() const
{
    const OpDistance* pOp = dynamic_cast<const OpDistance*>(getOperation());
    assert(pOp != NULL);
    return QString("%1").arg(pOp->getDistance().getFrom());
}

QString EDPICSNDistance::getPropTo() const
{
    const OpDistance* pOp = dynamic_cast<const OpDistance*>(getOperation());
    assert(pOp != NULL);
    int nTo = pOp->getDistance().getTo();
    if (nTo == PINF) 
        return EDPIPropertyTypeUnsignedIntWithUnl::getInstance()->getValue(0);
    else
        return QString("%1").arg(nTo);
}

QString EDPICSNDistance::getPropOrder() const
{
    const OpDistance* pOp = dynamic_cast<const OpDistance*>(getOperation());
    assert(pOp != NULL);
    bool bOrder = pOp->isOrderImportant();
    return EDPIPropertyTypeBool::getInstance()->getValue((!bOrder)?EDPIPropertyTypeBool::False : EDPIPropertyTypeBool::True);
}


///////////////////////////////////////////////////////////////////////////////////////

EDPICSNRepetition::EDPICSNRepetition(OpReiteration *pOp)
: EDPICSNode(pOp)
{
    update(true);
}

void EDPICSNRepetition::update(bool bupdateChildren)
{
    clearGroups();

    QString strType = "Type";
    QString strCountFrom = "Count from";
    QString strCountTo = "Count to";
    QString strFrom = "Distance from";
    QString strTo = "Distance to";
    QString strEditor = "Editor";
    QString strDistanceType = "Distance type";

    OpReiteration* pOp = dynamic_cast<OpReiteration*>(getOperation());
    assert(pOp != NULL);

    EDPIPropertyTypeList *pTList = EDPIPropertyTypeListCSNodeTypes::getInstance();
    EDPIProperty PropType(strType);
    PropType.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getTypeAsString));
    PropType.setType(pTList);

    EDPIProperty PropFrom(strFrom);
    PropFrom.setCallback(new Callback<EDPICSNRepetition, QString>(this, &EDPICSNRepetition::getPropFrom));
    PropFrom.setType(EDPIPropertyTypeUnsignedInt::getInstance());

    EDPIProperty PropTo(strTo);
    PropTo.setCallback(new Callback<EDPICSNRepetition, QString>(this, &EDPICSNRepetition::getPropTo));
    PropTo.setType(EDPIPropertyTypeUnsignedIntWithUnl::getInstance());

    EDPIProperty PropDistType(strDistanceType);
    PropDistType.setType(EDPIPropertyTypeDistType::getInstance());
    PropDistType.setCallback(new Callback<EDPICSNRepetition, QString>(this, &EDPICSNRepetition::getPropDistType));
    EDPIProperty PropCountFrom(strCountFrom);
    PropCountFrom.setType(EDPIPropertyTypeUnsignedInt::getInstance());
    PropCountFrom.setCallback(new Callback<EDPICSNRepetition, QString>(this, &EDPICSNRepetition::getCountFrom));

    EDPIProperty PropCountTo(strCountTo);
    PropCountTo.setType(EDPIPropertyTypeUnsignedInt::getInstance());
    PropCountTo.setCallback(new Callback<EDPICSNRepetition, QString>(this, &EDPICSNRepetition::getCountTo));

    EDPIPropertyGroup Editor(strEditor);
    Editor.addProperty(PropType);
    Editor.addProperty(PropCountFrom);
    Editor.addProperty(PropCountTo);
    Editor.addProperty(PropDistType);
    Editor.addProperty(PropFrom);
    Editor.addProperty(PropTo);
   addGroup(Editor);	

   emit si_getMetaInfoBase();

    EDPICSNode::update(bupdateChildren);
}

EDPICSNRepetition::~EDPICSNRepetition()
{
}

EItemType EDPICSNRepetition::getType() const
{
    return PIT_CSN_REPETITION;
}

QString EDPICSNRepetition::getPropDistType() const
{
    const OpReiteration* pOp = dynamic_cast<const OpReiteration*>(getOperation());
    assert(pOp != NULL);
    int nType = (int) pOp->getDistanceType();
    return EDPIPropertyTypeDistType::getInstance()->getValue(nType);
}

QString EDPICSNRepetition::getPropFrom() const
{
    const OpReiteration* pOp = dynamic_cast<const OpReiteration*>(getOperation());
    assert(pOp != NULL);
    return QString("%1").arg(pOp->getDistance().getFrom());
}

QString EDPICSNRepetition::getPropTo() const
{
    const OpReiteration* pOp = dynamic_cast<const OpReiteration*>(getOperation());
    assert(pOp != NULL);
    int nTo = pOp->getDistance().getTo();
    if (nTo == PINF) 
        return EDPIPropertyTypeUnsignedIntWithUnl::getInstance()->getValue(0);
    else
        return QString("%1").arg(nTo);
}

QString EDPICSNRepetition::getCountFrom() const
{
    const OpReiteration* pOp = dynamic_cast<const OpReiteration*>(getOperation());
    assert(pOp != NULL);
    return QString("%1").arg(pOp->getCount().getFrom());
}

QString EDPICSNRepetition::getCountTo() const
{
    const OpReiteration* pOp = dynamic_cast<const OpReiteration*>(getOperation());
    assert(pOp != NULL);
    return QString("%1").arg(pOp->getCount().getTo());
}


///////////////////////////////////////////////////////////////////////////////////////

EDPICSNInterval::EDPICSNInterval(OpInterval *pOp)
: EDPICSNode(pOp)
{
    update(true);
}

void EDPICSNInterval::update(bool bupdateChildren)
{
    clearGroups();

    QString strType = "Type";
    QString strFrom = "Distance from";
    QString strTo = "Distance to";
    QString strEditor = "Editor";

    OpInterval* pOp = dynamic_cast<OpInterval*>(getOperation());

    EDPIPropertyTypeList *pTList = EDPIPropertyTypeListCSNodeTypes::getInstance();
    EDPIProperty PropType(strType);
    PropType.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getTypeAsString));
    PropType.setType(pTList);

    EDPIProperty PropFrom(strFrom);
    PropFrom.setCallback(new Callback<EDPICSNInterval, QString>(this, &EDPICSNInterval::getPropFrom));
    PropFrom.setType(EDPIPropertyTypeUnsignedInt::getInstance());

    EDPIProperty PropTo(strTo);
    PropTo.setCallback(new Callback<EDPICSNInterval, QString>(this, &EDPICSNInterval::getPropTo));
    PropTo.setType(EDPIPropertyTypeUnsignedIntWithUnl::getInstance());

    EDPIPropertyGroup Editor(strEditor);
    Editor.addProperty(PropType);
    Editor.addProperty(PropFrom);
    Editor.addProperty(PropTo);
   addGroup(Editor);	

   emit si_getMetaInfoBase();

    EDPICSNode::update(bupdateChildren);

}

EDPICSNInterval::~EDPICSNInterval()
{
}

EItemType EDPICSNInterval::getType() const
{
    return PIT_CSN_INTERVAL;
}

QString EDPICSNInterval::getPropFrom() const
{
    const OpInterval* pOp = dynamic_cast<const OpInterval*>(getOperation());
    assert(pOp != NULL);
    return QString("%1").arg(pOp->getInt().getFrom());
}

QString EDPICSNInterval::getPropTo() const
{
    const OpInterval* pOp = dynamic_cast<const OpInterval*>(getOperation());
    assert(pOp != NULL);
    int nTo = pOp->getInt().getTo();
    if (nTo == PINF) 
        return EDPIPropertyTypeUnsignedIntWithUnl::getInstance()->getValue(0);
    else
        return QString("%1").arg(nTo);
}


///////////////////////////////////////////////////////////////////////////////////////

EDPICSNTSWord::EDPICSNTSWord(TS *pTS)
: EDPICSNode(pTS)
{
    update(true);
}

void EDPICSNTSWord::update(bool bupdateChildren) 
{
    clearGroups();

    QString strType = "Type";
    QString strEditor = "Editor";
    QString strWord = "Word";

    EDPIPropertyTypeList *pTList = EDPIPropertyTypeListCSNodeTypes::getInstance();
    EDPIProperty PropType(strType);
    PropType.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getTypeAsString));
    PropType.setType(pTList);

    EDPIProperty PropWord(strWord);
    PropWord.setType(EDPIPropertyTypeString::getInstance());
    PropWord.setCallback(new Callback<EDPICSNTSWord, QString>(this, &EDPICSNTSWord::getPropWord));

    EDPIPropertyGroup Editor(strEditor);
    Editor.addProperty(PropType);
    Editor.addProperty(PropWord);
   addGroup(Editor);	

    EDPICSNode::update(bupdateChildren);
}

EDPICSNTSWord::~EDPICSNTSWord()
{
}

EItemType EDPICSNTSWord::getType() const
{
    return PIT_CSN_WORD;
}

QString EDPICSNTSWord::getPropWord() const
{
    const TS *pTS = dynamic_cast<const TS*>(getOperation());
    assert(pTS != NULL);
    return pTS->getWord().c_str();
}

///////////////////////////////////////////////////////////////////////////////////////

EDPICSNTSMrkItem::EDPICSNTSMrkItem(TS *pTS)
: EDPICSNode(pTS)
{
    update(true);
}

void EDPICSNTSMrkItem::update(bool bupdateChildren) 
{
    clearGroups();

    QString strType = "Type";
    QString strEditor = "Editor";
    QString strFamily = "Family";
    QString strSignal = "Signal";

    TS *pTS = dynamic_cast<TS*>(getOperation());

    EDPIPropertyTypeList *pTList = EDPIPropertyTypeListCSNodeTypes::getInstance();
    EDPIProperty PropType(strType);
    PropType.setCallback(new Callback<EDPICSNode, QString>(this, &EDPICSNode::getTypeAsString));
    PropType.setType(pTList);

    emit si_getMetaInfoBase();

    if(getMinfo()){

        const MetaInfoBase& rDesc = *getMinfo();
        EDPIPropertyTypeDynamicList *pTFamily = EDPIPropertyTypeDynamicList::getInstance();
        int nFamilyNum = rDesc.getFamilyNumber();
        for (int i=0; i<nFamilyNum; i++)
            pTFamily->addValue(rDesc.getSignalFamily(i).getName().c_str());
        EDPIProperty PropFamily(strFamily);
        PropFamily.setType(pTFamily);
        PropFamily.setCallback(new Callback<EDPICSNTSMrkItem, QString>(this, &EDPICSNTSMrkItem::getPropFamily));

        EDPIPropertyTypeDynamicList *pTSignal = EDPIPropertyTypeDynamicList::getInstance();
        try {
            const Family& rFamily = rDesc.getSignalFamily(pTS->getFamily());
            int nSigNum = rFamily.getSignalNumber();
            for (int i=0;i<nSigNum; i++)
                pTSignal->addValue(rFamily.getMetaInfo(i).getName().c_str());
        }
        catch (exception) {

        }
        EDPIProperty PropSignal(strSignal);
        PropSignal.setType(pTSignal);
        PropSignal.setCallback(new Callback<EDPICSNTSMrkItem, QString>(this, &EDPICSNTSMrkItem::getPropSignal));

        EDPIPropertyGroup Editor(strEditor);
        Editor.addProperty(PropType);
        Editor.addProperty(PropFamily);
        Editor.addProperty(PropSignal);
        addGroup(Editor);	
    }

    EDPICSNode::update(bupdateChildren);
}

EDPICSNTSMrkItem::~EDPICSNTSMrkItem()
{
}

EItemType EDPICSNTSMrkItem::getType() const
{
    return PIT_CSN_MRK_ITEM;
}

QString EDPICSNTSMrkItem::getPropFamily() const
{
    const TS *pTS = dynamic_cast<const TS*>(getOperation());
    return pTS->getFamily().c_str();
}

QString EDPICSNTSMrkItem::getPropSignal() const
{
    const TS *pTS = dynamic_cast<const TS*>(getOperation());
    return pTS->getName().c_str();
}

///////////////////////////////////////////////////////////////////////////////////////

EDPIMrkRoot::EDPIMrkRoot()
: EDProjectItem()
{
    update(true);
}

EDPIMrkRoot::~EDPIMrkRoot()
{
}

EItemType EDPIMrkRoot::getType() const
{
    return PIT_MRK_ROOT;
}

void EDPIMrkRoot::update( bool bUpdateChildren)
{
    setName("Markup");
}

void EDPIMrkRoot::updMarkup(const ExpertDiscoveryData& d){
     takeChildren();
     const MetaInfoBase& rDesc = d.getDescriptionBase();
     int nFamilyNum = rDesc.getFamilyNumber();
     for (int i=0; i<nFamilyNum; i++){
        addChild( new EDPIMrkFamily(rDesc.getSignalFamily(i)) );
     }

}

bool EDPIMrkRoot::isConnectedTo(void *pData) const
{
    //return (pData == (void *)&GlobalgetDocument()->getDescriptionBase());
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////

EDPIMrkFamily::EDPIMrkFamily(const Family& rFamily)
: EDProjectItem()
, m_rFamily (rFamily)
{
    update(true);
}

EDPIMrkFamily::~EDPIMrkFamily()
{
}

EItemType EDPIMrkFamily::getType() const
{
    return PIT_MRK_FAMILY;
}

void EDPIMrkFamily::update(bool bupdateChildren)
{
    QString strName =  m_rFamily.getName().c_str();
    setName( strName );

    if (bupdateChildren) {
        takeChildren();
        int nSigNum = m_rFamily.getSignalNumber();
        for (int i=0; i<nSigNum; i++)
            addChild(dynamic_cast<EDProjectItem*>( new EDPIMrkItem(strName, m_rFamily.getMetaInfo(i)) ));
    }
}

bool EDPIMrkFamily::isConnectedTo(void *pData) const
{
    return (pData == (void *) &m_rFamily);
}

///////////////////////////////////////////////////////////////////////////////////////

EDPIMrkItem::EDPIMrkItem(QString strFamilyName, const MetaInfo& rMetaInfo)
: EDPICSNode(NULL)
, m_rMetaInfo(rMetaInfo)
{
    TS* pTS = new TS;
    pTS->setFromMarking(true);
    pTS->setName(m_rMetaInfo.getName());
    pTS->setFamily(strFamilyName.toStdString());
    pOp = pTS;
    update(true);
    setName(m_rMetaInfo.getName().c_str());
}

EDPIMrkItem::~EDPIMrkItem()
{
    delete pOp;
}

EItemType EDPIMrkItem::getType() const
{
    return PIT_MRK_ITEM;
}

void EDPIMrkItem::update(bool bupdateChildren)
{
    setName( m_rMetaInfo.getName().c_str() );
    EDPICSNode::update(true);
}

bool EDPIMrkItem::isConnectedTo(void *pData) const
{
    return (pData == (void *) &m_rMetaInfo);
}

} //namespace

