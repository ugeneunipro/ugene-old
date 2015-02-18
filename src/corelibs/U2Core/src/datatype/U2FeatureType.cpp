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

const QList<U2FeatureTypes::U2FeatureTypeInfo> U2FeatureTypes::typeInfos = U2FeatureTypes::initFeatureTypes();

QList<U2FeatureTypes::U2FeatureType> U2FeatureTypes::getTypes(Alphabets alphabets) {
    QList<U2FeatureTypes::U2FeatureType> types;
    foreach (const U2FeatureTypeInfo &info, typeInfos) {
        if (info.alphabets & alphabets) {
            types << info.featureType;
        }
    }
    return types;
}

QString U2FeatureTypes::getVisualName(U2FeatureType type) {
    foreach (const U2FeatureTypeInfo &info, typeInfos) {
        if (info.featureType == type) {
            return info.visualName;
        }
    }

    return "";
}

U2FeatureType U2FeatureTypes::getTypeByName(const QString &visualName) {
    foreach (const U2FeatureTypeInfo &info, typeInfos) {
        if (info.visualName == visualName) {
            return info.featureType;
        }
    }

    return Invalid;
}

QList<U2FeatureTypes::U2FeatureTypeInfo> U2FeatureTypes::initFeatureTypes() {
    QList<U2FeatureTypeInfo> featureTypeInfos;

    featureTypeInfos << U2FeatureTypeInfo(AaRich, "AA-Rich", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Acetylation, "Acetylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(ActiveSite, "Active Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Adenylation, "Adenylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Allele, "Allele", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(AlphaHelix, "Alpha-Helix", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(AlteredSite, "Altered Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Amidation, "Amidation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(AssemblyGap, "Assembly gap", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Attenuator, "Attenuator", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(BHlhDomain, "bHLH Domain", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Basic, "Basic", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(BetaSheet, "Beta-Sheet", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(BetaStrandRegion, "Beta-strand region", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(BiotinBindingSite, "Biotin Binding Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Blocked, "Blocked", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(C2, "C2", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(CRegion, "C-Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(CaatSignal, "CAAT Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Calcium, "Calcium", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(CatalyticRegion, "Catalytic Region", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Cds, "CDS", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(CellAttachment, "Cell Attachment", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Cellular, "Cellular", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Centromere, "Centromere", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(CholesterolBindingSite, "Cholesterol Binding Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(CleavageSite, "Cleavage Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(CoiledCoil, "Coiled coil", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(CollagenType, "Collagen-type", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Comment, "Comment", Alphabet_None);
    featureTypeInfos << U2FeatureTypeInfo(Conflict, "Conflict", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(ConnectingPeptide, "Connecting Peptide", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Cub, "CUB", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Cytoplasmic, "Cytoplasmic", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(DLoop, "D-Loop", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(DSegment, "D-Segment", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Disulfide, "Disulfide", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Egf, "EGF", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Enhancer, "Enhancer", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Exon, "Exon", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Exoplasmic, "Exoplasmic", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Extracellular, "Extracellular", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Farnesyl, "Farnesyl", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Fibronectin, "Fibronectin", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(FivePrimeClip, "5' Clip", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(FivePrimeUtr, "5' UTR", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Formylation, "Formylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(GammaCarboxyglumaticAcid, "Gamma-Carboxyglumatic Acid", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Gap, "Gap", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(GcSignal, "GC-Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Gene, "Gene", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(GeranylGeranyl, "Geranyl-Geranyl", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Glycosylation, "Glycosylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(GlycosylationSite, "Glycosylation Site", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(GpiAnchor, "GPI-Anchor", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(HelicalRegion, "Helical region", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(HemeBindingSite, "Heme Binding Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(HmgBox, "HMG-Box", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Homeodomain, "Homeodomain", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Hth, "H-T-H", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(HydrogenBondedTurn, "Hydrogen bonded turn", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Hydroxylation, "Hydroxylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(IDna, "iDNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Immunoglobulin, "Immunoglobulin", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Insertion, "Insertion", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Intracellular, "Intracellular", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Intron, "Intron", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(JRegion, "J-Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(JSegment, "J-Segment", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Kh, "KH", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Kinase, "Kinase", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(LeucineZipper, "Leucine Zipper", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(LeucineZipperDomain, "Leucine Zipper Domain", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Loci, "Loci", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Ltr, "LTR", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MRna, "mRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MatureChain, "Mature chain", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MaturePeptide, "Mature Peptide", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Methylation, "Methylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Minus10Signal, "-10 Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Minus35Signal, "-35 Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MiscBindingSite, "Misc. Binding Site", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscBond, "Bond: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscDifference, "Misc. Difference", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MiscDnaRnaBindingRegion, "DNA/RNA binding region: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscDomain, "Domain: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscFeature, "Misc. Feature", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscLipid, "Lipid: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscMarker, "Misc. Marker", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MiscMetal, "Metal: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscNpBindingRegion, "NP binding region: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscRecombination, "Misc. Recombination", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MiscRegion, "Region: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscResidueModification, "Residue Modification: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscRna, "Misc. RNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MiscSignal, "Misc. Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MiscSite, "Site: Misc", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(MiscStructure, "Misc. Structure", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(MobileElement, "Mobile element", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(ModifiedBase, "Modified Base", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Mutation, "Mutation", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Myristate, "Myristate", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(NAcylDiglyceride, "N-Acyl Diglyceride", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(NRegion, "N-Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(NcRna, "ncRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(NonConsecutiveResidues, "Non Consecutive Residues", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Nuclease, "Nuclease", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(OldSequence, "Old Sequence", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Operon, "Operon", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(OriT, "oriT", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Overhang, "Overhang", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Palmitate, "Palmitate", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Periplasmic, "Periplasmic", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Ph, "PH", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Phosphorylation, "Phosphorylation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(PolyASignal, "PolyA Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PolyASite, "PolyA Site", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PolyAa, "Poly-AA", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Precursor, "Precursor", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(PrecursorRna, "Precursor RNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Primer, "Primer", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PrimerBindingSite, "Primer Binding Site", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PrimeryTranscript, "Primery Transcript", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(ProcessedActivePeptide, "Processed active peptide", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Promoter, "Promoter", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PromoterEukaryotic, "Promoter Eukaryotic", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PromoterProkaryotic, "Promoter Prokaryotic", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Propeptide, "Propeptide", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Proprotein, "Proprotein", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Protease, "Protease", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Protein, "Protein", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(ProteinBindingSite, "Protein Binding Site", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Provirus, "Provirus", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(PyridoxalPhBindingSite, "Pyridoxal Ph. Binding Site", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(PyrrolidoneCarboxylicAcid, "Pyrrolidone Carboxylic Acid", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(RRna, "rRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Rbs, "RBS", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Region, "Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Regulatory, "Regulatory", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(RepeatRegion, "Repeat Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(RepeatUnit, "Repeat Unit", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(RepetitiveRegion, "Repetitive region", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(ReplicationOrigin, "Replication Origin", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(SRegion, "S-Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Satellite, "Satellite", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(ScRna, "scRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(SeconadaryStructure, "Seconadary structure", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Sh2, "SH2", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Sh3, "SH3", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(SignalPeptide, "Signal Peptide", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(SignalSequence, "Signal Sequence", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Silencer, "Silencer", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Similarity, "Similarity", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Site, "Site", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(SnRna, "snRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(SnoRna, "snoRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Source, "Source", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(SplicingSignal, "Splicing Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(SplicingVariant, "Splicing Variant", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(StemLoop, "Stem Loop", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Sts, "STS", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Sulfatation, "Sulfatation", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(TRna, "tRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(TataSignal, "TATA Signal", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Telomere, "Telomere", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Terminator, "Terminator", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Thioether, "Thioether", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Thiolester, "Thiolester", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(ThreePrimeClip, "3' Clip", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(ThreePrimeUtr, "3' UTR", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(TmRna, "tmRNA", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(TransitPeptide, "Transit Peptide", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(TransmembraneRegion, "Transmembrane Region", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Transposon, "Transposon", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Uncertainty, "Uncertainty", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Unsure, "Unsure", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(VRegion, "V-Region", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(VSegment, "V-Segment", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(Variant, "Variant", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Variation, "Variation", Alphabet_Nucleic | Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(Virion, "Virion", Alphabet_Nucleic);
    featureTypeInfos << U2FeatureTypeInfo(ZincFinger, "Zinc Finger", Alphabet_Amino);
    featureTypeInfos << U2FeatureTypeInfo(ZincFingerDomain, "Zinc Finger Domain", Alphabet_Nucleic);

    return featureTypeInfos;
}

U2FeatureTypes::U2FeatureTypeInfo::U2FeatureTypeInfo(U2FeatureType featureType, const QString &visualName, Alphabets alphabets) :
    featureType(featureType),
    visualName(visualName),
    alphabets(alphabets)
{

}

}   // namespace U2
