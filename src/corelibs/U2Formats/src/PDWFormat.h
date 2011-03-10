#ifndef _U2_PDW_FORMAT_H_
#define _U2_PDW_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;
class Annotation;
class AnnotationTableObject;
class DNASequenceObject;

class U2FORMATS_EXPORT  PDWFormat : public DocumentFormat {
    Q_OBJECT
public:
    PDWFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::PDW;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);
        
    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    
    static QByteArray parseSequence(IOAdapter* io, TaskStateInfo& ti);

    static Annotation* parseAnnotation(IOAdapter* io, TaskStateInfo& ti);

    static QByteArray readPdwValue(const QByteArray& readBuf, const QByteArray& valueName);

    void load(IOAdapter* io, const GUrl& docUrl, QList<GObject*>& objects, TaskStateInfo& ti, 
        DNASequenceObject* dnaObj, AnnotationTableObject* aObj);
    
    QString formatName;

};

}//namespace

#endif
