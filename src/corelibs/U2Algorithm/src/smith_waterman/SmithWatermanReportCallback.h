/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SMITH_WATERMAN_REPORT_CALLBACK_H_
#define _U2_SMITH_WATERMAN_REPORT_CALLBACK_H_

#include <U2Algorithm/SmithWatermanResult.h>
#include <U2Core/DNAAlphabet.h>

#include <QObject>
#include <U2Core/AnnotationTableObject.h>


namespace U2 {

class U2ALGORITHM_EXPORT SmithWatermanReportCallback {
public:
    virtual QString report(const QList<SmithWatermanResult>& ) = 0;
    virtual ~SmithWatermanReportCallback() {}
};

class U2ALGORITHM_EXPORT SmithWatermanReportCallbackAnnotImpl:  
                                        public QObject,
                                        public SmithWatermanReportCallback {
    Q_OBJECT
public:
    SmithWatermanReportCallbackAnnotImpl(AnnotationTableObject* _aobj,
                                    const QString& _annotationName,
                                    const QString& _annotationGroup, 
                                    QObject* pOwn = 0);

    virtual QString report(const QList<SmithWatermanResult>& result);    
    const QList<SharedAnnotationData>& getAnotations() const {return anns;}
private:    

    QString annotationName;
    QString annotationGroup;
    QPointer<AnnotationTableObject> aObj;
    QList<SharedAnnotationData> anns;
    bool autoReport;
};

class Project;

class U2ALGORITHM_EXPORT SmithWatermanReportCallbackMAImpl : public QObject, public SmithWatermanReportCallback {
    Q_OBJECT
public:
    struct TagExpansionPossibleData {
        inline TagExpansionPossibleData(const QString & _refSequenceName, const QString & _patternName)
        : refSequenceName(_refSequenceName), patternName(_patternName), curProcessingSubseq(NULL) {}

        const QString refSequenceName;
        const QString patternName;
        U2Region * curProcessingSubseq;
    };

    SmithWatermanReportCallbackMAImpl(const QString & _resultDirPath, const QString & _mobjectNamesTemplate,
                                        const QString & _refSubseqTemplate, const QString & _ptrnSubseqTemplate,
                                        const QByteArray & _refSequence, const QByteArray & _pattern,
                                        const QString & _refSeqName, const QString & _patternName,
                                        DNAAlphabet * _alphabet);
    virtual QString report(const QList<SmithWatermanResult> & _results);
    static void alignSequences(QByteArray & refSequence, QByteArray & ptrnSequence, const QByteArray & pairwiseAlignment);
    static void changeGivenUrlIfDocumentExists(QString & givenUrl, const Project * curProject);

private:
    QString resultDirPath;
    QString mobjectNamesTemplate;
    QString refSubseqTemplate;
    QString ptrnSubseqTemplate;
    QByteArray refSequence;
    QByteArray pattern;
    DNAAlphabet * alphabet;
    TagExpansionPossibleData expansionInfo;

    static const quint8 countOfSimultLoadedMADocs;
};

} // namespace

#endif
