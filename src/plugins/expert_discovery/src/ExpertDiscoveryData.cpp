#include "ExpertDiscoveryData.h"
#include "ExpertDiscoverySetupRecBoundDialog.h"
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectTypes.h>

#include <QProgressDialog>
namespace U2 {

const std::string ExpertDiscoveryData::FAMILY_LETTERS = "_LETTERS_";
const std::string ExpertDiscoveryData::FAMILY_LETTERS_METHOD = "EXPERT_DISCOVERY_LETTERS_MARKUP_METHOD";

ExpertDiscoveryData::ExpertDiscoveryData (){
    recognizationBound = 0;
}

void ExpertDiscoveryData::setPosBase(const QList<GObject*> & objects){
	foreach(GObject* obj, objects){
		if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
			posBase.addSequence(prerareSequence(obj));
		}
	}
}
void ExpertDiscoveryData::setNegBase(const QList<GObject*> & objects){
	foreach(GObject* obj, objects){
		if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
			negBase.addSequence(prerareSequence(obj));
		}
	}
}

void ExpertDiscoveryData::setConBase(const QList<GObject*> & objects){
	foreach(GObject* obj, objects){
		if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
			conBase.addSequence(prerareSequence(obj));
		}
	}
}

Sequence ExpertDiscoveryData::prerareSequence(const GObject* obj) const{
	const QString& name  = obj->getGObjectName();
	std::string n = name.toStdString();

	DNASequenceObject* seq = (DNASequenceObject*)obj;
	const QByteArray& seqArr =  seq->getSequence();	
	std::string seqStr = std::string(seqArr.data(),seqArr.length());
	Sequence seqReady = Sequence(n, seqStr);

	return seqReady;
}

void ExpertDiscoveryData::clearContrBase(){
    conBase.clear();
}
void ExpertDiscoveryData::clearContrAnnot(){
    conAnn.clear();
}

void ExpertDiscoveryData::markupLetters(void){
	clearScores();
	// If already exist return
	if (isLettersMarkedUp())
		return;

	// Adding new signal family for letters
	std::string strFamilyName = ExpertDiscoveryData::FAMILY_LETTERS;
	std::string strMethodName = ExpertDiscoveryData::FAMILY_LETTERS_METHOD;
	
	const char letter[] = {'A','C','T','G','\0'};
	Family letters;
	letters.setName(strFamilyName);	

	for (int i=0; letter[i] != 0; i++) {
		MetaInfo mi;
		mi.setName(char2string(letter[i]));
		mi.setNo(i);
		mi.setMethodName(strMethodName);
		letters.AddInfo(mi);
	}

	desc.addFamily(letters);
	
	markupLetters(posBase, posAnn);
	markupLetters(negBase, negAnn);
	
	if (conBase.getSize() != 0)
		markupLetters(conBase, conAnn);

}
void ExpertDiscoveryData::markupLetters(SequenceBase& rBase, MarkingBase& rAnn){
	std::string strFamilyName = ExpertDiscoveryData::FAMILY_LETTERS;
	const char letter[] = {'A','C','T','G','\0'};
	int size = rBase.getSize();
	std::string seq;
	for (int i=0; i<size; i++) {
		seq = rBase.getSequence(i).getSequence();

		Marking mrk;
		try {
			mrk = rAnn.getMarking(i);			
		} 
		catch (exception) {
		}
		int len = (int)seq.size();
		for (int j=0; j<len; j++) {
			if (strchr(letter, seq[j]) != NULL)
				mrk.set(char2string(seq[j]), strFamilyName, Interval(j,j));
		}

		rAnn.setMarking(i, mrk);
	}
	rBase.setMarking(rAnn);
}
void ExpertDiscoveryData::clearScores(){
	posBase.clearScores();
	negBase.clearScores();
	conBase.clearScores();
}
bool ExpertDiscoveryData::updateScore(Sequence& rSeq){
    if (rSeq.isHasScore())
        return true;
    rSeq.setScore(0);
    rSeq.setHasScore(false);

    // Evaluation of sequence score
    double dScore = 0;
    
    RecognizationData data;
    if (!recDataStorage.getRecognizationData(data, &rSeq, selectedSignals))
        return false;
    for (int i=0; i<(int)data.size(); i++)
        dScore += data[i];

    rSeq.setScore(dScore);
    rSeq.setHasScore(true);
    return true;
}

void ExpertDiscoveryData::optimizeRecognizationBound(){
    double dPosScore = 0;
    for (int i=0; i<posBase.getSize(); i++)
    {
        Sequence& rSeq = posBase.getSequence(i);
        if (rSeq.isHasScore() || updateScore(rSeq))
        {
            dPosScore += rSeq.getScore();
        }
    }
    dPosScore /= posBase.getSize();

    double dNegScore = 0;
    for (int i=0; i<negBase.getSize(); i++)
    {
        Sequence& rSeq = negBase.getSequence(i);
        if (rSeq.isHasScore() || updateScore(rSeq))
        {
            dNegScore += rSeq.getScore();
        }
    }
    dNegScore /= negBase.getSize();
    recognizationBound = (dPosScore + dNegScore)/2;
}

