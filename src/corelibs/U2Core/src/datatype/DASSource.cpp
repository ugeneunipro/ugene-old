/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "DASSource.h"

namespace U2 {

DASSourceRegistry::DASSourceRegistry()
:QObject()
{
    initDefaultSources();
}

DASSourceRegistry::~DASSourceRegistry(){
    referenceSources.clear();
    featureSources.clear();
}

DASSource DASSourceRegistry::findById( const QString& id ) const{
    DASSource res;

    foreach(const DASSource& s, referenceSources){
        if (s.getId() == id){
            res = s;
            return res;
        }
    }

    foreach(const DASSource& s, featureSources){
        if (s.getId() == id){
            res = s;
            return res;
        }
    }

    return res;
}

DASSource DASSourceRegistry::findByName( const QString& name ) const{
    DASSource res;

    foreach(const DASSource& s, referenceSources){
        if (s.getName() == name){
            res = s;
            return res;
        }
    }

    foreach(const DASSource& s, featureSources){
        if (s.getName() == name){
            res = s;
            return res;
        }
    }

    return res;
}

QList<DASSource> DASSourceRegistry::getReferenceSources() const{
    return referenceSources;
}

QList<DASSource> DASSourceRegistry::getFeatureSources() const{
    return featureSources;
}

QList<DASSource> DASSourceRegistry::getFeatureSourcesByType( DASReferenceType rType ) const{
    QList<DASSource> res;
    
    foreach(const DASSource& s, featureSources){
        if (s.getReferenceType() == rType){
            res.append(s);
        }
    }

    return res;
}

#define DAS_UNIPROT "dasuniprot"
#define DAS_ENSEMBL "dasensembl"
void DASSourceRegistry::initDefaultSources(){

    //uniprot
    //sources from DASTY (http://www.ebi.ac.uk/dasty/)
    DASSource uniprotRef(DAS_UNIPROT, 
                         tr("UniProt (DAS)"), 
                         tr("http://www.ebi.ac.uk/das-srv/uniprot/das/uniprot/"), 
                         DASProteinSequence,
                         tr("Use Swiss-Prot, TrEMBL, UniProt, IPI or UniParc accession number. For example: P05067"),
                         tr("UniProt database of protein sequences and features."));

    referenceSources.append(uniprotRef);

    DASSource uniprotFeature1(QString(DAS_UNIPROT)+"f1", 
        tr("UniProt"), 
        tr("http://www.ebi.ac.uk/das-srv/uniprot/das/uniprot/"), 
        DASProteinSequence,
        tr("DAS 1 reference server for sequence and features from UniProt.\n"
        "UniProt (Universal Protein Resource) is the worlds most comprehensive catalog of information on proteins.\n"
        "It is a central repository of protein sequence and function created by joining the information contained\n"
        "in Swiss-Prot, TrEMBL, and PIR.")
        );

    featureSources.append(uniprotFeature1);

    DASSource uniprotFeature2(QString(DAS_UNIPROT)+"f2", 
        tr("Pride DAS 1.6"), 
        tr("http://www.ebi.ac.uk/pride-das/das/PrideDataSource/"), 
        DASProteinSequence,
        tr("The PRIDE database is a centralized, public data repository for proteomics data. \n"
        "It has been developed to provide the proteomics community with a public repository \n"
        "for protein and peptide identifications together with the evidence supporting these \n"
        "identifications. In addition to identifications, PRIDE is able to capture details of \n"
        "post-translational modifications coordinated relative to the peptides in which they have been found.")
        );

    featureSources.append(uniprotFeature2);

    DASSource uniprotFeature3(QString(DAS_UNIPROT)+"f3", 
        tr("cbs_sort"), 
        tr("http://das.cbs.dtu.dk:9000/das/cbs_sort/"), 
        DASProteinSequence,
        tr("The combined result of the signalp, secretomep, targetp, netnes & lipop server for the given id.")
        );

    featureSources.append(uniprotFeature3);

    DASSource uniprotFeature4(QString(DAS_UNIPROT)+"f4", 
        tr("signalp"), 
        tr("http://das.cbs.dtu.dk:9000/das/signalp/"), 
        DASProteinSequence,
        tr("SignalP (http://www.cbs.dtu.dk/services/SignalP) predictions for UniProt.")
        );

    featureSources.append(uniprotFeature4);

    DASSource uniprotFeature5(QString(DAS_UNIPROT)+"f5", 
        tr("InterPro-Matches-Overview"), 
        tr("http://www.ebi.ac.uk/das-srv/interpro/das/InterPro-matches-overview/"), 
        DASProteinSequence,
        tr("Shows the maximum extent of the matches from all signatures that are \n"
        "integrated into a single InterPro entry against UniProtKB protein sequences \n"
        "(i.e. if an InterPro entry contains 2 signatures, A and B, and both of \n"
        "these match a protein, A between residue 3 and 125 and B between residue \n"
        "5 and 127, the overview supermatch of these signatures is between 3 and 127).")
        );

    featureSources.append(uniprotFeature5);

    //Ensembl Human Genes
    DASSource ensemblRef(DAS_ENSEMBL, 
        tr("Ensembl Human Genes (DAS)"), 
        tr("http://www.ebi.ac.uk/das-srv/genedas/das/Homo_sapiens.Gene_ID.reference/"), 
        DASDNASequence,
        tr("Use Ensemble Gene IDs. For example: ENSG00000139618"),
        tr("The Ensembl human Gene_ID reference source, serving sequences and non-location features."));

    referenceSources.append(ensemblRef);

    DASSource ensemblFeatures1(QString(DAS_ENSEMBL) + "f1", 
        tr("Ensembl Human Gene Features"), 
        tr("http://www.ebi.ac.uk/das-srv/genedas/das/Homo_sapiens.Gene_ID.reference/"), 
        DASDNASequence,
        tr("The Ensembl human Gene_ID source."));

    featureSources.append(ensemblFeatures1);

    DASSource ensemblFeatures2(QString(DAS_ENSEMBL) + "f2", 
        tr("HGNC"), 
        tr("http://www.genenames.org/das/HGNC/"), 
        DASDNASequence,
        tr("The HGNC (HUGO Gene Nomenclature Committee) DAS Reference Server serves up-to-date\n"
        "approved gene symbols and names for human genes from the latest HGNC dataset, mapped via Entrez Gene ID."));

    featureSources.append(ensemblFeatures2);

    DASSource ensemblFeatures3(QString(DAS_ENSEMBL) + "f3", 
        tr("GAD"), 
        tr("http://www.ebi.ac.uk/das-srv/genedas/das/gad/"), 
        DASDNASequence,
        tr("Genetic Association Database (diseases)."));

    featureSources.append(ensemblFeatures3);

}

QString DASSourceRegistry::getRequestURLString(const DASSource& source, const QString& accId, DASObjectType& requestType ){

    QString res = "";
    if (!source.isValid() || accId.isEmpty()){
        return res;
    }

    res = source.getUrl();

    if (requestType == DASSequence){
        res += "sequence";
    }else if(requestType == DASFeatures){
        res += "features";
    }else{
        return "";
    }

    res += "?segment=" + accId;

    return res;
}

}//namespace
