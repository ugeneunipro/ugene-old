#include "ExpertDiscoveryData.h"
#include "ExpertDiscoverySetupRecBoundDialog.h"
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectTypes.h>

#include <fstream>
#include <set>
#include <iomanip>

#include <QProgressDialog>
#include <QtGui/QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTime>
#include <QDomDocument>

namespace U2 {

const std::string ExpertDiscoveryData::FAMILY_LETTERS = "_LETTERS_";
const std::string ExpertDiscoveryData::FAMILY_LETTERS_METHOD = "EXPERT_DISCOVERY_LETTERS_MARKUP_METHOD";

ExpertDiscoveryData::ExpertDiscoveryData (){
    recognizationBound = 0;
    modified = false;
}

void ExpertDiscoveryData::setPosBase(const QList<GObject*> & objects){
    foreach(GObject* obj, objects){
        if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
            Sequence seq = prerareSequence(obj);
            seq.setHasScore(false);
            posBase.addSequence(seq);
            QString name=QString::fromStdString(seq.getName());
            recDataStorage.addSequence(name);
        }
    }
}
void ExpertDiscoveryData::setNegBase(const QList<GObject*> & objects){
    foreach(GObject* obj, objects){
        if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
            Sequence seq = prerareSequence(obj);
            seq.setHasScore(false);
            negBase.addSequence(seq);
            QString name=QString::fromStdString(seq.getName());
            recDataStorage.addSequence(name);
        }
    }
}

void ExpertDiscoveryData::setConBase(const QList<GObject*> & objects){
    foreach(GObject* obj, objects){
        if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
            Sequence seq = prerareSequence(obj);
            seq.setHasScore(false);
            conBase.addSequence(seq);
            QString name=QString::fromStdString(seq.getName());
            recDataStorage.addSequence(name);
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

// bool ExpertDiscoveryData::loadAnnotationFromUgeneDocument(MarkingBase& base, const SequenceBase& seqBase, Document* doc){
// 
//     foreach(GObject* obj, doc->getObjects()){
//         GObject* objk1 = obj;
//     }
// //     int objN = seqBase.getObjNo(sequenceId.toStdString().c_str());
// //     if(objN >= 0){
// //         QDomNode pInstanceNode = pSequence.firstChild();
// //         if(pInstanceNode.toElement().tagName() != "instance"){
// //             return false;
// //         }
// //         Marking mrk;
// //         try {
// //             mrk = base.getMarking(objN);
// //         }
// //         catch (...) {}
// //         while(!pInstanceNode.isNull()){
// //             QDomElement pInstance = pInstanceNode.toElement();
// //             if(pInstance.tagName() == "instance"){
// //                 int startPos = pInstance.attribute("start").toInt() - 1;
// //                 int endPos = pInstance.attribute("end").toInt() - 1;
// //                 if (endPos >= startPos && startPos >= 0) {
// //                     mrk.set(signalName.toStdString(), familyName.toStdString(), DDisc::Interval(startPos, endPos));
// //                 }
// //                 //instance
// //             }
// //             pInstanceNode = pInstanceNode.nextSibling();
// //         }
// //         base.setMarking(objN, mrk);
// //     }
//     return true;
// }

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

        if(!generateRecognizationReportHeader(out) ||
           !generateRecognizationReport(out, posBase, "Positive", false) ||
           !generateRecognizationReport(out, negBase, "Negative", true) ||
           (conBase.getSize() != 0 && !generateRecognizationReport(out, conBase, "Control", true)) ||
           !generateRecognizationReportFooter(out))
        {
            QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Report generation failed"));
            mb.exec();
            return;
        }

    }
}
bool ExpertDiscoveryData::generateRecognizationReportHeader(ostream& out) const{
    
    out << "<HTML><HEAD><TITLE> UGENE (ExpertDiscovery plugin): Recognition report</TITLE></HEAD><BODY>" << endl
        << "<H1>ExpertDiscovery 2.0 and UGENE: Recognization report</H1><BR>" << endl
        << "<I>Report genrated at " << (QDateTime::currentDateTime().toString("hh:mm on dd/MM/yyyy").toStdString())
        << "<BR>Recognization bound was set to " << recognizationBound
        << "</I><BR><BR><BR>" << endl;
    return true;
}
bool ExpertDiscoveryData::generateRecognizationReportFooter(ostream& out) const{
    out << "</BODY></HTML>";
    return true;
}
bool ExpertDiscoveryData::generateRecognizationReport(ostream& out, const SequenceBase& rBase, QString strName, bool bSuppressNulls){

    if (&rBase == &posBase)
    {
        return generateRecognizationReportPositive(out, strName, bSuppressNulls);
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
        if (dScore > recognizationBound) nRecognized++;
        if (dScore == 0) nNulls++;
    }
    out << "<BR><H2>" << strName.toStdString() << " base</H2><BR>"
        << "Total sequences: <I>" << rBase.getSize() << "</I><BR>"
        << "Recognized sequences: <I>" << nRecognized << "</I><BR>";

    if (bSuppressNulls)
        out << "Sequences with zero score: <I>" << nNulls << "</I><BR>";

    out    << "Details: <BR>"
        << "<TABLE border=1>"
        << "<TR align=center><TD>Sequence No</TD><TD>Sequence Name</TD><TD>Score</TD><TD>Result</TD></TR>" << endl;

    for (int i=0; i<rBase.getSize(); i++)
    {
        const Sequence& rSeq = rBase.getSequence(i);
        if (bSuppressNulls && rSeq.getScore()==0) continue;
        const char* result = (rSeq.getScore() >= recognizationBound)?"Recognized":"Not recognized";
        out << "<TR align=center><TD>" << i+1 << "</TD>"
            << "<TD>" << rSeq.getName() << "</TD>"
            << "<TD>" << rSeq.getScore() << "</TD>"
            << "<TD>" << result << "</TD></TR>" << endl;
    }

    out << "</TABLE><BR>";
    return true;
}

