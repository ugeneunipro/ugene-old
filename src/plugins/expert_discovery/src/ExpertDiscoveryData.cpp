#include "ExpertDiscoveryData.h"
#include "ExpertDiscoverySetupRecBoundDialog.h"
#include "DDisc/Context.h"
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/AppContext.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2View/WebWindow.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>

#include <fstream>
#include <set>
#include <iomanip>

#include <QProgressDialog>
#include <QtGui/QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTime>
#include <QDomDocument>
#include <QtCore/qmath.h>

namespace U2 {

const std::string ExpertDiscoveryData::FAMILY_LETTERS = "_LETTERS_";
const std::string ExpertDiscoveryData::FAMILY_LETTERS_METHOD = "EXPERT_DISCOVERY_LETTERS_MARKUP_METHOD";
const QString ExpertDiscoveryData::FAMILY_ED_SIGNAL = "ExpertDiscovery_Signals";
const QString ExpertDiscoveryData::FAMILY_ED_METHOD = "EXPERT_DISCOVERY_METHOD";

ExpertDiscoveryData::ExpertDiscoveryData (){
    recognizationBound = 0;
    modified = false;
}

void ExpertDiscoveryData::setPosBase(const QList<GObject*> & objects){
    setBase(objects, posBase);
}
void ExpertDiscoveryData::setNegBase(const QList<GObject*> & objects){
    setBase(objects, negBase);
}

void ExpertDiscoveryData::setConBase(const QList<GObject*> & objects){
    setBase(objects, conBase);
}

inline Sequence ExpertDiscoveryData::prepareSequence( const GObject* obj ) const{
    U2SequenceObject* seq = (U2SequenceObject*)obj;
    QByteArray seqArr =  seq->getWholeSequenceData();
    std::string seqStr = std::string(seqArr.data(),seqArr.length());
    Sequence seqReady = Sequence(obj->getGObjectName().toStdString(), seqStr);

    return seqReady;
}
inline Sequence ExpertDiscoveryData::prepareSequence(MAlignmentRow& row) const{
    const QByteArray& seqArr = row.getCore();
    std::string seqStr = std::string(seqArr.data(),seqArr.length());
    Sequence seqReady = Sequence(row.getName().toStdString(), seqStr);

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

    setModifed();

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

void ExpertDiscoveryData::markupLetters(Sequence& edSeq){
    std::string strFamilyName = ExpertDiscoveryData::FAMILY_LETTERS;
    const char letter[] = {'A','C','T','G','\0'};
    std::string seq;
    seq = edSeq.getSequence();

    Marking mrk;
    int len = (int)seq.size();
    for (int j=0; j<len; j++) {
        if (strchr(letter, seq[j]) != NULL)
            mrk.set(char2string(seq[j]), strFamilyName, Interval(j,j));
    }
    edSeq.setSequenceMarking(mrk);
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

    setModifed();

    // Evaluation of sequence score
    double dScore = 0;

    U2OpStatus2Log st;
    RecognizationData data;
    if (!recDataStorage.getRecognizationData(data, &rSeq, selectedSignals, st))
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

    setModifed();

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
   
    ExpertDiscoverySetupRecBoundDialog dlg(recognizationBound, vPosScore, vNegScore);
    if(dlg.exec()){
        recognizationBound = dlg.getRecognizationBound();
        conBase.clearScores();
    }

    setModifed();
}
bool ExpertDiscoveryData::updateScores(){
 
    QProgressDialog pd(tr("Setting up recognition bound. Please wait"), tr("Cancel"), 0, 100);
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
//     for (int i=0; i<conBase.getSize(); i++)
//     {
//         if (pd.wasCanceled())
//             return false;
//         Sequence& rSeq = conBase.getSequence(i);
//         if (!rSeq.isHasScore())
//         {
//             updateScore(rSeq);
//         }
//         pd.setValue((100*(i+posBase.getSize() + negBase.getSize()))/sizeTotal);
//     }

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

void ExpertDiscoveryData::setBase(const QList<GObject*> &objects, SequenceBase& base){
    foreach(GObject* obj, objects){
        if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
            Sequence seq = prepareSequence(obj);
            seq.setHasScore(false);
            base.addSequence(seq);
            QString name=QString::fromStdString(seq.getName());
            recDataStorage.addSequence(name);
        }else if(obj->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT){
            MAlignmentObject* mobj =  qobject_cast<MAlignmentObject*>(obj);
            if(mobj){
                const MAlignment& ma = mobj->getMAlignment();
                const QList<MAlignmentRow>& rows = ma.getRows();
                foreach(MAlignmentRow row, rows){
                    Sequence seq = prepareSequence(row);
                    seq.setHasScore(false);
                    base.addSequence(seq);
                    QString name=QString::fromStdString(seq.getName());
                    recDataStorage.addSequence(name);   
                }
            }
        }
    }
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
            onSetCurrentSignalParamsAsPrior(pPICS, upd);
        }

        selectedSignals.AddSignal(pSignal);
    }
    clearScores();