void ExpertDiscoveryData::setRecBound(){
    if (!updateScores())
        return;
    vector<double> vPosScore = posBase.getScores();
    vector<double> vNegScore = negBase.getScores();
    /*CSetupRecBoundDlg dlg(m_dRecognizationBound, vPosScore, vNegScore, m_bLargeSequenceMode, m_nWindowSize);
    if (dlg.DoModal() == IDOK) {
        m_dRecognizationBound = dlg.GetRecognizationBound();
        m_bLargeSequenceMode = dlg.LargeSequenceMode();
        m_nWindowSize = dlg.GetWindowSize();
        m_ControlBase.clearScores();
        SetModifiedFlag();
        CDocument::UpdateAllViews(NULL, -1);
    }*/
    ExpertDiscoverySetupRecBoundDialog dlg(recognizationBound, vPosScore, vNegScore);
    if(dlg.exec()){
        recognizationBound = dlg.getRecognizationBound();
        conBase.clearScores();
    }
}
bool ExpertDiscoveryData::updateScores(){
 
    QProgressDialog pd(tr("Setting up recognization bound. Please wait"), tr("Cancel"), 0, 100);
    pd.setWindowModality(Qt::WindowModal);
    pd.show();

    pd.setLabelText(tr("Updating positive sequences"));
    pd.setValue(0);

    int sizeTotal = posBase.getSize() + negBase.getSize() + conBase.getSize();
    for (int i=0; i<posBase.getSize(); i++){
        if (pd.wasCanceled())
            return false;
        Sequence& rSeq = posBase.getSequence(i);
        if (!rSeq.isHasScore()){
            updateScore(rSeq);
        }
        pd.setValue((100*i)/sizeTotal);
    }

    pd.setLabelText(tr("Updating negative sequences"));
    for (int i=0; i<negBase.getSize(); i++)
    {
        if (pd.wasCanceled())
            return false;
        Sequence& rSeq = negBase.getSequence(i);
        if (!rSeq.isHasScore())
        {
            updateScore(rSeq);
        }
        pd.setValue((100*(i+posBase.getSize()))/sizeTotal);
    }
    
    pd.setLabelText(tr("Updating control sequences"));
    for (int i=0; i<conBase.getSize(); i++)
    {
        if (pd.wasCanceled())
            return false;
        Sequence& rSeq = conBase.getSequence(i);
        if (!rSeq.isHasScore())
        {
            updateScore(rSeq);
        }
        pd.setValue((100*(i+posBase.getSize() + negBase.getSize()))/sizeTotal);
    }

    return true;   
}

bool ExpertDiscoveryData::isLettersMarkedUp(void) const
{
	std::string family = ExpertDiscoveryData::FAMILY_LETTERS;
	try {
		desc.getSignalFamily(family);
	}
	catch (exception&) {
		return false;
	}

	return true;
}

std::string ExpertDiscoveryData::char2string(char ch) {
	char ar[] = {ch, 0};
	return std::string(ar);
}

void ExpertDiscoveryData::switchSelection(EDProjectItem* pItem, bool upd){
    EDPICS* pPICS = dynamic_cast<EDPICS*>(pItem);
    if (!pPICS)
        return;
    Signal* pSignal = const_cast<Signal*>(pPICS->getSignal());
    if (!pSignal->check())
        return;
    if (selectedSignals.IsSelected(pSignal))
        selectedSignals.RemoveSignal(pSignal);
    else {
        if (!pSignal->isPriorParamsDefined()) {
            //onSetCurrentSignalParamsAsPrior(pPICS, bUpdate);
        }

        selectedSignals.AddSignal(pSignal);
    }
    clearScores();

}

bool ExpertDiscoveryData::isSignalSelected(const EDProjectItem* pItem){
    const EDPICS* signal = dynamic_cast<const EDPICS*>(pItem);
    if(!signal){
        return false;
    }
    return selectedSignals.IsSelected(signal->getSignal());
}

void ExpertDiscoveryData::onSetCurrentSignalParamsAsPrior(EDPICS *pItem, bool bUpdate){
    Signal* pSignal = const_cast<Signal*>(pItem->getSignal());
    const EDProcessedSignal* pPS = pItem->getProcessedSignal(*this);
    if (pPS && pSignal)
    {
        pSignal->setPriorParamsDefined(true);
        pSignal->setPriorProbability(pPS->getProbability());
        pSignal->setPriorFisher(pPS->getFisher());
        pSignal->setPriorPosCoverage(pPS->getPosCoverage());
        pSignal->setPriorNegCoverage(pPS->getNegCoverage());
        pItem->update(false);
        if (bUpdate) {
            //UpdateAllViews(NULL, CURRENT_ITEM_CHANGED, pItem);
            clearScores();
        }
    }
}
void ExpertDiscoveryData::onClearSignalPriorParams(EDPICS *pItem){
    Signal* pSignal = const_cast<Signal*>(pItem->getSignal());
    if (pSignal)
    {
        pSignal->setPriorParamsDefined(false);
        pSignal->setPriorProbability(0);
        pSignal->setPriorFisher(1);
        pSignal->setPriorPosCoverage(0);
        pSignal->setPriorNegCoverage(0);
        pItem->update(false);
        //UpdateAllViews(NULL, CURRENT_ITEM_CHANGED, pItem);
        clearScores();
    }
}

}//namespace
