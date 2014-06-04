/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QFileInfo>

extern "C" {
#include <bam.h>
#include <bam_sort.c>
#include <bam_rmdup.c>
#include <kseq.h>
#include <sam.h>
#include <sam_header.h>
#include <bgzf.h>
}

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2CoreAttributes.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include <SamtoolsAdapter.h>

#include "BAMUtils.h"

namespace U2 {

BAMUtils::ConvertOption::ConvertOption(bool samToBam, const QString &referenceUrl)
: samToBam(samToBam), referenceUrl(referenceUrl)
{

}

namespace {
    void closeFiles(samfile_t *in, samfile_t *out) {
        if (NULL != in) {
            samclose(in);
        }

        if (NULL != out) {
            samclose(out);
        }
    }

    QString openFileError(const QByteArray &file) {
        return QObject::tr("Fail to open \"%1\" for reading").arg(file.constData());
    }

    QString headerError(const QByteArray &file) {
        return QObject::tr("Fail to read the header from the file: \"%1\"").arg(file.constData());
    }

    QString faiError(const QByteArray &file) {
        return QObject::tr("Can not build the fasta index for the file: \"%1\"").arg(file.constData());
    }

    QString readsError(const QByteArray &file) {
        return QObject::tr("Error parsing the reads from the file: \"%1\"").arg(file.constData());
    }

    QString truncatedError(const QByteArray &file) {
        return QObject::tr("Truncated file: \"%1\"").arg(file.constData());
    }