    setModifed();
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
        setModifed();
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
        setModifed();
    }
}

SequenceType ExpertDiscoveryData::getSequenceTypeByName(const QString& seqName){
    if(posBase.getObjNo(seqName.toStdString().c_str()) != -1){
        return POSITIVE_SEQUENCE;
    }else if(negBase.getObjNo(seqName.toStdString().c_str()) != -1){
        return NEGATIVE_SEQUENCE;
    }else if(conBase.getObjNo(seqName.toStdString().c_str()) != -1){
        return CONTROL_SEQUENCE;
    }else{
        return UNKNOWN_SEQUENCE;
    }
}
int ExpertDiscoveryData::getSequenceIndex(const QString& seqName, SequenceType type){
    switch(type){
        case POSITIVE_SEQUENCE:
            return posBase.getObjNo(seqName.toStdString().c_str());
        case NEGATIVE_SEQUENCE:
            return negBase.getObjNo(seqName.toStdString().c_str());
        case CONTROL_SEQUENCE:
            return conBase.getObjNo(seqName.toStdString().c_str());
        default:
            return -1;
    }
}
RecognizationData ExpertDiscoveryData::getRecognitionData(int seqIndex, SequenceType type){
    RecognizationData d;

    SequenceBase base;
    switch(type){
        case POSITIVE_SEQUENCE:
            base = posBase;
            break;
        case NEGATIVE_SEQUENCE:
            base = negBase;
            break;
        case CONTROL_SEQUENCE:
            base = conBase;
            break;
        default:
            return d;
    }
    U2OpStatus2Log st;
    const Sequence& seq = base.getSequence(seqIndex);

    recDataStorage.getRecognizationData(d, &seq, selectedSignals, st);
    return d;
}

bool ExpertDiscoveryData::loadMarkup(const QString& firstF, const QString& secondF, const QString& thirdF, bool generateDescr){

    clearScores();
    posAnn.clear();
    negAnn.clear();
    desc.clear();
 
    QString strPosName = firstF;
    try {
        if (strPosName.right(4).compare(".xml", Qt::CaseInsensitive) == 0) {
            if (!loadAnnotation(posAnn, posBase, strPosName))
                throw std::exception();
        }
        else {
            ifstream fPosAnn(strPosName.toStdString().c_str());  
            posAnn.load(fPosAnn);
        }
     }
     catch (exception& ex) {
        posAnn.clear();
        QString str = "Positive annotation: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        return false;
     }

    QString strNegName = secondF;
    try {
        if (strPosName.right(4).compare(".xml", Qt::CaseInsensitive) == 0) {
            if (!loadAnnotation(negAnn, negBase, strNegName))
                throw std::exception();
        }
        else {
            ifstream fNegAnn(strNegName.toStdString().c_str());
            negAnn.load(fNegAnn);
        }
    }
    catch (exception& ex) {
        posAnn.clear();
        negAnn.clear();
        QString str = "Negative annotation: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        return false;
    }

    try {
        if (generateDescr) {
            if (!generateDescription())
                throw std::exception();
        }
        else {
            ifstream fDesc( thirdF.toStdString().c_str() );
            desc.load(fDesc);
        }
    }
    catch (exception& ex) {
        posAnn.clear();
        negAnn.clear();
        desc.clear();
        QString str = "Description: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        return false;
    }

    posBase.setMarking(posAnn);
    negBase.setMarking(negAnn);

    return true;
}

