#ifndef _S3_VARIATION_INFO_H_
#define _S3_VARIATION_INFO_H_

#include "DamageEffectEvaluator.h"

#include <U2Core/Gene.h>
#include <U2Core/DamageEffect.h>

#include <U2Core/U2Variant.h>
#include <U2Core/U2Type.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>

namespace U2 {

/* Class for S3 variation info storage. Contains variation description for each gene*/
    

class VariationInfo : public QObject{
    Q_OBJECT
public:
    enum ReportColumns {
        Chr = 1,
        Position, 
        Allele,
        dbSNPId,
        GeneId,
        Clinical_significance,
        Location,
        Protein,
        Codon,
        SubstitutionAA,
        SIFTeffect,
        SIFTscore,
        LRT,
        PhyloP,
        PolyPhen2,
        MutationTaster,
        genomes1000,
        segmental,
        conserved,
        altall,
        hapmap,
        gerpScore
    };


    VariationInfo(const U2Variant& var, const U2DataId& seqId, U2SequenceDbi* _seqDbi, const QString& sequenceName = QString());
    VariationInfo(const U2Variant& var, const U2DataId& seqId, const QList<Gene>& genes, U2SequenceDbi* _seqDbi, const QString& sequenceName = QString());

    QString getInGeneTableHeader();
    QString getOutGeneTableHeader();

    QStringList getInGeneTableRaws();

    void initInfo(U2VariantDbi* varDbi, QSharedPointer<DamageEffectEvaluator> evaluator, bool withEffect = false);
    QString getVariationInfoHeader();
    QString getInGeneEffectInfo(const QString& geneName, QMap<ReportColumns, QString>& rawData );
    QString getInGeneLocationInfo(const QString& geneName);
    QString gentFullGeneReport(const QString& geneName);
    QString getFullVariationReport();

    bool containsGene(const QString& name) const;
    Gene getGeneByName(const QString& name) const;

    bool isIntergenic();
    QString getNearGenes(const QList<Gene>& predefinedGenes = QList<Gene>());

private:
    U2SequenceDbi* seqDbi;
    QSharedPointer<DamageEffectEvaluator> evaluator;
    U2Variant           variant;
    U2VariantTrack      variantTrack;
    U2DataId            seqId;
    QString             sequenceName;
    QList<Gene>         genes;
    QList<DamageEffect> deffectList;

    bool                genesFound;
    bool                effectLoaded;

    DNATranslation* complTransl;
    DNATranslation* aaTransl;

    QList<ReportColumns> columnsOrderInGene;

private:
    void initOrderColumns();
    void addValueToRaw(const QString& val, ReportColumns key, QMap<ReportColumns, QString>& rawData);


};

} //namespace

#endif //_S3_VARIATION_INFO_H_
