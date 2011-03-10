#ifndef _U2_MEGA_FORMAT_H_
#define _U2_MEGA_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

class U2FORMATS_EXPORT MegaFormat : public DocumentFormat {
Q_OBJECT
public:
    MegaFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const { return BaseDocumentFormats::MEGA; }
    virtual const QString& getFormatName() const { return formatName; }
    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
    virtual void storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io );
    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;
    
private:
    QString formatName;
    void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti);
    void save(IOAdapter* io, Document* d, TaskStateInfo& ti);
    static void skipWhites(IOAdapter *io, QByteArray &line);
    static void readHeader(IOAdapter* io, QByteArray &line, TaskStateInfo &ti);
    static void readTitle(IOAdapter* io, QByteArray &line, TaskStateInfo &ti);
    static bool readName(IOAdapter* io, QByteArray &line, QByteArray &name, TaskStateInfo &ti);
    static bool readSequence(IOAdapter* io, QByteArray &line, TaskStateInfo &ti,
                             QByteArray &value, bool *lastIteration);

    static void workUpIndels(MAlignment & al);
    static bool getNextLine(IOAdapter* io, QByteArray& line);
    static bool skipComments(IOAdapter* io, QByteArray &line, TaskStateInfo &ti);
    static bool checkName(QByteArray &name);
    static const QByteArray MEGA_HEADER;
    static const char MEGA_SEPARATOR;
    static const QByteArray MEGA_TITLE;
    static const QByteArray MEGA_UGENE_TITLE;
    static const char MEGA_IDENTICAL;
    static const char MEGA_INDEL;
    static const char MEGA_START_COMMENT;
    static const char MEGA_END_COMMENT;
    static const int BLOCK_LENGTH=45;
};

} //namespace

#endif
