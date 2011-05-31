include (U2Formats.pri)

# Input
HEADERS += src/ABIFormat.h \
           src/ACEFormat.h \
           src/ASNFormat.h \
           src/ClustalWAlnFormat.h \
           src/DocumentFormatUtils.h \
           src/EMBLGenbankAbstractDocument.h \
           src/EMBLPlainTextFormat.h \
           src/FastaFormat.h \
           src/FastqFormat.h \
           src/GenbankFeatures.h \
           src/GenbankLocationParser.h \
           src/GenbankPlainTextFormat.h \
           src/GFFFormat.h \
           src/IndexFormat.h \
           src/IOLibUtils.h \
           src/MegaFormat.h \
           src/MSFFormat.h \
           src/NewickFormat.h \
           src/NEXUSFormat.h \
           src/NEXUSParser.h \
           src/PDBFormat.h \
           src/PDWFormat.h \
           src/PlainTextFormat.h \
           src/RawDNASequenceFormat.h \
           src/SAMFormat.h \
           src/SCFFormat.h \
           src/SRFastaFormat.h \
           src/StdResidueDictionary.h \
           src/StockholmFormat.h \
           src/StreamSequenceReader.h \
           src/StreamSequenceWriter.h \
           src/SwissProtPlainTextFormat.h \
           src/tasks/ConvertAssemblyToSamTask.h
SOURCES += src/ABIFormat.cpp \
           src/ACEFormat.cpp \
           src/ASNFormat.cpp \
           src/ClustalWAlnFormat.cpp \
           src/DocumentFormatUtils.cpp \
           src/EMBLGenbankAbstractDocument.cpp \
           src/EMBLPlainTextFormat.cpp \
           src/FastaFormat.cpp \
           src/FastqFormat.cpp \
           src/GenbankFeatures.cpp \
           src/GenbankLocationParser.cpp \
           src/GenbankPlainTextFormat.cpp \
           src/GFFFormat.cpp \
           src/IndexFormat.cpp \
           src/MegaFormat.cpp \
           src/MSFFormat.cpp \
           src/NewickFormat.cpp \
           src/NEXUSFormat.cpp \
           src/PDBFormat.cpp \
           src/PDWFormat.cpp \
           src/PlainTextFormat.cpp \
           src/RawDNASequenceFormat.cpp \
           src/SAMFormat.cpp \
           src/SCFFormat.cpp \
           src/SRFastaFormat.cpp \
           src/StdResidueDictionary.cpp \
           src/StockholmFormat.cpp \
           src/StreamSequenceReader.cpp \
           src/StreamSequenceWriter.cpp \
           src/SwissProtPlainTextFormat.cpp \
           src/tasks/ConvertAssemblyToSamTask.cpp
RESOURCES += U2Formats.qrc
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