    void samreadCheck(int read, U2OpStatus &os, const QByteArray& fileName)
    {
        if (READ_ERROR_CODE == read) {
            if (NULL != SAMTOOLS_ERROR_MESSAGE) {
                os.setError(SAMTOOLS_ERROR_MESSAGE);
            } else {
                os.setError(readsError(fileName));
            }
        } else if (read < -1) {
            os.setError(truncatedError(fileName));
        }
    }
    QList<QByteArray> convertStringList(const QStringList& sList){
        QList<QByteArray> result;
        for(int i = 0; i < sList.size(); i++){
            result.append(sList.at(i).toLocal8Bit());
        }
        return result;
    }
    void convertByteArray(const QList<QByteArray>& byteArray, char ** charArray){
        for(int i = 0; i < byteArray.size(); i++){
            charArray[i] = const_cast<char*>(byteArray[i].constData());
        }
    }
}

#define SAMTOOL_CHECK(cond, msg, ret) \
    if (!(cond)) {\
        if (NULL != SAMTOOLS_ERROR_MESSAGE) { \
            os.setError(SAMTOOLS_ERROR_MESSAGE); \
        } else { \
            os.setError(msg); \
        } \
        closeFiles(in, out); \
        return ret; \
    } \


void BAMUtils::convertToSamOrBam(const GUrl &samUrl, const GUrl &bamUrl, const ConvertOption &options, U2OpStatus &os ) {
    const QByteArray samFileName = samUrl.getURLString().toLocal8Bit();
    const QByteArray bamFileName = bamUrl.getURLString().toLocal8Bit();

    QByteArray sourceName = (options.samToBam)? samFileName : bamFileName;
    QByteArray targetName = (options.samToBam)? bamFileName : samFileName;

    samfile_t *in = NULL;
    samfile_t *out = NULL;

    // open files
    {
        QByteArray readMode = ( options.samToBam ) ? "r" : "rb";
        void *aux = NULL;
        if (options.samToBam && !options.referenceUrl.isEmpty()) {
            aux = samfaipath(options.referenceUrl.toLocal8Bit().constData());
            SAMTOOL_CHECK(NULL != aux, faiError(options.referenceUrl.toLocal8Bit()), );
        }

        in = samopen(sourceName.constData(), readMode, aux);
        SAMTOOL_CHECK(NULL != in, openFileError(sourceName), );
        SAMTOOL_CHECK(NULL != in->header, headerError(sourceName), );

        QByteArray writeMode = ( options.samToBam ) ? "wb" : "wh";
        out = samopen(targetName.constData(), writeMode, in->header);
        SAMTOOL_CHECK(NULL != out, openFileError(targetName), );
    }
    // convert files
    bam1_t *b = bam_init1();
    int r;
    {
        while ((r = samread(in, b)) >= 0) { // read one alignment from `in'
            samwrite(out, b); // write the alignment to `out'
        }

        samreadCheck(r, os, sourceName);
        bam_destroy1(b);
    }

    closeFiles(in, out);
    return;
}

static bool isSorted(const QString &headerText) {
    QString text = headerText;
    text = text.replace('\r', QString(""));
    QStringList lines = text.split('\n');

    bool result = false;
    foreach (const QString &line, lines) {
        if (!line.startsWith("@HD")) {
            continue;
        }

        QStringList tokens = line.split('\t');
        for (int i = 1; i < tokens.size(); i++) {
            QString &token = tokens[i];
            int colonIdx = token.indexOf(':');
            if(-1 == colonIdx) {
                continue;
            }
            QString fieldTag = token.mid(0, colonIdx);
            QString fieldValue = token.mid(colonIdx + 1);

            if ("SO" == fieldTag) {
                result = ("coordinate" == fieldValue);
                break;
            }
        }
    }

    return result;
}

bool BAMUtils::isSortedBam(const GUrl &bamUrl, U2OpStatus &os) {
    const QByteArray &bamFileName = bamUrl.getURLString().toLocal8Bit();

    bamFile bamHandler = NULL;
    bam_header_t *header = NULL;
    QString error;
    bool result = false;

    bamHandler = bam_open(bamFileName.constData(), "r");
    if (NULL != bamHandler) {
        header = bam_header_read(bamHandler);
        if (NULL != header) {
            result = isSorted(header->text);
        } else {
            error = QString("Can't read header from file '%1'").arg(bamFileName.constData());
        }
    } else {
        error = QString("Can't open file '%1'").arg(bamFileName.constData());
    }

    // deallocate resources
    {
        if (NULL != header) {
            bam_header_destroy(header);
        }
        if (NULL != bamHandler) {
            bam_close(bamHandler);
        }
    }

    if (!error.isEmpty()) {
        os.setError(error);
        return false;
    }

    /**
     * There is a bug in samtools. If you sort a BAM using samtools then
     * the header tag "SO" is not changed. So, if the check above is false
     * then maybe it is not true that the file is not sorted.
     */
    if (!result) {
        result = BAMUtils::hasValidBamIndex(bamUrl);
        if (!result) {
            U2OpStatusImpl idxOs;
            BAMUtils::createBamIndex(bamUrl, idxOs);
            CHECK_OP(idxOs, false);
        }
    }

    return true;
}

inline static int bytes2MB(qint64 bytes) {
    return (int) (bytes / (1024 * 1024)) + 1;
}

inline static qint64 mB2bytes(int mb) {
    return (qint64)mb * 1024 * 1024;
}

#define INITIAL_SAMTOOLS_MEM_SIZE_MB 500
#define SAMTOOLS_MEM_BOOST 5

GUrl BAMUtils::sortBam(const GUrl &bamUrl, const QString &sortedBamBaseName, U2OpStatus &os) {
    const QByteArray &bamFileName = bamUrl.getURLString().toLocal8Bit();

    QString baseName = sortedBamBaseName;
    if(baseName.endsWith(".bam")){
        baseName = baseName.left(baseName.size() - QString(".bam").size());
    }
    QByteArray sortedFileName = baseName.toLocal8Bit() + ".bam";


    // get memory resource
    AppSettings *appSettings = AppContext::getAppSettings();
    AppResourcePool *resPool = appSettings->getAppResourcePool();
    AppResource *memory = resPool->getResource(RESOURCE_MEMORY);
    SAFE_POINT_EXT(NULL != memory, os.setError("No memory resource"), QString());

    // calculate needed memory
    QFileInfo info(bamFileName);
    qint64 fileSizeBytes = info.size();
    CHECK_EXT(fileSizeBytes >= 0, os.setError(QString("Unknown file size: %1").arg(bamFileName.constData())), QString());

    int maxMemMB = INITIAL_SAMTOOLS_MEM_SIZE_MB;
    int fileSizeMB = bytes2MB(fileSizeBytes);
    if( fileSizeMB < 10 ) {
        maxMemMB = fileSizeMB;
    } else if( fileSizeMB < 100 ) {
        maxMemMB = fileSizeMB / SAMTOOLS_MEM_BOOST;
    }
    maxMemMB = qMin( maxMemMB, INITIAL_SAMTOOLS_MEM_SIZE_MB);
    while (!memory->tryAcquire(maxMemMB)) {
        // reduce used memory
        maxMemMB = maxMemMB * 2 / 3;
        CHECK_EXT(maxMemMB > 0, os.setError("Failed to lock enough memory resource"), QString());
    }
    // sort bam
    {
        coreLog.details(BAMUtils::tr("Sort bam file: \"%1\" using %2 Mb of memory. Result sorted file is: \"%3\"")
            .arg(QString(bamFileName)).arg(maxMemMB).arg(QString(sortedFileName)));
        size_t maxMemBytes = (size_t)(mB2bytes(maxMemMB)); // maxMemMB < 500 Mb, so the conversation is correct!
        bam_sort_core(0, bamFileName.constData(), baseName.toLocal8Bit().constData(), maxMemBytes); //maxMemBytes
    }
    memory->release(maxMemMB);

    return QString(sortedFileName);
}

GUrl BAMUtils::mergeBam(const QStringList &bamUrls, const QString &mergetBamTargetUrl, U2OpStatus &/*os*/){
    coreLog.details(BAMUtils::tr("Merging BAM files: \"%1\". Resulting merged file is: \"%2\"")
        .arg(QString(bamUrls.join(","))).arg(QString(mergetBamTargetUrl)));

    int urlsSize = bamUrls.size();
    char** mergeArgv = new char*[urlsSize];
    QList<QByteArray> byteArray = convertStringList(bamUrls);
    convertByteArray(byteArray, mergeArgv);

    bam_merge_core(0, mergetBamTargetUrl.toLocal8Bit().constData(), 0, urlsSize, mergeArgv, 0, 0);

    delete mergeArgv;

    return QString(mergetBamTargetUrl);
}

GUrl BAMUtils::rmdupBam(const QString &bamUrl, const QString &rmdupBamTargetUrl, U2OpStatus &os, bool removeSingleEnd, bool treatReads)
{
    coreLog.details(BAMUtils::tr("Remove PCR duplicate in BAM file: \"%1\". Resulting  file is: \"%2\"")
        .arg(QString(bamUrl)).arg(QString(rmdupBamTargetUrl)));

    int is_se = 0;
    int force_se = 0;
    if(removeSingleEnd){
        is_se = 1;
    }
    if(treatReads){
        is_se = 1;
        force_se = 1;
    }

    const QByteArray& inFileName = bamUrl.toLocal8Bit();
    const QByteArray& outFileName = rmdupBamTargetUrl.toLocal8Bit();
    samfile_t *in = NULL;
    samfile_t *out = NULL;
    {
        in = samopen(inFileName, "rb", 0);
        SAMTOOL_CHECK(NULL != in, openFileError(inFileName), QString(""));
        SAMTOOL_CHECK(NULL != in->header, headerError(inFileName), QString(""));

        out = samopen(outFileName, "wb", in->header);
        SAMTOOL_CHECK(NULL != out, openFileError(outFileName), QString(""));
    }

    if (is_se){
        bam_rmdupse_core(in, out, force_se);
    } else{
        bam_rmdup_core(in, out);
    }

    samclose(in);
    samclose(out);

    return QString(rmdupBamTargetUrl);
}

bool BAMUtils::hasValidBamIndex(const GUrl &bamUrl) {
    const QByteArray bamFileName = bamUrl.getURLString().toLocal8Bit();

    bam_index_t *index = bam_index_load(bamFileName.constData());
    
    if (NULL == index) {
        return false;
    } else {
        bam_index_destroy(index);
        return true;
    }
}

bool BAMUtils::hasValidFastaIndex(const GUrl &fastaUrl) {
    const QByteArray fastaFileName = fastaUrl.getURLString().toLocal8Bit();

    char *index = samfaipath(fastaFileName.constData());

    if (NULL == index) {
        return false;
    } else {
        free(index);
        return true;
    }
}

void BAMUtils::createBamIndex(const GUrl &bamUrl, U2OpStatus &os) {
    const QByteArray &bamFileName = bamUrl.getURLString().toLocal8Bit();

    coreLog.details(BAMUtils::tr("Build index for bam file: \"%1\"").arg(QString(bamFileName)));

    int error = bam_index_build(bamFileName.constData());
    if (-1 == error) {
        os.setError("Can't build the index");
    }
}

static qint64 getSequenceLength(U2Dbi *dbi, const U2DataId &objectId, U2OpStatus &os) {
    qint64 seqLength = -1;

    U2AssemblyDbi *assemblyDbi = dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, os.setError("NULL assembly DBI"), seqLength);