bool ExpertDiscoveryData::loadAnnotation(MarkingBase& base, const SequenceBase& seqBase, QString strFileName){

    QDomDocument pDoc;
    QFile xmlFile (strFileName);
    if(!xmlFile.open(QIODevice::ReadOnly)){
        return false;
    }
    if (!pDoc.setContent(&xmlFile)) {
        xmlFile.close();
        return false;
    }
    xmlFile.close();

    QDomElement pFamilies = pDoc.documentElement();
    if(pFamilies.tagName() != "markup"){
        return false;
    }

    QDomNode pFamilyNode = pFamilies.firstChild();
    while(!pFamilyNode.isNull()){
        QDomElement pFamily = pFamilyNode.toElement();
        if(pFamily.tagName() == "family"){
            QString familyName = pFamily.attribute("name");
            //family

            QDomNode pSignalNode = pFamily.firstChild();
            if(pSignalNode.toElement().tagName() != "signal"){
                return false;
            }
            while(!pSignalNode.isNull()){
                QDomElement pSignal = pSignalNode.toElement();
                if(pSignal.tagName() == "signal"){
                    QString signalName = pSignal.attribute("name");
                    //signal

                    QDomNode pSequenceNode = pSignal.firstChild();
                    if(pSequenceNode.toElement().tagName() != "sequence"){
                        return false;
                    }
                    while(!pSequenceNode.isNull()){
                        QDomElement pSequence = pSequenceNode.toElement();
                        if(pSequence.tagName() == "sequence"){
                            QString sequenceId = pSequence.attribute("id");
                            int cutPos = sequenceId.indexOf(">");
                            if(cutPos >= 0){
                                sequenceId = sequenceId.right(sequenceId.length() - cutPos - 1);
                            }
                            sequenceId = sequenceId.trimmed();
                            
                            //sequence

                            int objN = seqBase.getObjNo(sequenceId.toStdString().c_str());
                            if(objN >= 0){
                                QDomNode pInstanceNode = pSequence.firstChild();
                                if(pInstanceNode.toElement().tagName() != "instance"){
                                    return false;
                                }
                                Marking mrk;
                                try {
                                    mrk = base.getMarking(objN);
                                }
                                catch (...) {}
                                while(!pInstanceNode.isNull()){
                                    QDomElement pInstance = pInstanceNode.toElement();
                                    if(pInstance.tagName() == "instance"){
                                        int startPos = pInstance.attribute("start").toInt() - 1;
                                        int endPos = pInstance.attribute("end").toInt() - 1;
                                        if (endPos >= startPos && startPos >= 0) {
                                            mrk.set(signalName.toStdString(), familyName.toStdString(), DDisc::Interval(startPos, endPos));
                                        }
                                        //instance
                                    }
                                    pInstanceNode = pInstanceNode.nextSibling();
                                }
                                base.setMarking(objN, mrk);
                            }
                            
                        }
                        pSequenceNode = pSequenceNode.nextSibling();
                    }
                }
                pSignalNode = pSignalNode.nextSibling();
            }
        }
        pFamilyNode = pFamilyNode.nextSibling();
    }
    return true;  
       
}

bool ExpertDiscoveryData::generateDescription(bool clearDescr){
    if(clearDescr){
        desc.clear();
    }
    SequenceBase* seqBase = &posBase;
    MarkingBase* base = &posAnn; 
    for (int k=0; k<2; k++) {
        for (int i=0; i<seqBase->getSize(); i++) {
            try {
                Marking mrk = base->getMarking(i);
                set<std::string> families = mrk.getFamilies();
                set<std::string>::iterator i = families.begin();
                while (i != families.end()) {
                    set<std::string> edsignals = mrk.getSignals(*i);
                    set<std::string>::iterator j = edsignals.begin();
                    while (j != edsignals.end()) {
                        MetaInfo mi;
                        mi.setName(*j);
                        mi.setMethodName("Generated");
                        desc.insert((*i).c_str(), mi);
                        j++;
                    }
                    i++;
                }
            }
            catch (...) {}
        }
        seqBase = &negBase;
        base = &negAnn;
    }
    return true;
}

