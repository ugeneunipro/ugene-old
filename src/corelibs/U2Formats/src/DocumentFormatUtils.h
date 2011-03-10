#ifndef _U2_DOCUMENT_FORMAT_UTILS_H_
#define _U2_DOCUMENT_FORMAT_UTILS_H_

#include <U2Core/U2Region.h>

#include <QtCore/QStringList>

//todo: move to separate header
#define MERGE_MULTI_DOC_GAP_SIZE_SETTINGS           "merge_gap"
#define MERGE_MULTI_DOC_SEQUENCE_SIZE_SETTINGS      "merge_size"

namespace U2  {

class Document;
class DocumentFormat;
class DNAAlphabet;
class DNASequenceObject;
class GObject;
class MAlignment;
class DNASequence;
class GUrl;
class AnnotationSettings;

class U2FORMATS_EXPORT DocumentFormatUtils {
public:
    static DNASequenceObject* addSequenceObject(QList<GObject*>& objects, const QString& name, DNASequence& seq);

    /** if no docURL provided -> relations are not set*/
    static DNASequenceObject* addMergedSequenceObject(QList<GObject*>& objects, const GUrl& docUrl, const QStringList& contigs, 
                            QByteArray& mergedSequence, const QVector<U2Region>& mergedMapping);

    static DNAAlphabet* findAlphabet(const QByteArray& arr);
    
    static QList<DNAAlphabet*> findAlphabets(const QByteArray& arr);

    static DNAAlphabet* findAlphabet(const QByteArray& arr, const QVector<U2Region>& regionsToProcess);

    static void trySqueeze(QByteArray& a);

    static int getIntSettings(const QVariantMap& fs, const char* sName, int defVal);

    static void updateFormatSettings(QList<GObject*>& objects, QVariantMap& fs);

    static QList<DocumentFormatId> toIds(const QList<DocumentFormat*>& formats);

    static void assignAlphabet(MAlignment& ma);

    static void assignAlphabet(MAlignment& ma, char ignore);

    static QList<AnnotationSettings*> predefinedSettings();
};

}//namespace

#endif