    U2AttributeDbi *attributeDbi = dbi->getAttributeDbi();
    bool useMaxPos = true;
    if (NULL != attributeDbi) {
        U2IntegerAttribute attr = U2AttributeUtils::findIntegerAttribute(attributeDbi, objectId,
            U2BaseAttributeName::reference_length, os);
        CHECK_OP(os, seqLength);
        if (attr.hasValidId()) {
            seqLength = attr.value;
            useMaxPos = false;
        }
    }
    if (useMaxPos) {
        seqLength = assemblyDbi->getMaxEndPos(objectId, os) + 1;
        CHECK_OP(os, seqLength);
    }

    return seqLength;
}

static void createHeader(bam_header_t *header, const QList<GObject*> &objects, U2OpStatus &os) {
    CHECK_EXT(NULL != header, os.setError("NULL header"), );

    header->n_targets = objects.size();
    header->target_name = new char*[header->n_targets];
    header->target_len = new uint32_t[header->n_targets];

    QByteArray headerText;
    headerText += "@HD\tVN:1.4\tSO:coordinate\n";

    int objIdx = 0;
    foreach (GObject *obj, objects) {
        AssemblyObject *assemblyObj = dynamic_cast<AssemblyObject*>(obj);
        SAFE_POINT_EXT(NULL != assemblyObj, os.setError("NULL assembly object"), );

        DbiConnection con(obj->getEntityRef().dbiRef, os);
        CHECK_OP(os, );

        qint64 seqLength = getSequenceLength(con.dbi, obj->getEntityRef().entityId, os);
        CHECK_OP(os, );

        QByteArray seqName = obj->getGObjectName().toLatin1();
        header->target_name[objIdx] = new char[seqName.length() + 1];
        {
            char *name = header->target_name[objIdx];
            qstrncpy(name, seqName.constData(), seqName.length()+1);
            name[seqName.length()] = 0;
        }
        header->target_len[objIdx] = seqLength;

        headerText += QString("@SQ\tSN:%1\tLN:%2\n").arg(seqName.constData()).arg(seqLength);

        objIdx++;
    }

    if (headerText.length() > 0) {
        header->text = new char[headerText.length() + 1];
        qstrncpy(header->text, headerText.constData(), headerText.length() + 1);
        header->text[headerText.length()] = 0;
        header->l_text = headerText.length();
        header->dict = sam_header_parse2(header->text);
    }
}