void ExpertDiscoveryData::loadControlSequenceAnnotation(const QString& fileName){
    ifstream in(fileName.toStdString().c_str());
    if (!in.is_open()) {
        QMessageBox mb(QMessageBox::Critical, tr("Error"), "Can't open file");
        mb.exec();
    }
    try {
        conAnn.load(in);
        conBase.setMarking(conAnn);
    }
    catch (exception& ) {
        conBase.clearMarking();
        conAnn.clear();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), "Error loading control markup");
        mb.exec();
    }
    if (isLettersMarkedUp() && conBase.getSize() != 0)
        markupLetters(conBase, conAnn);
}

void ExpertDiscoveryData::cleanup(){
    recDataStorage.clear();
    selectedSignals.Clear();

    posBase.clear();
    negBase.clear();
    conBase.clear();

    fileNamesMap.clear();

    desc.clear();
    posAnn.clear();
    negAnn.clear();
    conAnn.clear();

    rootFolder.clear();

    clearSelectedSequencesList();
}

void ExpertDiscoveryData::addSequenceToSelected(EDPISequence* seq){
    selSequences.append(seq);
}
void ExpertDiscoveryData::clearSelectedSequencesList(){
    selSequences.clear();
}
bool ExpertDiscoveryData::isSequenceSelected(EDPISequence* seq){
    return selSequences.contains(seq);
}

QList<EDPISequence*> ExpertDiscoveryData::getSelectetSequencesList(){
    return selSequences;
}

