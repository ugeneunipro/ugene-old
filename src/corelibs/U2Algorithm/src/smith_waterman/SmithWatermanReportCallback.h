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

class U2ALGORITHM_EXPORT SmithWatermanReportCallbackAnnotImpl : public QObject,
    public SmithWatermanReportCallback
{
    Q_OBJECT
public:
    SmithWatermanReportCallbackAnnotImpl(AnnotationTableObject* _aobj,
        const QString& _annotationName, const QString& _annotationGroup,
        bool _addPatternSubseqToQual, QObject* pOwn = 0);

    virtual QString report(const QList<SmithWatermanResult>& result);    
    const QList<SharedAnnotationData>& getAnotations() const {return anns;}
private:    

    QString annotationName;
    QString annotationGroup;
    QPointer<AnnotationTableObject> aObj;
    QList<SharedAnnotationData> anns;
    bool autoReport;
    bool addPatternSubseqToQual;
};

class Project;

class U2ALGORITHM_EXPORT SmithWatermanReportCallbackMAImpl : public QObject, public SmithWatermanReportCallback {
    Q_OBJECT
public:
    struct TagExpansionPossibleData {
        TagExpansionPossibleData() {}
        inline TagExpansionPossibleData(const QString & _refSequenceName, const QString & _patternName)
        : refSequenceName(_refSequenceName), patternName(_patternName), curProcessingSubseq(NULL) {}

        const QString refSequenceName;
        const QString patternName;
        U2Region * curProcessingSubseq;
    };

    enum WhatDoYouWantFromMe {
        NotDefined = 0,
        SequenceView_Search = 1,
        MSA_Alignment_InNewWindow = 2,
        MSA_Alignment_InCurrentWindow = 3
    };

public:
    //Smith-Waterman search in sequence viewer
    SmithWatermanReportCallbackMAImpl(const QString & _resultDirPath, const QString & _mobjectNamesTemplate,
                                        const QString & _refSubseqTemplate, const QString & _ptrnSubseqTemplate,
                                        const QByteArray & _refSequence, const QByteArray & _pattern,
                                        const QString & _refSeqName, const QString & _patternName,
                                      DNAAlphabet * _alphabet, WhatDoYouWantFromMe _plan = SequenceView_Search);

    //Smith-Waterman alignment in MSA Editor (in new window)
    SmithWatermanReportCallbackMAImpl(const QString & _resultDirPath, const QString & _mobjectName,
                                      const U2EntityRef& _firstSequenceRef, const U2EntityRef& _secondSequenceRef,
                                      const U2EntityRef& _sourceMsaRef, WhatDoYouWantFromMe _plan = MSA_Alignment_InNewWindow);

    //Smith-Waterman alignment in MSA Editor (in current window)
    SmithWatermanReportCallbackMAImpl(const U2EntityRef& _firstSequenceRef, const U2EntityRef& _secondSequenceRef,
                                      const U2EntityRef& _sourceMsaRef, WhatDoYouWantFromMe _plan = MSA_Alignment_InCurrentWindow);

    virtual ~SmithWatermanReportCallbackMAImpl();

    virtual QString report(const QList<SmithWatermanResult> & _results);
    static void alignSequences(QByteArray & refSequenceData, QByteArray & ptrnSequence, const QByteArray & pairwiseAlignment);
    static void alignSequences(QList<U2MsaGap>& refSequenceGapModel, QList<U2MsaGap>& ptrnSequenceGapModel, const QByteArray& pairwiseAlignment);
    static void changeGivenUrlIfDocumentExists(QString & givenUrl, const Project * curProject);

private:
    QString planFor_SequenceView_Search(const QList<SmithWatermanResult> & _results);
    QString planFor_MSA_Alignment_InNewWindow(const QList<SmithWatermanResult> & _results);
    QString planFor_MSA_Alignment_InCurrentWindow(const QList<SmithWatermanResult> & _results);

private:
    WhatDoYouWantFromMe plan;       //determine actions to be perfomed

    QString resultDirPath;
    QString mobjectNamesTemplate;
    QString refSubseqTemplate;
    QString ptrnSubseqTemplate;
    QByteArray refSequenceData;
    QByteArray ptrnSequenceData;
    DNAAlphabet * alphabet;
    TagExpansionPossibleData expansionInfo;

    U2EntityRef firstSequenceRef;
    U2EntityRef secondSequenceRef;
    U2EntityRef sourceMsaRef;
    QString mobjectName;
    DbiConnection sourceMsaConnection;

    static const quint8 countOfSimultLoadedMADocs;
};

} // namespace

#endif