static QMap<QString, int> getNumMap(const QList<GObject*> &objects, U2OpStatus &os) {
    QMap<QString, int> result;
    int i = 0;
    foreach (GObject *obj, objects) {
        AssemblyObject *assemblyObj = dynamic_cast<AssemblyObject*>(obj);
        SAFE_POINT_EXT(NULL != assemblyObj, os.setError("NULL assembly object"), result);

        QString name = assemblyObj->getGObjectName();
        result[name] = i;
        i++;
    }
    return result;
}

static void writeObjectsWithSamtools(samfile_t *out, const QList<GObject*> &objects, U2OpStatus &os) {
    foreach (GObject *obj, objects) {
        AssemblyObject *assemblyObj = dynamic_cast<AssemblyObject*>(obj);
        SAFE_POINT_EXT(NULL != assemblyObj, os.setError("NULL assembly object"), );

        DbiConnection con(assemblyObj->getEntityRef().dbiRef, os);
        CHECK_OP(os, );

        U2AssemblyDbi *dbi = con.dbi->getAssemblyDbi();
        SAFE_POINT_EXT(NULL != dbi, os.setError("NULL assembly DBI"), );

        U2DataId assemblyId = assemblyObj->getEntityRef().entityId;
        qint64 maxPos = dbi->getMaxEndPos(assemblyId, os);
        U2Region wholeAssembly(0, maxPos + 1);
        QScopedPointer< U2DbiIterator<U2AssemblyRead> > reads(dbi->getReads(assemblyId, wholeAssembly, os, true));
        CHECK_OP(os, );

        ReadsContext ctx(assemblyObj->getGObjectName(), getNumMap(objects, os));
        CHECK_OP(os, );
        bam1_t *read = bam_init1();
        while (reads->hasNext()) {
            U2AssemblyRead r = reads->next();
            SamtoolsAdapter::read2samtools(r, ctx, os, *read);
            CHECK_OP_EXT(os, bam_destroy1(read), );
            samwrite(out, read);
        }
        bam_destroy1(read);
    }
}

void BAMUtils::writeDocument(Document *doc, U2OpStatus &os) {
    writeObjects(
        doc->findGObjectByType(GObjectTypes::ASSEMBLY),
        doc->getURL(),
        doc->getDocumentFormatId(),
        os);
}