void ExpertDiscoveryData::generateRecognitionReportFull(){
    QFileDialog saveRepDialog;
    saveRepDialog.setFileMode(QFileDialog::AnyFile);
    saveRepDialog.setNameFilter(tr("Hypertext files (*.htm *.html)"));
    saveRepDialog.setViewMode(QFileDialog::Detail);
    saveRepDialog.setAcceptMode(QFileDialog::AcceptSave);

    if(saveRepDialog.exec()){
        QStringList fileNames = saveRepDialog.selectedFiles();
        if(fileNames.isEmpty()) return;

        QString fileName = fileNames.first();
        fileName = fileName+".htm";
        ofstream out(fileName.toStdString().c_str());
        if(!updateScores()){
            return;
        }
        if(!out.is_open()){
            QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Report generation failed"));
            mb.exec();
            return;
        }
        QString resultText;

        if(!generateRecognizationReportHeader(resultText) ||
           !generateRecognizationReport(posBase, "Positive", false, resultText) ||
           !generateRecognizationReport(negBase, "Negative", true, resultText) ||
           (conBase.getSize() != 0 && !generateRecognizationReport(conBase, "Control", true, resultText)) ||
           !generateRecognizationReportSignals(resultText) ||
           !generateRecognizationReportFooter(resultText))
        {
            QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Report generation failed"));
            mb.exec();
            return;
        }else{
            out<<resultText.toStdString();
            QString title;
            WebWindow* w = new WebWindow(title, resultText);
            w->setWindowIcon(QIcon(":core/images/chart_bar.png"));
            AppContext::getMainWindow()->getMDIManager()->addMDIWindow(w);
        }

    }
}
bool ExpertDiscoveryData::generateRecognizationReportHeader(QString& resultText ) const
{
    resultText.append("<HTML><HEAD><TITLE> UGENE (ExpertDiscovery plugin): Recognition report</TITLE></HEAD><BODY>\n");
    resultText.append("<H1>UGENE (ExpertDiscovery plugin): Recognition report</H1><BR>\n");
    resultText.append("<I>Report generated at ");
    resultText.append(QDateTime::currentDateTime().toString("hh:mm on dd/MM/yyyy"));

    resultText.append(QString("\n<BR><BR>Recognition bound was set to %1\n").arg(recognizationBound));
    resultText.append("<BR><BR>The recognition result for each sequence provide the foloving details:\n");
    resultText.append("<BR>Score - the more the score the more signals are found in the sequence\n");
    resultText.append("<BR>Recognized - the sequence is recognized if its score higher than the recognition bound\n");
    resultText.append("<BR>FP_Learning - S(negative)/count(negative)\n");
    resultText.append("<BR>FP_Control - S(control)/count(countrol)\n");
    resultText.append("<BR>     S - amount of sequences in a sequence base having a score higher than the current sequence score\n");
    resultText.append("<BR>     count - amount of sequences in a sequence base\n");
    resultText.append("</I><BR><BR><BR>\n");
    return true;
}
bool ExpertDiscoveryData::generateRecognizationReportFooter(QString& resultText) const{
    resultText.append("</BODY></HTML>\n");
    return true;
}
bool ExpertDiscoveryData::generateRecognizationReportSignals(QString& resultText) const{
    const SignalList& rSelList = selectedSignals.GetSelectedSignals();
    if (rSelList.size() == 0)
        return true;

    resultText.append("<BR><H2>Selected signals</H2><BR>");
    resultText.append(QString("Total signals selected <I>%1</I><BR>").arg(selectedSignals.GetSelectedSignals().size()));

    resultText.append("Details: <BR>");
    resultText.append("<TABLE border=1>");
    resultText.append("<TR align=center><TD>Signal No</TD><TD>Signal Name</TD><TD>Positive Coverage</TD><TD>Probability</TD><TD>Fisher</TD></TR>\n");
    SignalList::const_iterator iter = rSelList.begin();
    int i = 0;
    while (iter != rSelList.end()) {
        const Signal* pSignal = (*iter);
        resultText.append(QString("<TD>%1</TD>").arg(i+1));
        resultText.append(QString("<TD>%1</TD>").arg(QString::fromStdString(pSignal->getName())));
        resultText.append(QString("<TD>%1</TD>").arg(pSignal->getPriorPosCoverage()/100));
        resultText.append(QString("<TD>%1</TD>").arg(pSignal->getPriorProbability()));
        resultText.append(QString("<TD>%1</TD>").arg(pSignal->getPriorFisher()));
        i++;
        iter++;
    }
    resultText.append("</TABLE><BR>");
    return true;
}
bool ExpertDiscoveryData::generateRecognizationReport(const SequenceBase& rBase, QString strName, bool bSuppressNulls, QString& resultText){

    if (&rBase == &posBase){
        return generateRecognizationReportPositive(strName, bSuppressNulls, resultText);
    }

    if(rBase.getSize() == 0){
        return true;
    }

    int nRecognized = 0;
    int nNulls = 0;
    for (int i=0; i<rBase.getSize(); i++)
    {
        Sequence& rSeq = const_cast<Sequence&>(rBase.getSequence(i));
        updateScore(rSeq);
        double dScore = rSeq.getScore();
        if (dScore >= recognizationBound) nRecognized++;
        if (dScore == 0) nNulls++;
    }
    resultText.append("<BR><H2>");
    resultText.append(strName);
    resultText.append(" base</H2><BR>");
    resultText.append(QString("Total sequences: <I>%1</I><BR>").arg(rBase.getSize()));
    resultText.append(QString("Recognized sequences: <I>%1</I><BR>").arg(nRecognized));

    if (bSuppressNulls){
        resultText.append(QString("Sequences with zero score (NOT SHOWN): <I>%1</I><BR>").arg(nNulls));
    }

    if(fileNamesMap.find(&rBase) != fileNamesMap.end()){
        std::map<const SequenceBase*, std::string>::iterator it;
        it = fileNamesMap.find(&rBase);
        QString filename = QString::fromStdString(it->second);
        if(filename != ""){
            resultText.append("<BR> The sequence base was loaded from the file: ");
            resultText.append(filename);
            resultText.append("<BR>\n");
        }
    }

    resultText.append("Details: <BR>");
    resultText.append("<TABLE border=1>");
    resultText.append("<TR align=center><TD>Sequence No</TD><TD>Sequence Name</TD><TD>Score</TD><TD>Result</TD></TR>\n");

    for (int i=0; i<rBase.getSize(); i++)
    {
        const Sequence& rSeq = rBase.getSequence(i);
        if (bSuppressNulls && rSeq.getScore()==0) continue;
        const char* result = (rSeq.getScore() >= recognizationBound)?"Recognized":"Not recognized";
        resultText.append(QString("<TR align=center><TD>%1</TD>").arg(i+1));
        resultText.append(QString("<TD>%1</TD>").arg(QString::fromStdString(rSeq.getName())));
        resultText.append(QString("<TD>%1</TD>").arg(rSeq.getScore()));
        resultText.append(QString("<TD>%1</TD></TR>\n").arg(result));
    }

    resultText.append("</TABLE><BR>");
    return true;
}

