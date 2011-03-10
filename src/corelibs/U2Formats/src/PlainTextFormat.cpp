#include "PlainTextFormat.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/Task.h>

#include <U2Core/TextObject.h>
#include <U2Core/TextUtils.h>


namespace U2 {

/* TRANSLATOR U2::IOAdapter */    

PlainTextFormat::PlainTextFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_W1, QStringList("txt")) 
{
    formatName = tr("Plain text");
    supportedObjectTypes+=GObjectTypes::TEXT;
}


Document* PlainTextFormat::createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs) {
    Document* d = DocumentFormat::createNewDocument(io, url, fs);
    GObject* o = new TextObject("", "Text");
    d->addObject(o);
    return d;
}

#define BUFF_SIZE 1024

Document* PlainTextFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode) {
    QString text;
    int size = io->left();
    if (size > 0) {
        text.reserve(size);
    }
    QByteArray block(BUFF_SIZE, '\0');
    int blockLen = 0;
    while ((blockLen = io->readBlock(block.data(), BUFF_SIZE)) > 0) {
        int sizeBefore = text.length();
        QString line = QString::fromLocal8Bit(block.data(), blockLen);
        text.append(line);
        if (text.length() != sizeBefore + blockLen) {
            ti.setError(L10N::errorReadingFile(io->getURL()));
            break;
        }
        ti.progress = io->getProgress();
    }
    
    if (ti.hasErrors()) {
        return NULL;
    }

    //todo: check file-readonly status?

    TextObject* to = new TextObject(text, "Text");
    QList<GObject*> objects;
    objects.append(to);
    Document* d = new Document(this, io->getFactory(), io->getURL(), objects, fs);
    return d;
}

void PlainTextFormat::storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io) {
    assert(d->getObjects().size() ==1);
    GObject* obj = d->getObjects().first();
    TextObject* to = qobject_cast<TextObject*>(obj);
    assert(to!=NULL);
    const QString& text = to->getText();

    QByteArray local8bit = text.toLocal8Bit();
    storeRawData(local8bit, ts, io);
}

void PlainTextFormat::storeRawData(const QByteArray& rawData, TaskStateInfo& ts, IOAdapter* io) {
    int nWritten = 0;
    int nTotal = rawData.size();
    while(nWritten < nTotal) {
        int l = io->writeBlock(rawData.data() + nWritten, nTotal - nWritten);
        if (l <= 0 ) {
            ts.setError(L10N::errorWritingFile(io->getURL()));
            return;
        }
        nWritten+= l;
    }
}


FormatDetectionResult PlainTextFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    bool hasBinaryData = TextUtils::contains(TextUtils::BINARY, data, size);
    return hasBinaryData ? FormatDetection_NotMatched :FormatDetection_LowSimilarity;
}

}//namespace
