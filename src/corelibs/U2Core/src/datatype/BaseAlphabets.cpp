#include "DNAAlphabetRegistryImpl.h"

#include <U2Core/TextUtils.h>

namespace U2 {


static void fillBitArray(QBitArray& map, const char* str, Qt::CaseSensitivity caseMode) {
    QByteArray line(str);
    foreach(char c, line) {
        if (caseMode == Qt::CaseSensitive) {
            map[(uchar)c]=true;
        } else {
            char lc = TextUtils::UPPER_CASE_MAP.at((uchar)c);
            char uc = TextUtils::LOWER_CASE_MAP.at((uchar)c);
            map[(uchar)lc] = true;
            map[(uchar)uc] = true;
        }
    }
}
#define CASE_OFFSET ('a'-'A')
static void fill(QBitArray& map, const char *s, Qt::CaseSensitivity caseMode) {
    for(const char* i =s; *i!=0; i++) {
        char c = *i;
        map[int((uchar)c)] = true;
        if (caseMode == Qt::CaseInsensitive && c >='A' && c<='Z') {
            map[int((uchar)c) + CASE_OFFSET] = true;
        }
    }
}

#define REG(a) \
    map[int((uchar)a)] = true; \
    if (caseMode == DNAAlphabet::CaseInsensitive) { \
        map[int((uchar)a) + CASE_OFFSET] = true; \
    }

void DNAAlphabetRegistryImpl::initBaseAlphabets() {
    //raw text
    {
        QBitArray map(256, true);
        DNAAlphabet* a = new DNAAlphabet(BaseDNAAlphabetIds::RAW(), tr("All symbols"), DNAAlphabet_RAW, map, Qt::CaseSensitive, '\0');
        registerAlphabet(a);
    }

    //default DNA nucl
    {
        QBitArray map(256, false);
        fill(map, "ACGTN-", Qt::CaseInsensitive);
        DNAAlphabet* a = new DNAAlphabet(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), tr("Standard DNA"),
                                        DNAAlphabet_NUCL, map, Qt::CaseInsensitive, 'N');
        registerAlphabet(a);
    }

    //default RNA nucl
    {
        QBitArray map(256, false);
        fill(map, "ACGUN-", Qt::CaseInsensitive);
        DNAAlphabet* a = new DNAAlphabet(BaseDNAAlphabetIds::NUCL_RNA_DEFAULT(), tr("Standard RNA"), 
            DNAAlphabet_NUCL, map, Qt::CaseInsensitive, 'N');
        registerAlphabet(a);
    }


    //extended NUCL DNA
    {
        QBitArray map(256, false);
        fill(map, "ACGTMRWSYKVHDBNX-", Qt::CaseInsensitive); //X == N
        DNAAlphabet* a = new DNAAlphabet(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED(), tr("Extended DNA"),
                                        DNAAlphabet_NUCL, map, Qt::CaseInsensitive, 'N');
        registerAlphabet(a);
    }

    //extended NUCL RNA
    {
        QBitArray map(256, false);
        fill(map, "ACGUMRWSYKVHDBNX-", Qt::CaseInsensitive); //X == N
        DNAAlphabet* a = new DNAAlphabet(BaseDNAAlphabetIds::NUCL_RNA_EXTENDED(), tr("Extended RNA"), 
            DNAAlphabet_NUCL, map, Qt::CaseInsensitive, 'N');
        registerAlphabet(a);
    }

    //default amino
    {
        QBitArray map(256, false);
        // 
        // http://www.jbc.org/cgi/reprint/243/13/3557.pdf IUPAC-IUB Commission on Biochemical Nomenclature
        fillBitArray(map, "ABCDEFGHIKLMNPQRSTVWXYZ*X-", Qt::CaseInsensitive);
        // http://en.wikipedia.org/wiki/Genetic_code#Variations_to_the_standard_genetic_code
        //O = pyrrolysine, U = selenocysteine
        fillBitArray(map, "OU", Qt::CaseInsensitive);

        DNAAlphabet* a = new DNAAlphabet(BaseDNAAlphabetIds::AMINO_DEFAULT(), tr("Standard amino"), 
            DNAAlphabet_AMINO, map, Qt::CaseInsensitive, 'X');
        registerAlphabet(a);
    }

}



}//namespace