bool ExpertDiscoveryData::generateRecognizationReportPositive(QString strName, bool bSuppressNulls, QString& resultText){
    const SequenceBase& rBase = posBase;
   
    int nRecognized = 0;
    int nNulls = 0;
    for (int i=0; i<rBase.getSize(); i++)
    {
        Sequence& rSeq = const_cast<Sequence&>(rBase.getSequence(i));
        updateScore(rSeq);
        double dScore = rSeq.getScore();
        if (dScore >= recognizationBound) nRecognized++;
        if (dScore == 0) nNulls++;
    }
    resultText.append("<BR><H2>");
    resultText.append(strName);
    resultText.append(" base</H2><BR>");
    resultText.append(QString("Total sequences: <I>%1</I><BR>").arg(rBase.getSize()));
    resultText.append(QString("Recognized sequences: <I>%1</I><BR>").arg(nRecognized));

    if (bSuppressNulls){
        resultText.append(QString("Sequences with zero score: <I>%1</I><BR>").arg(nNulls));
    }

    if(fileNamesMap.find(&rBase) != fileNamesMap.end()){
        std::map<const SequenceBase*, std::string>::iterator it;
        it = fileNamesMap.find(&rBase);
        QString filename = QString::fromStdString(it->second);
        if(filename != ""){
            resultText.append("<BR> The sequence base was loaded from the file: ");
            resultText.append(filename);
            resultText.append("<BR>\n");
        }
    }

    resultText.append("Details: <BR>");
    resultText.append("<TABLE border=1>");
    resultText.append("<TR align=center><TD>Sequence No</TD><TD>Sequence Name</TD><TD>Score</TD><TD>Result</TD><TD>FP_Learning</TD><TD>FP_Control</TD></TR>\n");

    for (int i=0; i<rBase.getSize(); i++)
    {
        const Sequence& rSeq = rBase.getSequence(i);
        if (bSuppressNulls && rSeq.getScore()==0) continue;
        double fp_control = getSequencesCountWithScoreMoreThan(rSeq.getScore(), conBase) / (double) conBase.getSize();
        double fp_learning = getSequencesCountWithScoreMoreThan(rSeq.getScore(), negBase) / (double) negBase.getSize();

        const char* result = (rSeq.getScore() >= recognizationBound)?"Recognized":"Not recognized";
        resultText.append(QString("<TR align=center><TD>%1</TD>").arg(i+1));
        resultText.append(QString("<TD>%1</TD>").arg(QString::fromStdString(rSeq.getName())));
        resultText.append(QString("<TD>%1</TD>").arg(rSeq.getScore()));
        resultText.append(QString("<TD>%1</TD>").arg(result));
        resultText.append(QString("<TD>%1</TD>").arg(fp_learning));
        resultText.append(QString("<TD>%1</TD></TR>\n").arg(fp_control));
    }

    resultText.append("</TABLE><BR>");
    return true;
}

int ExpertDiscoveryData::getSequencesCountWithScoreMoreThan(double dScore, const SequenceBase& rBase) const{
    int result = 0;
    for (int i=0; i<rBase.getSize(); i++) {
        const Sequence& rSeq = rBase.getSequence(i);
        if (rSeq.getScore() >= dScore)
            result++;
    }
    return result;
}

