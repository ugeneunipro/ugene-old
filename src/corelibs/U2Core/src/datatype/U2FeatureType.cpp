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

#include "U2FeatureType.h"

namespace U2 {

const QHash<U2FeatureTypes::U2FeatureType, QString> U2FeatureTypes::typesAndVisualNames = U2FeatureTypes::initFeatureTypes();

QString U2FeatureTypes::getVisualName(U2FeatureType type) {
    return typesAndVisualNames[type];
}

U2FeatureTypes::U2FeatureType U2FeatureTypes::getTypeByName(const QString &visualName) {
    return typesAndVisualNames.key(visualName, Invalid);
}

QHash<U2FeatureTypes::U2FeatureType, QString> U2FeatureTypes::initFeatureTypes() {
    QHash<U2FeatureType, QString> featureTypes;

    featureTypes.insert(AaRich,                     "AA-Rich");
    featureTypes.insert(Acetylation,                "Acetylation");
    featureTypes.insert(ActiveSite,                 "Active Site");
    featureTypes.insert(Adenylation,                "Adenylation");
    featureTypes.insert(Allele,                 "Allele");
    featureTypes.insert(AlphaHelix,                 "Alpha-Helix");
    featureTypes.insert(AlteredSite,                "Altered Site");
    featureTypes.insert(Amidation,                  "Amidation");
    featureTypes.insert(AssemblyGap,             "Assembly gap");
    featureTypes.insert(Attenuator,             "Attenuator");
    featureTypes.insert(BHlhDomain,             "bHLH Domain");
    featureTypes.insert(Basic,                      "Basic");
    featureTypes.insert(BetaSheet,                  "Beta-Sheet");
    featureTypes.insert(BetaStrandRegion,           "Beta-strand region");
    featureTypes.insert(BindingSiteMisc,            "Binding Site: Misc");
    featureTypes.insert(BiotinBindingSite,          "Biotin Binding Site");
    featureTypes.insert(Blocked,                    "Blocked");
    featureTypes.insert(BondMisc,                   "Bond: Misc");
    featureTypes.insert(C2,                         "C2");
    featureTypes.insert(CRegion,                "C-Region");
    featureTypes.insert(CaatSignal,             "CAAT Signal");
    featureTypes.insert(Calcium,                    "Calcium");
    featureTypes.insert(CatalyticRegion,            "Catalytic Region");
    featureTypes.insert(Cds,                    "CDS");
    featureTypes.insert(CellAttachment,             "Cell Attachment");
    featureTypes.insert(Cellular,               "Cellular");
    featureTypes.insert(Centromere,             "Centromere");
    featureTypes.insert(CholesterolBindingSite,     "Cholesterol Binding Site");
    featureTypes.insert(CleavageSite,               "Cleavage Site");
    featureTypes.insert(CoiledCoil,                 "Coiled coil");
    featureTypes.insert(CollagenType,               "Collagen-type");
    featureTypes.insert(Conflict,       "Conflict");
    featureTypes.insert(ConnectingPeptide,          "Connecting Peptide");
    featureTypes.insert(Cub,                        "CUB");
    featureTypes.insert(Cytoplasmic,                "Cytoplasmic");
    featureTypes.insert(DLoop,                  "D-Loop");
    featureTypes.insert(DSegment,               "D-Segment");
    featureTypes.insert(Disulfide,                  "Disulfide");
    featureTypes.insert(DnaRnaBindingRegionMisc,    "DNA/RNA binding region: Misc");
    featureTypes.insert(DomainMisc,                 "Domain: Misc");
    featureTypes.insert(Egf,                        "EGF");
    featureTypes.insert(Enhancer,               "Enhancer");
    featureTypes.insert(Exon,                   "Exon");
    featureTypes.insert(Exoplasmic,                 "Exoplasmic");
    featureTypes.insert(Extracellular,              "Extracellular");
    featureTypes.insert(Farnesyl,                   "Farnesyl");
    featureTypes.insert(Fibronectin,                "Fibronectin");
    featureTypes.insert(FivePrimeClip,          "5' Clip");
    featureTypes.insert(FivePrimeUtr,           "5' UTR");
    featureTypes.insert(Formylation,                "Formylation");
    featureTypes.insert(GammaCarboxyglumaticAcid,   "Gamma-Carboxyglumatic Acid");
    featureTypes.insert(Gap,                    "Gap");
    featureTypes.insert(GcSignal,               "GC-Signal");
    featureTypes.insert(Gene,                    "Gene");
    featureTypes.insert(GeranylGeranyl,             "Geranyl-Geranyl");
    featureTypes.insert(Glycosylation,              "Glycosylation");
    featureTypes.insert(GlycosylationSite,      "Glycosylation Site");
    featureTypes.insert(GpiAnchor,                  "GPI-Anchor");
    featureTypes.insert(HelicalRegion,              "Helical region");
    featureTypes.insert(HemeBindingSite,            "Heme Binding Site");
    featureTypes.insert(HmgBox,                     "HMG-Box");
    featureTypes.insert(Homeodomain,    "Homeodomain");
    featureTypes.insert(Hth,                        "H-T-H");
    featureTypes.insert(HydrogenBondedTurn,         "Hydrogen bonded turn");
    featureTypes.insert(Hydroxylation,              "Hydroxylation");
    featureTypes.insert(IDna,                   "iDNA");
    featureTypes.insert(Immunoglobulin,             "Immunoglobulin");
    featureTypes.insert(Insertion,              "Insertion");
    featureTypes.insert(Intracellular,              "Intracellular");
    featureTypes.insert(Intron,                 "Intron");
    featureTypes.insert(JRegion,                "J-Region");
    featureTypes.insert(JSegment,               "J-Segment");
    featureTypes.insert(Kh,                         "KH");
    featureTypes.insert(Kinase,                     "Kinase");
    featureTypes.insert(LeucineZipper,              "Leucine Zipper");
    featureTypes.insert(LeucineZipperDomain,    "Leucine Zipper Domain");
    featureTypes.insert(LipidMisc,                  "Lipid: Misc");
    featureTypes.insert(Loci,                   "Loci");
    featureTypes.insert(Ltr,                    "LTR");
    featureTypes.insert(MRna,                   "mRNA");
    featureTypes.insert(MatureChain,                "Mature chain");
    featureTypes.insert(MaturePeptide,          "Mature Peptide");
    featureTypes.insert(MetalMisc,                  "Metal: Misc");
    featureTypes.insert(Methylation,                "Methylation");
    featureTypes.insert(Minus10Signal,          "-10 Signal");
    featureTypes.insert(Minus35Signal,          "-35 Signal");
    featureTypes.insert(MiscBindingSite,        "Misc. Binding Site");
    featureTypes.insert(MiscDifference,         "Misc. Difference");
    featureTypes.insert(MiscFeature,    "Misc. Feature");
    featureTypes.insert(MiscMarker,             "Misc. Marker");
    featureTypes.insert(MiscRecombination,      "Misc. Recombination");
    featureTypes.insert(MiscRna,                "Misc. RNA");
    featureTypes.insert(MiscSignal,             "Misc. Signal");
    featureTypes.insert(MiscStructure,          "Misc. Structure");
    featureTypes.insert(MobileElement,           "Mobile element");
    featureTypes.insert(ModifiedBase,           "Modified Base");
    featureTypes.insert(Mutation,               "Mutation");
    featureTypes.insert(Myristate,                  "Myristate");
    featureTypes.insert(NAcylDiglyceride,           "N-Acyl Diglyceride");
    featureTypes.insert(NcRna,                "ncRNA");
    featureTypes.insert(NRegion,                "N-Region");
    featureTypes.insert(NonConsecutiveResidues,     "Non Consecutive Residues");
    featureTypes.insert(NpBindingRegionMisc,        "NP binding region: Misc");
    featureTypes.insert(Nuclease,                   "Nuclease");
    featureTypes.insert(OldSequence,            "Old Sequence");
    featureTypes.insert(Operon,                 "Operon");
    featureTypes.insert(OriT,                   "oriT");
    featureTypes.insert(Overhang,               "Overhang");
    featureTypes.insert(Palmitate,                  "Palmitate");
    featureTypes.insert(Periplasmic,                "Periplasmic");
    featureTypes.insert(Ph,                         "PH");
    featureTypes.insert(Phosphorylation,            "Phosphorylation");
    featureTypes.insert(PolyASignal,            "PolyA Signal");
    featureTypes.insert(PolyASite,              "PolyA Site");
    featureTypes.insert(PolyAa,                     "Poly-AA");
    featureTypes.insert(Precursor,                  "Precursor");
    featureTypes.insert(PrecursorRna,           "Precursor RNA");
    featureTypes.insert(Primer,                 "Primer");
    featureTypes.insert(PrimerBindingSite,      "Primer Binding Site");
    featureTypes.insert(PrimeryTranscript,      "Primery Transcript");
    featureTypes.insert(ProcessedActivePeptide,     "Processed active peptide");
    featureTypes.insert(Promoter,     "Promoter");
    featureTypes.insert(PromoterEukaryotic,     "Promoter Eukaryotic");
    featureTypes.insert(PromoterProkaryotic,    "Promoter Prokaryotic");
    featureTypes.insert(Propeptide,                 "Propeptide");
    featureTypes.insert(Proprotein,                 "Proprotein");
    featureTypes.insert(Protease,                   "Protease");
    featureTypes.insert(Protein,                "Protein");
    featureTypes.insert(ProteinBindingSite,     "Protein Binding Site");
    featureTypes.insert(Provirus,               "Provirus");
    featureTypes.insert(PyridoxalPhBindingSite,     "Pyridoxal Ph. Binding Site");
    featureTypes.insert(PyrrolidoneCarboxylicAcid,  "Pyrrolidone Carboxylic Acid");
    featureTypes.insert(RRna,                   "rRNA");
    featureTypes.insert(Rbs,                    "RBS");
    featureTypes.insert(Region,                 "Region");
    featureTypes.insert(RegionMisc,                 "Region: Misc");
    featureTypes.insert(Regulatory,           "Regulatory");
    featureTypes.insert(RepeatRegion,           "Repeat Region");
    featureTypes.insert(RepeatUnit,             "Repeat Unit");
    featureTypes.insert(RepetitiveRegion,           "Repetitive region");
    featureTypes.insert(ReplicationOrigin,      "Replication Origin");
    featureTypes.insert(ResidueModificationMisc,    "Residue Modification: Misc");
    featureTypes.insert(SRegion,                "S-Region");
    featureTypes.insert(Satellite,              "Satellite");
    featureTypes.insert(ScRna,                  "scRNA");
    featureTypes.insert(SeconadaryStructure,        "Seconadary structure");
    featureTypes.insert(Sh2,                        "SH2");
    featureTypes.insert(Sh3,                        "SH3");
    featureTypes.insert(SignalPeptide,          "Signal Peptide");
    featureTypes.insert(SignalSequence,             "Signal Sequence");
    featureTypes.insert(Silencer,               "Silencer");
    featureTypes.insert(Similarity,                 "Similarity");
    featureTypes.insert(Site,                   "Site");
    featureTypes.insert(SiteMisc,                   "Site: Misc");
    featureTypes.insert(SnRna,                  "snRNA");
    featureTypes.insert(SnoRna,                 "snoRNA");
    featureTypes.insert(Source,                 "Source");
    featureTypes.insert(SplicingSignal,         "Splicing Signal");
    featureTypes.insert(SplicingVariant,            "Splicing Variant");
    featureTypes.insert(StemLoop,               "Stem Loop");
    featureTypes.insert(Sts,                    "STS");
    featureTypes.insert(Sulfatation,                "Sulfatation");
    featureTypes.insert(TRna,                   "tRNA");
    featureTypes.insert(TataSignal,             "TATA Signal");
    featureTypes.insert(Telomere,               "Telomere");
    featureTypes.insert(Terminator,             "Terminator");
    featureTypes.insert(Thioether,                  "Thioether");
    featureTypes.insert(Thiolester,                 "Thiolester");
    featureTypes.insert(ThreePrimeClip,         "3' Clip");
    featureTypes.insert(ThreePrimeUtr,          "3' UTR");
    featureTypes.insert(TmRna,                  "tmRNA");
    featureTypes.insert(TransitPeptide, "Transit Peptide");
    featureTypes.insert(TransmembraneRegion,        "Transmembrane Region");
    featureTypes.insert(Transposon,             "Transposon");
    featureTypes.insert(Uncertainty,                "Uncertainty");
    featureTypes.insert(Unsure,                 "Unsure");
    featureTypes.insert(VRegion,                "V-Region");
    featureTypes.insert(VSegment,               "V-Segment");
    featureTypes.insert(Variant,                    "Variant");
    featureTypes.insert(Variation,      "Variation");
    featureTypes.insert(Virion,                 "Virion");
    featureTypes.insert(ZincFinger,                 "Zinc Finger");
    featureTypes.insert(ZincFingerDomain,       "Zinc Finger Domain");

    return featureTypes;
}

}   // namespace U2
