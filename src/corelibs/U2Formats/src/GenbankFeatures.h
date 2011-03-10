#ifndef _U2_GENBANK_FEATURES_H_
#define _U2_GENBANK_FEATURES_H_

#include <U2Core/global.h>

#include <QtCore/QMultiMap>
#include <QtCore/QVector>
#include <QtGui/QColor>

namespace U2 {

enum GBFeatureKey {
    GBFeatureKey_attenuator,	    // Sequence related to transcription termination
    GBFeatureKey_C_region,	    // Span of the C immunological feature
    GBFeatureKey_CAAT_signal,	    // `CAAT box' in eukaryotic promoters
    GBFeatureKey_CDS,		        // Sequence coding for amino acids in protein (includes stop codon)
    GBFeatureKey_conflict,	    // Independent sequence determinations differ
    GBFeatureKey_D_loop,      	// Displacement loop
    GBFeatureKey_D_segment,	    // Span of the D immunological feature
    GBFeatureKey_enhancer,	    // Cis-acting enhancer of promoter function
    GBFeatureKey_exon,		    // Region that codes for part of spliced mRNA
    GBFeatureKey_gene,            // Region that defines a functional gene, possibly including upstream (promotor, enhancer, etc) and downstream control elements, and for which a name has been assigned.
    GBFeatureKey_GC_signal,	    // `GC box' in eukaryotic promoters
    GBFeatureKey_iDNA,		    // Intervening DNA eliminated by recombination
    GBFeatureKey_intron,		    // Transcribed region excised by mRNA splicing
    GBFeatureKey_J_region,	    // Span of the J immunological feature
    GBFeatureKey_LTR,		        // Long terminal repeat
    GBFeatureKey_mat_peptide,	    // Mature peptide coding region (does not include stop codon)
    GBFeatureKey_misc_binding,	// Miscellaneous binding site
    GBFeatureKey_misc_difference,	// Miscellaneous difference feature
    GBFeatureKey_misc_feature,	// Region of biological significance that cannot be described by any other feature
    GBFeatureKey_misc_recomb,     // Miscellaneous, recombination feature
    GBFeatureKey_misc_RNA,	    // Miscellaneous transcript feature not defined by other RNA keys
    GBFeatureKey_misc_signal,	    // Miscellaneous signal
    GBFeatureKey_misc_structure,	// Miscellaneous DNA or RNA structure
    GBFeatureKey_modified_base,	// The indicated base is a modified nucleotide
    GBFeatureKey_mRNA,		    // Messenger RNA
    GBFeatureKey_N_region,	    // Span of the N immunological feature
    GBFeatureKey_old_sequence,	// Presented sequence revises a previous version
    GBFeatureKey_polyA_signal,	// Signal for cleavage & polyadenylation
    GBFeatureKey_polyA_site,	    // Site at which polyadenine is added to mRNA
    GBFeatureKey_precursor_RNA,	// Any RNA species that is not yet the mature RNA product
    GBFeatureKey_prim_transcript,	// Primary (unprocessed) transcript
    GBFeatureKey_primer,		    // Primer binding region used with PCR
    GBFeatureKey_primer_bind,	    // Non-covalent primer binding site
    GBFeatureKey_promoter,	    // A region involved in transcription initiation
    GBFeatureKey_protein_bind,	// Non-covalent protein binding site on DNA or RNA
    GBFeatureKey_RBS,		        // Ribosome binding site
    GBFeatureKey_rep_origin,	    // Replication origin for duplex DNA
    GBFeatureKey_repeat_region,	// Sequence containing repeated subsequences
    GBFeatureKey_repeat_unit,	    // One repeated unit of a repeat_region
    GBFeatureKey_rRNA,		    // Ribosomal RNA
    GBFeatureKey_S_region,	    // Span of the S immunological feature
    GBFeatureKey_satellite,	    // Satellite repeated sequence
    GBFeatureKey_scRNA,		    // Small cytoplasmic RNA
    GBFeatureKey_sig_peptide,	    // Signal peptide coding region
    GBFeatureKey_snRNA,		    // Small nuclear RNA
    GBFeatureKey_source,		    // Identifies the biological source of the specified span of the sequence
    GBFeatureKey_stem_loop,	    // Hair-pin loop structure in DNA or RNA
    GBFeatureKey_STS,		        // Sequence Tagged Site; operationally unique sequence that identifies the combination of primer spans used in a PCR assay
    GBFeatureKey_TATA_signal,	    // `TATA box' in eukaryotic promoters
    GBFeatureKey_terminator,	    // Sequence causing transcription termination
    GBFeatureKey_transit_peptide,	// Transit peptide coding region
    GBFeatureKey_transposon,	    // Transposable element (TN)
    GBFeatureKey_tRNA, 		    // Transfer RNA
    GBFeatureKey_unsure,		    // Authors are unsure about the sequence in this region
    GBFeatureKey_V_region,	    // Span of the V immunological feature
    GBFeatureKey_variation, 	    // A related population contains stable mutation
    GBFeatureKey__10_signal,	    // `Pribnow box' in prokaryotic promoters
    GBFeatureKey__35_signal,	    // `-35 box' in prokaryotic promoters
    GBFeatureKey_3_clip,		    // 3'-most region of a precursor transcript removed in processing
    GBFeatureKey_3_UTR,		    // 3' untranslated region (trailer)
    GBFeatureKey_5_clip,		    // 5'-most region of a precursor transcript removed in processing
    GBFeatureKey_5_UTR,		    // 5' untranslated region (leader)
    GBFeatureKey_Protein,       // 
    GBFeatureKey_Region,        // 
    GBFeatureKey_Site,          // 
    GBFeatureKey_NUM_KEYS,
    GBFeatureKey_UNKNOWN = GBFeatureKey_NUM_KEYS
};

class U2FORMATS_EXPORT GBFeatureKeyInfo {
public:
    GBFeatureKeyInfo() : id (GBFeatureKey_UNKNOWN), showOnaminoFrame(false) {} 
    GBFeatureKeyInfo(GBFeatureKey _id, const QString& _text, const QColor& _color, bool _aminoFrame, QString _desc) 
        : id (_id), text(_text), color(_color), showOnaminoFrame(_aminoFrame), desc(_desc) {} 

    GBFeatureKey  id;
    QString     text;
    QColor      color;
    bool        showOnaminoFrame;
    QString     desc;
    QStringList namingQuals;
};

class U2FORMATS_EXPORT GBFeatureUtils : public QObject {
    Q_OBJECT
public:
    static const QVector<GBFeatureKeyInfo>& allKeys();

    static const GBFeatureKeyInfo& getKeyInfo(GBFeatureKey key) {return allKeys().at(key);}

    static const QMultiMap<QString, GBFeatureKey>& getKeyGroups();
    
    static GBFeatureKey getKey(const QString& text);

    static const QByteArray QUALIFIER_AMINO_STRAND;
    static const QByteArray QUALIFIER_AMINO_STRAND_YES;
    static const QByteArray QUALIFIER_AMINO_STRAND_NO;

    static const QByteArray QUALIFIER_NAME;
    static const QByteArray QUALIFIER_GROUP;

    static const QString DEFAULT_KEY;

    static const QString QUALIFIER_CUT;
    
    //max annotation key len in Genbank or EMBL formats
    static const int    MAX_KEY_LEN;
};

}//namespace
#endif

