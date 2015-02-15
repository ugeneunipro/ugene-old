/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FEATURE_TYPE_H_
#define _U2_FEATURE_TYPE_H_

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT U2FeatureTypes {
public:
    enum U2FeatureType {
        Invalid                     = 0,
        AaRich                      = 1,
        Acetylation                 = 2,
        ActiveSite                  = 3,
        Adenylation                 = 4,
        Allele                      = 5,
        AlphaHelix                  = 6,
        AlteredSite                 = 7,
        Amidation                   = 8,
        AssemblyGap                 = 9,
        Attenuator                  = 10,
        BHlhDomain                  = 11,
        Basic                       = 12,
        BetaSheet                   = 13,
        BetaStrandRegion            = 14,
        BindingSiteMisc             = 15,
        BiotinBindingSite           = 16,
        Blocked                     = 17,
        BondMisc                    = 18,
        C2                          = 19,
        CRegion                     = 20,
        CaatSignal                  = 21,
        Calcium                     = 22,
        CatalyticRegion             = 23,
        Cds                         = 24,
        CellAttachment              = 25,
        Cellular                    = 26,
        Centromere                  = 27,
        CholesterolBindingSite      = 28,
        CleavageSite                = 29,
        CoiledCoil                  = 30,
        CollagenType                = 31,
        Conflict                    = 32,
        ConnectingPeptide           = 33,
        Cub                         = 34,
        Cytoplasmic                 = 35,
        DLoop                       = 36,
        DSegment                    = 37,
        Disulfide                   = 38,
        DnaRnaBindingRegionMisc     = 39,
        DomainMisc                  = 40,
        Egf                         = 41,
        Enhancer                    = 42,
        Exon                        = 43,
        Exoplasmic                  = 44,
        Extracellular               = 45,
        Farnesyl                    = 46,
        Fibronectin                 = 47,
        FivePrimeClip               = 48,
        FivePrimeUtr                = 49,
        Formylation                 = 50,
        GammaCarboxyglumaticAcid    = 51,
        Gap                         = 52,
        GcSignal                    = 53,
        Gene                        = 54,
        GeranylGeranyl              = 55,
        Glycosylation               = 56,
        GlycosylationSite           = 57,
        GpiAnchor                   = 58,
        HelicalRegion               = 59,
        HemeBindingSite             = 60,
        HmgBox                      = 61,
        Homeodomain                 = 62,
        Hth                         = 63,
        HydrogenBondedTurn          = 64,
        Hydroxylation               = 65,
        IDna                        = 66,
        Immunoglobulin              = 67,
        Insertion                   = 68,
        Intracellular               = 69,
        Intron                      = 70,
        JRegion                     = 71,
        JSegment                    = 72,
        Kh                          = 73,
        Kinase                      = 74,
        LeucineZipper               = 75,
        LeucineZipperDomain         = 76,
        LipidMisc                   = 77,
        Loci                        = 78,
        Ltr                         = 79,
        MRna                        = 80,
        MatureChain                 = 81,
        MaturePeptide               = 82,
        MetalMisc                   = 83,
        Methylation                 = 84,
        Minus10Signal               = 85,
        Minus35Signal               = 86,
        MiscBindingSite             = 87,
        MiscDifference              = 88,
        MiscFeature                 = 89,
        MiscMarker                  = 90,
        MiscRecombination           = 91,
        MiscRna                     = 92,
        MiscSignal                  = 93,
        MiscStructure               = 94,
        MobileElement               = 95,
        ModifiedBase                = 96,
        Mutation                    = 97,
        Myristate                   = 98,
        NAcylDiglyceride            = 99,
        NRegion                     = 100,
        NcRna                       = 101,
        NonConsecutiveResidues      = 102,
        NpBindingRegionMisc         = 103,
        Nuclease                    = 104,
        OldSequence                 = 105,
        Operon                      = 106,
        OriT                        = 107,
        Overhang                    = 108,
        Palmitate                   = 109,
        Periplasmic                 = 110,
        Ph                          = 111,
        Phosphorylation             = 112,
        PolyASignal                 = 113,
        PolyASite                   = 114,
        PolyAa                      = 115,
        Precursor                   = 116,
        PrecursorRna                = 117,
        Primer                      = 118,
        PrimerBindingSite           = 119,
        PrimeryTranscript           = 120,
        ProcessedActivePeptide      = 121,
        Promoter                    = 122,
        PromoterEukaryotic          = 123,
        PromoterProkaryotic         = 124,
        Propeptide                  = 125,
        Proprotein                  = 126,
        Protease                    = 127,
        Protein                     = 128,
        ProteinBindingSite          = 129,
        Provirus                    = 130,
        PyridoxalPhBindingSite      = 131,
        PyrrolidoneCarboxylicAcid   = 132,
        RRna                        = 133,
        Rbs                         = 134,
        Region                      = 135,
        RegionMisc                  = 136,
        Regulatory                  = 137,
        RepeatRegion                = 138,
        RepeatUnit                  = 139,
        RepetitiveRegion            = 140,
        ReplicationOrigin           = 141,
        ResidueModificationMisc     = 142,
        SRegion                     = 143,
        Satellite                   = 144,
        ScRna                       = 145,
        SeconadaryStructure         = 146,
        Sh2                         = 147,
        Sh3                         = 148,
        SignalPeptide               = 149,
        SignalSequence              = 150,
        Silencer                    = 151,
        Similarity                  = 152,
        Site                        = 153,
        SiteMisc                    = 154,
        SnRna                       = 155,
        SnoRna                      = 156,
        Source                      = 157,
        SplicingSignal              = 158,
        SplicingVariant             = 159,
        StemLoop                    = 160,
        Sts                         = 161,
        Sulfatation                 = 162,
        TRna                        = 163,
        TataSignal                  = 164,
        Telomere                    = 165,
        Terminator                  = 166,
        Thioether                   = 167,
        Thiolester                  = 168,
        ThreePrimeClip              = 169,
        ThreePrimeUtr               = 170,
        TmRna                       = 171,
        TransitPeptide              = 172,
        TransmembraneRegion         = 173,
        Transposon                  = 174,
        Uncertainty                 = 175,
        Unsure                      = 176,
        VRegion                     = 177,
        VSegment                    = 178,
        Variant                     = 179,
        Variation                   = 180,
        Virion                      = 181,
        ZincFinger                  = 182,
        ZincFingerDomain            = 183,
        FIRST_FEATURE_TYPE          = AaRich,
        LAST_FEATURE_TYPE           = ZincFingerDomain

        // Feature types:
        //    vntifkey  gb-type             label
        //    0         "attenuator"        "Attenuator"
        //    1         "bHLH_domain"       "bHLH\Domain"
        //    2         "C_region"          "C-Region"
        //    3         "CAAT_signal"       "CAAT\Signal"
        //    4         "CDS"               "CDS"
        //    5         "centromere"        "Centromere"
        //    6         "D-loop"            "D-Loop"
        //    7         "D_segment"         "D-Segment"
        //    9         "enhancer"          "Enhancer"
        //    10        "GC_signal"         "GC-Signal"
        //    11        "glycosil_site"     "Glycosylation\Site"
        //    12        "homeodomain"       "Homeodomain"
        //    13        "iDNA"              "iDNA"
        //    14        "insertion_seq"     "Insertion"
        //    15        "intron"            "Intron"
        //    16        "J_segment"         "J-Segment"
        //    17        "LZ_domain"         "Leucine\Zipper\Domain"
        //    18        "loci"              "Loci"
        //    19        "LTR"               "LTR"
        //    20        "misc_binding"      "Misc.\Binding\Site"
        //    21        "misc_feature"      "Misc.\Feature"
        //    22        "misc_marker"       "Misc.\Marker"
        //    23        "modified_base"     "Modified\Base"
        //    24        "N_region"          "N-Region"
        //    25        "polyA_signal"      "PolyA\Signal"
        //    26        "polyA_site"        "PolyA\Site"
        //    27        "primer"            "Primer"
        //    28        "primer_bind"       "Primer\Binding\Site"
        //    29        "promoter"          "Promoter\Eukaryotic"
        //    30        "promoter"          "Promoter\Prokaryotic"
        //    31        "protein_bind"      "Protein\Binding\Site"
        //    32        "RBS"               "RBS"
        //    33        "rep_origin"        "Replication\Origin"
        //    34        "repeat_region"     "Repeat\Region"
        //    35        "repeat_unit"       "Repeat\Unit"
        //    36        "S_region"          "S-Region"
        //    37        "silencer"          "Silencer"
        //    38        "splicing_signal"   "Splicing\Signal"
        //    39        "stem_loop"         "Stem\Loop"
        //    40        "STS"               "STS"
        //    41        "TATA_signal"       "TATA\Signal"
        //    42        "telomere"          "Telomere"
        //    43        "terminator"        "Terminator"
        //    44        "transposon"        "Transposon"
        //    45        "V_region"          "V-Region"
        //    46        "ZF_domain"         "Zinc\Finger\Domain"
        //    47        "-10_signal"        "-10\Signal"
        //    48        "-35_signal"        "-35\Signal"
        //    49        "3'clip"            "3'\Clip"
        //    50        "3'UTR"             "3'\UTR"
        //    51        "5'clip"            "5'\Clip"
        //    52        "5'UTR"             "5'\UTR"
        //    53        "misc_RNA"          "RNA\-\Misc."
        //    54        "mRNA"              "mRNA"
        //    55        "precursor_RNA"     "Precursor\RNA"
        //    56        "scRNA"             "scRNA"
        //    57        "snRNA"             "snRNA"
        //    58        "rRNA"              "rRNA"
        //    59        "tRNA"              "tRNA"
        //    61        "exon"              "Exon"
        //    62        "mutation"          "Mutation"
        //    81        "allele"            "Allele"
        //    82        "cellular"          "Cellular"
        //    83        "conflict"          "Conflict"
        //    84        "mat_peptide"       "Mature\Peptide"
        //    85        "misc_difference"   "Misc.\Difference"
        //    86        "misc_recomb"       "Misc.\Recombination"
        //    87        "misc_signal"       "Misc.\Signal"
        //    88        "misc_structure"    "Misc.\Structure"
        //    89        "old_sequence"      "Old\Sequence"
        //    91        "prim_transcript"   "Primery\Transcript"
        //    92        "provirus"          "Provirus"
        //    93        "satellite"         "Satellite"
        //    94        "sig_peptide"       "Signal\Peptide"
        //    95        "transit_peptide"   "Transit\Peptide"
        //    96        "variation"         "Variation"
        //    97        "virion"            "Virion"
        //    99        "unsure"            "Unsure"
        //    100       "V_segment"         "V-Segment"
        //    102       "gap"               "Gap"
        //    103       "operon"            "Operon"
        //    104       "oriT"              "oriT"
        //    105       "snoRNA"            "snoRNA"
        //    274       "overhang"          "Overhang"

        // Feature types:
        //    vntifkey  gb-type         label                           path
        //    200       "Region"        "Signal\Sequence"               "Protein Functional Features/Regions"
        //    201       "Region"        "Transit\peptide"               "Protein Functional Features/Regions"
        //    202       "Region"        "Propeptide"                    "Protein Functional Features/Regions"
        //    203       "Region"        "Mature\chain"                  "Protein Functional Features/Regions"
        //    204       "Region"        "Processed\active\peptide"      "Protein Functional Features/Regions"
        //    205       "Region"        "Repetitive\region"             "Protein Sequence Features"
        //    206       "Region"        "Similarity"                    "Protein Sequence Features"
        //    207       "Region"        "Helical\region"                "Protein Structure Features/3D"
        //    208       "Region"        "Hydrogen\bonded\turn"          "Protein Structure Features/3D"
        //    209       "Region"        "Beta-strand\region"            "Protein Structure Features/3D"
        //    210       "Region"        "Transmembrane\Region"          "Protein Functional Features/Domains"
        //    211       "Region"        "Calcium"                       "Protein Functional Features/Domains/Metal Binding"
        //    212       "Site"          "DNA/RNA\binding\region:\Misc"  "Protein Functional Features/Domains/DNA//Rna Binding"
        //    213       "Site"          "NP\binding\region:\Misc"       "Protein Functional Features/Domains/Other Binding"
        //    219       "Site"          "Cell\Attachment"               "Protein Functional Features/Sites"
        //    220       "Site"          "Active\Site"                   "Protein Functional Features/Sites"
        //    221       "Site"          "Cleavage\Site"                 "Protein Functional Features/Sites"
        //    223       "Site"          "Heme\Binding\Site"             "Protein Functional Features/Domains/Other Binding"
        //    224       "Site"          "Pyridoxal\Ph.\Binding\Site"    "Protein Functional Features/Domains/Other Binding"
        //    225       "Site"          "Biotin\Binding\Site"           "Protein Functional Features/Domains/Other Binding"
        //    226       "Site"          "Cholesterol\Binding\Site"      "Protein Functional Features/Domains/Other Binding"
        //    227       "Site"          "Acetylation"                   "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    228       "Site"          "Adenylation"                   "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    229       "Site"          "Amidation"                     "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    230       "Site"          "Blocked"                       "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    231       "Site"          "Formylation"                   "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    232       "Site"          "Gamma-Carboxyglumatic\Acid"    "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    233       "Site"          "Hydroxylation"                 "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    234       "Site"          "Methylation"                   "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    235       "Site"          "Phosphorylation"               "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    236       "Site"          "Pyrrolidone\Carboxylic\Acid"   "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    237       "Site"          "Sulfatation"                   "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    238       "Site"          "Glycosylation"                 "Protein Structure Features/Post-Translational Modifications"
        //    239       "Site"          "Residue\Modification:\Misc"    "Protein Structure Features/Post-Translational Modifications/Residue Modifications"
        //    240       "Region"        "Conflict"                      "Protein Sequence Features"
        //    241       "Region"        "Variant"                       "Protein Sequence Features"
        //    242       "Region"        "Splicing\Variant"              "Protein Sequence Features"
        //    243       "Site"          "Altered\Site"                  "Protein Sequence Features"
        //    244       "Site"          "Non\Consecutive\Residues"      "Protein Sequence Features"
        //    245       "Region"        "Uncertainty"                   "Protein Sequence Features"
        //    248       "Site"          "Myristate"                     "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    249       "Site"          "Palmitate"                     "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    250       "Site"          "Farnesyl"                      "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    251       "Site"          "Geranyl-Geranyl"               "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    252       "Site"          "GPI-Anchor"                    "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    253       "Site"          "N-Acyl\Diglyceride"            "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    254       "Bond"          "Disulfide"                     "Protein Structure Features/Bond Modifications"
        //    255       "Bond"          "Thiolester"                    "Protein Structure Features/Bond Modifications"
        //    256       "Bond"          "Thioether"                     "Protein Structure Features/Bond Modifications"
        //    262       "Site"          "Lipid:\Misc"                   "Protein Structure Features/Post-Translational Modifications/Lipid Modifications"
        //    263       "Bond"          "Bond:\Misc"                    "Protein Structure Features/Bond Modifications"
        //    264       "Site"          "Metal:\Misc"                   "Protein Functional Features/Domains/Metal Binding"
        //    265       "Site"          "Site:\Misc"                    "Protein Functional Features/Sites"
        //    266       "Region"        "Region:\Misc"                  "Protein Functional Features/Regions"
        //    272       "proprotein"    "Proprotein"                    "Protein Functional Features/Regions"
        //    273       "misc_feat"     "Misc.\Feature"                 ""
        //    275       "variation"     "Variation"                     "Protein Sequence Features"
        //    276       "Precursor"     "Precursor"                     "Protein Functional Features/Regions"
        //    277       "SecStr"        "Seconadary\structure"          "Protein Structure Features/3D"
        //    1000      "Region"        "Domain:\Misc"                  "Protein Functional Features/Domains/Other Functional"
        //    1001      "Region"        "Extracellular"                 "Protein Functional Features/Domains"
        //    1002      "Region"        "Intracellular"                 "Protein Functional Features/Domains"
        //    1003      "Region"        "Connecting\Peptide"            "Protein Functional Features/Domains"
        //    1004      "Region"        "Cytoplasmic"                   "Protein Functional Features/Domains"
        //    1005      "Region"        "Periplasmic"                   "Protein Functional Features/Domains"
        //    1006      "Region"        "Exoplasmic"                    "Protein Functional Features/Domains"
        //    1007      "Site"          "Binding\Site:\Misc"            "Protein Functional Features/Domains/Other Binding"
        //    1008      "Region"        "Catalytic\Region"              "Protein Functional Features/Regions"
        //    1011      "Region"        "Kinase"                        "Protein Functional Features/Domains/Other Functional"
        //    1012      "Region"        "Protease"                      "Protein Functional Features/Domains/Other Functional"
        //    1013      "Region"        "Nuclease"                      "Protein Functional Features/Domains/Other Functional"
        //    1014      "Region"        "AA-Rich"                       "Protein Functional Features/Domains/Other Functional"
        //    1015      "Region"        "Poly-AA"                       "Protein Functional Features/Domains/Other Functional"
        //    1016      "Region"        "CUB"                           "Protein Functional Features/Domains/Other Functional"
        //    1017      "Region"        "EGF"                           "Protein Functional Features/Domains/Other Functional"
        //    1018      "Region"        "Fibronectin"                   "Protein Functional Features/Domains/Other Functional"
        //    1019      "Region"        "Immunoglobulin"                "Protein Functional Features/Domains/Other Functional"
        //    1020      "Region"        "Leucine\Zipper"                "Protein Functional Features/Domains/Other Functional"
        //    1021      "Region"        "C2"                            "Protein Functional Features/Domains/Other Functional"
        //    1022      "Region"        "Coiled\coil"                   "Protein Structure Features/3D"
        //    1024      "Region"        "KH"                            "Protein Functional Features/Domains/Other Functional"
        //    1025      "Region"        "PH"                            "Protein Functional Features/Domains/Other Functional"
        //    1026      "Region"        "SH2"                           "Protein Functional Features/Domains/Other Functional"
        //    1027      "Region"        "SH3"                           "Protein Functional Features/Domains/Other Functional"
        //    1028      "Region"        "Zinc\Finger"                   "Protein Functional Features/Domains/DNA//Rna Binding"
        //    1029      "Region"        "Collagen-type"                 "Protein Functional Features/Domains/Other Functional"
        //    1030      "Site"          "Basic"                         "Protein Functional Features/Domains/DNA//Rna Binding"
        //    1031      "Site"          "HMG-Box"                       "Protein Functional Features/Domains/DNA//Rna Binding"
        //    1032      "Site"          "Homeodomain"                   "Protein Functional Features/Domains/DNA//Rna Binding"
        //    1033      "Site"          "H-T-H"                         "Protein Functional Features/Domains/DNA//Rna Binding"
        //    1100      "Region"        "Alpha-Helix"                   "Protein Structure Features/3D"
        //    1101      "Region"        "Beta-Sheet"                    "Protein Structure Features/3D"
    };

    static QString getVisualName(U2FeatureType type);
    static U2FeatureType getTypeByName(const QString &visualName);

private:
    static QHash<U2FeatureType, QString> initFeatureTypes();
    const static QHash<U2FeatureType, QString> typesAndVisualNames;
};

typedef U2FeatureTypes::U2FeatureType U2FeatureType;

}   // namespace U2

#endif // _U2_FEATURE_TYPE_H_