void BAMUtils::writeObjects(const QList<GObject*> &objects, const GUrl &urlStr, const DocumentFormatId &formatId, U2OpStatus &os) {
    CHECK_EXT(!objects.isEmpty(), os.setError("No assembly objects"), );

    QByteArray url = urlStr.getURLString().toLocal8Bit();
    CHECK_EXT(!url.isEmpty(), os.setError("Empty file url"), );

    QByteArray openMode("w");
    if (BaseDocumentFormats::BAM == formatId) {
        openMode += "b"; // BAM output
    } else if (BaseDocumentFormats::SAM == formatId) {
        openMode += "h"; // SAM only: write header
    } else {
        os.setError("Only BAM or SAM files could be written");
        return;
    }

    bam_header_t *header = bam_header_init();
    createHeader(header, objects, os);
    if (os.isCoR()) {
        bam_header_destroy(header);
        return;
    }

    samfile_t *out = samopen(url.constData(), openMode.constData(), header);
    bam_header_destroy(header);
    CHECK_EXT(NULL != out, os.setError(QString("Can not open file for writing: %1").arg(url.constData())), );

    writeObjectsWithSamtools(out, objects, os);
    samclose(out);
}

//the function assumes the equal order of alignments in files
bool BAMUtils::isEqualByLength(const GUrl &fileUrl1, const GUrl &fileUrl2, U2OpStatus &os, bool isBAM){
    const QByteArray& fileName1 = fileUrl1.getURLString().toLocal8Bit();
    const QByteArray& fileName2 = fileUrl2.getURLString().toLocal8Bit();

    samfile_t *in = NULL;
    samfile_t *out = NULL;

    QByteArray readMode = "r";
    if(isBAM){
        readMode = "rb";
    }
    {
        void *aux = NULL;
        in = samopen(fileName1.constData(), readMode.constData(), aux);
        SAMTOOL_CHECK(NULL != in, openFileError(fileName1), false);
        SAMTOOL_CHECK(NULL != in->header, headerError(fileName1), false);

        out = samopen(fileName2.constData(), readMode.constData(), aux);
        SAMTOOL_CHECK(NULL != out, openFileError(fileName2), false);
        SAMTOOL_CHECK(NULL != out->header, headerError(fileName2), false);
    }

    if(in->header->target_len && out->header->target_len){
        //if there are headers
        if(*(in->header->target_len) != *(out->header->target_len)){
            os.setError(QString("Different target length of files. %1 and %2").arg(qint64(in->header->target_len)).arg(qint64(out->header->target_len)));
            closeFiles(in, out);
            return false;
        }
    }

    bam1_t *b1 = bam_init1();
    bam1_t *b2 = bam_init1();
    int r1;
    int r2;
    {
        while ((r1 = samread(in, b1)) >= 0) { // read one alignment from file1
            if((r2 = samread(out, b2) >= 0)){ //read one alignment from file2

            }else{
                samreadCheck(r2, os, fileName2);
                os.setError("Different number of reads in files");
                break;
            }
        }

        samreadCheck(r1, os, fileName1);
        if((r2 = samread(out, b2)) >= 0){
            os.setError("Different number of reads in files");
        }
        bam_destroy1(b1);
        bam_destroy1(b2);
    }

    closeFiles(in, out);

    if(os.hasError()){
        return false;
    }

    return true;
}

/////////////////////////////////////////////////
//FASTQIterator

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4018 )
#endif

KSEQ_INIT(gzFile, gzread)

#ifdef _MSC_VER
#pragma warning( pop )
#endif

FASTQIterator::FASTQIterator(const QString &fileUrl)
    : seq(NULL)
{
    fp = gzopen(fileUrl.toLatin1().constData(), "r");
    seq = kseq_init(static_cast<gzFile>(fp));
    fetchNext();
}

FASTQIterator::~FASTQIterator(){
    kseq_destroy(static_cast<kseq_t *>(seq));
    gzclose(static_cast<gzFile>(fp));
}

DNASequence FASTQIterator::next(){
    if (hasNext()) {
        kseq_t *realSeq = static_cast<kseq_t *>(seq);

        QString name = QString::fromLatin1(realSeq->name.s);
        QString comment = QString::fromLatin1(realSeq->comment.s);
        QString rseq = QString::fromLatin1(realSeq->seq.s);
        DNAQuality quality = (realSeq->qual.l) ? QString::fromLatin1(realSeq->qual.s).toLatin1()
            : QByteArray("");
        DNASequence res(name, rseq.toLatin1());
        res.quality = quality;
        res.info.insert(DNAInfo::FASTQ_COMMENT, comment);

        fetchNext();

        return res;
    }
    return DNASequence();
}

bool FASTQIterator::hasNext(){
    return seq != NULL;
}

void FASTQIterator::fetchNext(){
    if (kseq_read(static_cast<kseq_t *>(seq)) < 0 ) {
        seq = NULL;
    }
}

} // U2
