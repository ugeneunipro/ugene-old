/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "SNPEffectTests.h"

#define FILE_NAME "file_name"
#define CHR_NAME "chr_name"
#define ALLELE "allele"
#define POS "pos"
#define GENE_ID "gene_id"
#define REPORT_TYPE "report_type"
#define COLUMN_NAME "column_name"
#define EXPECTED_VALUE "expected_value"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//GTest_FindInSNPResultTable
static QString getTempDir(const GTestEnvironment* env) {
    QString result = env->getVar("TEMP_DATA_DIR");
    if (result.isEmpty()) {
        result = QCoreApplication::applicationDirPath();
    }
    return result;
}
    
void GTest_FindInSNPResultTable::init(XMLTestFormat *tf, const QDomElement& el){
    Q_UNUSED(tf);

    chrName.clear();
    allele.clear();
    pos = 0;
    geneId.clear();
    fileName.clear();
    columnName.clear();
    expectedValue.clear();
    resTable.clear();


    QString tmp = el.attribute(FILE_NAME);
    if(tmp.isEmpty()){
        failMissingValue(FILE_NAME);
        return;
    }
    fileName = getTempDir(env) + "/" + tmp;

    tmp = el.attribute(CHR_NAME);
    if(tmp.isEmpty()){
        failMissingValue(CHR_NAME);
        return;
    }
    chrName = tmp;

    tmp = el.attribute(ALLELE);
    if(tmp.isEmpty()){
        failMissingValue(ALLELE);
        return;
    }
    allele = tmp;
    
    tmp = el.attribute(GENE_ID);
    if(tmp.isEmpty()){
        failMissingValue(GENE_ID);
        return;
    }
    geneId = tmp;

    tmp = el.attribute(POS);
    if(tmp.isEmpty()){
        failMissingValue(POS);
        return;
    }
    bool ok = false;
    pos = tmp.toInt(&ok);
    if (!ok){
        stateInfo.setError(QString("Not an integer attribute '%1': %2").arg(POS).arg(tmp));
        return;
    }

    tmp = el.attribute(REPORT_TYPE);
    if(tmp.isEmpty()){
        failMissingValue(REPORT_TYPE);
        return;
    }
    if (tmp == "ingene"){
        resultType = SNPResultInGene;
    }else if (tmp == "regulatory"){
        resultType = SNPResultRegulatory;
    }else{
        stateInfo.setError(QString("Unknown result type '%1': %2. Expected values: ingene or regulatory").arg(REPORT_TYPE).arg(tmp));
        return;
    }

    tmp = el.attribute(COLUMN_NAME);
    if(tmp.isEmpty()){
        failMissingValue(COLUMN_NAME);
        return;
    }
    columnName = tmp;

    tmp = el.attribute(EXPECTED_VALUE);
    if(tmp.isEmpty()){
        failMissingValue(EXPECTED_VALUE);
        return;
    }
    expectedValue = tmp;

}

void GTest_FindInSNPResultTable::prepare(){
    if (hasError()){
        return;
    }
    bool initialized = resTable.init(fileName, resultType);
    if (!initialized){
        stateInfo.setError(QString("Cannot read SNP result table file: %1").arg(fileName));
        return;
    }
}

Task::ReportResult GTest_FindInSNPResultTable::report(){
    if (hasError()){
        return ReportResult_Finished;
    }

    QStringList snpRaw = resTable.findSNPRaw(chrName, allele, pos, geneId);
    if (snpRaw.isEmpty()){
        stateInfo.setError(GTest::tr("Cannot find snp raw for snp %1 %2 %3 %4").arg(chrName).arg(allele).arg(pos).arg(geneId));
        return ReportResult_Finished;
    }

    int colIdx = resTable.getColumnIdxByName(columnName);
    if (colIdx == -1){
        stateInfo.setError(GTest::tr("Cannot find column name %1").arg(columnName));
        return ReportResult_Finished;
    }

    QString currentValue = snpRaw.at(colIdx);
    if (currentValue != expectedValue){
        stateInfo.setError(GTest::tr("Values do not match. Current:%1. Expected:%2").arg(currentValue).arg(expectedValue));
        return ReportResult_Finished;
    }
    
    return ReportResult_Finished;
}

QList<XMLTestFactory*> SNPResultTests::createTestFactories(){
    QList<XMLTestFactory*> res;
    res.append(GTest_FindInSNPResultTable::createFactory());
    return res;
}


bool GTest_SNPResultTable::init( const QString& fileName, SNPResultType _resultType ){
    if (initialized){
        return true;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    while (!file.atEnd()) {
        QString line = file.readLine();
        if (line.startsWith("#") || line.isEmpty()){
            continue;
        }
        QStringList columns = line.split('\t');
        raws.append(columns);
    }
    file.close();

    initOrders();

    resultType = _resultType;
    initialized = true;

    return true;
}

QStringList GTest_SNPResultTable::findSNPRaw( const QString& chrName, const QString& allele, qint64 pos, const QString& geneId ){
    QStringList res;
    if (!initialized){
        return res;
    }
    int chrIdx = getColumnIdxByName("Chr");
    int alleleIdx = getColumnIdxByName("Allele");
    int posIdx = getColumnIdxByName("Position");
    int geneidIdx = getColumnIdxByName("GeneId");

    if (chrIdx == -1 || alleleIdx == -1 || posIdx == -1 || geneidIdx == -1){
        return res;
    }

    foreach(QStringList raw, raws){
        QString curChr = raw.at(chrIdx);
        QString curAllele = raw.at(alleleIdx);
        qint64 curPos = raw.at(posIdx).toInt();
        QString curGeneId = raw.at(geneidIdx);
        if ((0 == curChr.compare(chrName)) && (0 == curAllele.compare(allele)) && (curPos == pos) && (0 == curGeneId.compare(geneId))){
            res = raw;
            break;
        }
    }

    
    return res;
}

void GTest_SNPResultTable::clear(){
    raws.clear();
    initialized = false;
    ingeneOrder.clear();
    regulatoryOrder.clear();

}

qint64 GTest_SNPResultTable::getColumnIdxByName( const QString& name ){
    int res = -1;
    if (!initialized){
        return res;
    }

    if (resultType == SNPResultInGene){
        res = ingeneOrder.indexOf(name);
    }else if (resultType == SNPResultRegulatory){
        res = regulatoryOrder.indexOf(name);
    }

    return res;
}

void GTest_SNPResultTable::initOrders(){
    ingeneOrder.clear();
    regulatoryOrder.clear();

    
    ingeneOrder 
        << "Chr" 
        << "Position" 
        << "Allele"
        << "dbSNPId"
        << "GeneId"
        << "Clinical_significance"
        << "Location"
        << "Protein"
        << "Codon"
        << "SubstitutionAA"
        << "SIFTeffect"
        << "SIFTscore"
        << "prot1d"
        << "prot3d"
        << "snp2pdb"
        << "LRT"
        << "PhyloP"
        << "PolyPhen2"
        << "MutationTaster"
        << "genomes1000"
        << "segmental"
        << "conserved"
        << "altall"
        << "hapmap"
        << "gerpScore";

    regulatoryOrder
        << "Chr" 
        << "Position" 
        << "Allele"
        << "dbSNPId"
        << "GeneId"
        << "Clinical_significance"
        << "promoterPos"
        << "rSNPTranscrFactors"
        << "ChIPTools"
        << "TATATool"
        << "genomes1000"
        << "segmental"
        << "conserved"
        << "altall"
        << "hapmap"
        << "gerpScore";
}

} //namespace
