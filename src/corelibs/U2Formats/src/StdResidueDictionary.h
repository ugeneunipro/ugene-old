#ifndef _U2_STD_RESIDUE_DICTIONARY_H_
#define _U2_STD_RESIDUE_DICTIONARY_H_

#include <QByteArray>
#include <QHash>
#include <memory>

namespace U2 {

    class AsnNode;

    enum ResidueType {
        DEOXYRIBONUCLEOTIDE, RIBONUCLEOTIDE, AMINO_ACID, OTHER = 255  
    };
    
    struct StdBond {
        int atom1Id;
        int atom2Id;
    };
    
    struct StdAtom {
        QByteArray name;
        int atomicNum;
    };
    
    struct StdResidue {
        QByteArray name;
        ResidueType type;
        char code;
        QHash<int, StdAtom> atoms;
        QList<StdBond> bonds;
    };

    class StdResidueDictionary {
        QHash<int, StdResidue> residues;
        bool load(const QString& fileName);
        void buildDictionaryFromAsnTree(AsnNode* rootElem);
        bool validate() const;
        static QMutex standardDictionaryLock;
        static std::auto_ptr<StdResidueDictionary> standardDictionary;
    public:
        ~StdResidueDictionary();
        static StdResidueDictionary* createStandardDictionary();
        static StdResidueDictionary* createFromAsnTree(AsnNode* rootElem);
        //TODO : move this method to AppContext
        static const StdResidueDictionary* getStandardDictionary();
        static ResidueType getResidueTypeByName(const QByteArray& name);
        const StdResidue getResidueById(int id) const;
    };

} // namespace

#endif // _U2_STD_RESIDUE_DICTIONARY_H_