bool ExpertDiscoveryData::generateRecognizationReportPositive(ostream& out, QString strName, bool bSuppressNulls){
    const SequenceBase& rBase = posBase;
   
    int nRecognized = 0;
    int nNulls = 0;
    for (int i=0; i<rBase.getSize(); i++)
    {
        Sequence& rSeq = const_cast<Sequence&>(rBase.getSequence(i));
        updateScore(rSeq);
        double dScore = rSeq.getScore();
        if (dScore > recognizationBound) nRecognized++;
        if (dScore == 0) nNulls++;
    }
    out << "<BR><H2>" << strName.toStdString() << " base</H2><BR>"
        << "Total sequences: <I>" << rBase.getSize() << "</I><BR>"
        << "Recognized sequences: <I>" << nRecognized << "</I><BR>";

    if (bSuppressNulls)
        out << "Sequences with zero score: <I>" << nNulls << "</I><BR>";

    out    << "Details: <BR>"
        << "<TABLE border=1>"
        << "<TR align=center><TD>Sequence No</TD><TD>Sequence Name</TD><TD>Score</TD><TD>Result</TD><TD>FP_Learning</TD><TD>FP_Control</TD></TR>" << endl;

    for (int i=0; i<rBase.getSize(); i++)
    {
        const Sequence& rSeq = rBase.getSequence(i);
        if (bSuppressNulls && rSeq.getScore()==0) continue;
        double fp_control = getSequencesCountWithScoreMoreThan(rSeq.getScore(), conBase) / (double) conBase.getSize();
        double fp_learning = getSequencesCountWithScoreMoreThan(rSeq.getScore(), negBase) / (double) negBase.getSize();

        const char* result = (rSeq.getScore() >= recognizationBound)?"Recognized":"Not recognized";
        out << "<TR align=center><TD>" << i+1 << "</TD>"
            << "<TD>" << rSeq.getName() << "</TD>"
            << "<TD>" << rSeq.getScore() << "</TD>"
            << "<TD>" << result << "</TD>"
            << "<TD>" << setiosflags(ios::scientific) << fp_learning << "</TD>"
            << "<TD>" << fp_control << resetiosflags(ios::scientific) << "</TD></TR>" << endl;
    }

    out << "</TABLE><BR>";
    return true;
}

int ExpertDiscoveryData::getSequencesCountWithScoreMoreThan(double dScore, const SequenceBase& rBase) const{
    int result = 0;
    for (int i=0; i<rBase.getSize(); i++) {
        const Sequence& rSeq = rBase.getSequence(i);
        if (rSeq.getScore() > dScore)
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

        if(!generateRecognizationReportHeader(out) ||
            !generateRecognizationReport(out, pBase->getSequenceBase(), pBase->getName(), true) ||
            !generateRecognizationReportFooter(out))
        {
            QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Report generation failed"));
            mb.exec();
            return;
        }

    }
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

}//namespace