void ExpertDiscoveryData::generateRecognizationReport(EDProjectItem* pItem){
    EDPISequenceBase* pBase = dynamic_cast<EDPISequenceBase*>(pItem);
    if (!pBase)
    {
        assert(0);
        return;
    }

    QFileDialog saveRepDialog;
    saveRepDialog.setFileMode(QFileDialog::AnyFile);
    saveRepDialog.setNameFilter(tr("Hypertext files (*.htm *.html)"));
    saveRepDialog.setViewMode(QFileDialog::Detail);
    saveRepDialog.setAcceptMode(QFileDialog::AcceptSave);

    if(saveRepDialog.exec()){
        QStringList fileNames = saveRepDialog.selectedFiles();
        if(fileNames.isEmpty()) return;

        QString fileName = fileNames.first();
        fileName = fileName+".htm";
        ofstream out(fileName.toStdString().c_str());
        if(!updateScores()){
            return;
        }
        if(!out.is_open()){
            QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Report generation failed"));
            mb.exec();
            return;
        }

        QString resultText;
        if(!generateRecognizationReportHeader(resultText) ||
            !generateRecognizationReport(pBase->getSequenceBase(), pBase->getName(), true, resultText) ||
            !generateRecognizationReportFooter(resultText))
        {
            QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Report generation failed"));
            mb.exec();
            return;
        }else{
            out<<resultText.toStdString();
            QString title;
            WebWindow* w = new WebWindow(title, resultText);
            w->setWindowIcon(QIcon(":core/images/chart_bar.png"));
            AppContext::getMainWindow()->getMDIManager()->addMDIWindow(w);
        }

    }
}

void ExpertDiscoveryData::setBaseFilename(const SequenceBase& base, const QString& fileName){
    fileNamesMap[&base] = fileName.toStdString();
}

int ExpertDiscoveryData::getMaxPosSequenceLen(){
    int maxLen = 0;
    int curLen = 0;

    for (int i = 0; i < posBase.getSize(); i++){
        curLen = posBase.getSequence(i).getSize();
        if(curLen > maxLen){
            maxLen = curLen;
        }
    }

    return maxLen;
}

float ExpertDiscoveryData::calculateSequenceScore(const char* seq, int seqLen, ExpertDiscoveryData& edData, DNATranslation* complTT){
    
    Sequence edSequence;
    if(complTT != NULL){
        QByteArray revComplDna(seqLen, 0);    
        complTT->translate(seq, seqLen, revComplDna.data(), seqLen);
        TextUtils::reverse(revComplDna.data(), revComplDna.size());
        edSequence.setSequence(revComplDna.data());
    }else{
        edSequence.setSequence(std::string(seq, seqLen));
    }

    Marking mrk;
    if(edData.isLettersMarkedUp()){
        std::string strFamilyName = ExpertDiscoveryData::FAMILY_LETTERS;
        const char letter[] = {'A','C','T','G','\0'};
        std::string seq;
        seq = edSequence.getSequence();

        int len = (int)seq.size();
        for (int j=0; j<len; j++) {
            if (strchr(letter, seq[j]) != NULL)
                mrk.set(char2string(seq[j]), strFamilyName, Interval(j,j));
        }
        edSequence.setSequenceMarking(mrk);
    }
    
    const SignalList& rSelList = edData.getSelectedSignalsContainer().GetSelectedSignals();
    int listSize = rSelList.size();
    if (listSize == 0){
        return 0;
    }

    RecognizationData data;
    data.resize(seqLen);
    fill(data.begin(), data.end(), 0);
 
    SignalList::const_iterator iter = rSelList.begin();
    int it = 0;
    while (iter != rSelList.end()) {
        const Signal* pSignal = (*iter);
        DDisc::Context& context = pSignal->createCompartibleContext();
        while (pSignal->find(edSequence,context)) {
            double t = pSignal->getPriorProbability()/100;
            if (t>=1) t = 0.999999;
            int nPos = context.getPosition();
            int not_null_length = 0;
            double value = -qLn(1-t);
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
        it++;
    }
    
    float score = 0;
    for (int i = 0; i < seqLen; i++){
        score+=data[i];
    }

    return score;
}

}//namespace
