/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "DNASequenceGenerator.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/ProjectModel.h>

#include <U2Gui/OpenViewTask.h>

#include <U2Gui/DialogUtils.h>


namespace U2 {

int getRandomInt(int max) {
    return qrand() % (max + 1);
}

const QString DNASequenceGenerator::ID("dna_generator");

QString DNASequenceGenerator::prepareReferenceFileFilter() {
    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true) +
        ";;" + DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, false);
    return filter;
}

void DNASequenceGenerator::generateSequence(const QMap<char, qreal>& charFreqs, int length, QByteArray& result) {
    QMap<char, int> content;
    QMapIterator<char, qreal> iter(charFreqs);
    assert(iter.hasNext());
    iter.next();
    char ch = iter.key();
    int total = 0;
    while (iter.hasNext()) {
        iter.next();
        char ch = iter.key();
        qreal percent = iter.value();
        assert(percent>=0 && percent<=1);
        int num = length * percent;
        total += num;
        content.insertMulti(ch, num);
    }
    assert(total>=0 && total<=length);
    content[ch] = length - total;

    result.resize(length);
    for (int idx = 0; idx < length; idx++) {
        int rnd = getRandomInt(length - idx - 1);
        assert(rnd < length - idx);
        int charRange = 0;
        foreach(char ch, content.keys()) {
            charRange += content.value(ch);
            if (rnd < charRange) {
                --content[ch];
                result[idx] = ch;
                break;
            }
        }
    }
}

void evaluate(const QByteArray& seq, QMap<char, qreal>& result) {
    QMap<char, int> occurrencesMap;
    foreach(char ch, seq) {
        if (!occurrencesMap.keys().contains(ch)) {
            occurrencesMap.insertMulti(ch, 1);
        } else {
            ++occurrencesMap[ch];
        }
    }

    qreal len = seq.length();
    QMapIterator<char, int> i(occurrencesMap);
    while (i.hasNext()) {
        i.next();
        int count = i.value();
        qreal freq = count / len;
        result.insertMulti(i.key(), freq);
    }
}

void DNASequenceGenerator::evaluateBaseContent(const DNASequence& sequence, QMap<char, qreal>& result) {
    evaluate(sequence.seq, result);
}

void DNASequenceGenerator::evaluateBaseContent(const MAlignment& ma, QMap<char, qreal>& result) {
    QList< QMap<char, qreal> > rowsContents;
    foreach(const MAlignmentRow& row, ma.getRows()) {
        QMap<char, qreal> rowContent;
        evaluate(row.getCore(), rowContent);
        rowsContents.append(rowContent);
    }
    
    QListIterator< QMap<char, qreal> > listIter(rowsContents);
    while (listIter.hasNext()) {
        const QMap<char, qreal>& cm = listIter.next();
        QMapIterator<char, qreal> mapIter(cm);
        while (mapIter.hasNext()) {
            mapIter.next();
            char ch = mapIter.key();
            qreal freq = mapIter.value();
            if (!result.keys().contains(ch)) {
                result.insertMulti(ch, freq);
            } else {
                result[ch] += freq;
            }
        }
    }

    int rowsNum = ma.getNumRows();
    QMutableMapIterator<char, qreal> i(result);
    while (i.hasNext()) {
        i.next();
        i.value() /= rowsNum;
    }
}

// DNASequenceGeneratorTask
//////////////////////////////////////////////////////////////////////////

EvaluateBaseContentTask* DNASequenceGeneratorTask::createEvaluationTask(Document* doc, QString& err) {
    assert(doc->isLoaded());
    QList<GObject*> gobjects = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    gobjects << doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (!gobjects.isEmpty()) {
        return new EvaluateBaseContentTask(gobjects.first());
    }
    err = tr("Reference for sequence generator has to be a sequence or an alignment");
    return NULL;
}

DNASequenceGeneratorTask::DNASequenceGeneratorTask(const DNASequenceGeneratorConfig& cfg_)
: Task(tr("Generate sequence task"), TaskFlag_NoRun), cfg(cfg_), loadRefTask(NULL),
evalTask(NULL), generateTask(NULL), saveTask(NULL) {
    if (cfg.useReference()) {
        // do not load reference file if it is already in project and has loaded state
        const QString& docUrl = cfg.getReferenceUrl();
        Project* prj = AppContext::getProject();
        if (prj) {
            Document* doc = prj->findDocumentByURL(docUrl);
            if (doc && doc->isLoaded()) {
                QString err;
                evalTask = createEvaluationTask(doc, err);
                if (evalTask) {
                    addSubTask(evalTask);
                } else {
                    stateInfo.setError(err);
                }
                return;
            }
        }

        loadRefTask = LoadDocumentTask::getDefaultLoadDocTask(GUrl(docUrl));
        if (loadRefTask) {
            addSubTask(loadRefTask);
        } else {
            stateInfo.setError(tr("Incorrect reference file"));
            return;
        }
    } else {
        generateTask = new GenerateDNASequenceTask(cfg.getContent(), cfg.getLength(), cfg.window, cfg.getNumberOfSequences(), cfg.seed);
        addSubTask(generateTask);
    }
}

QList<Task*> DNASequenceGeneratorTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> tasks;
    if (hasError() || isCanceled()) {
        return tasks;
    }

    if (subTask == loadRefTask) {
        QString err;
        Document* doc = loadRefTask->getDocument();
        evalTask = createEvaluationTask(doc, err);
        if (evalTask) {
            tasks.append(evalTask);
        } else {
            stateInfo.setError(err);
        }
    } else if (subTask == evalTask) {
        cfg.alphabet = evalTask->getAlphabet();
        QMap<char, qreal> content = evalTask->getResult();
        generateTask = new GenerateDNASequenceTask(content, cfg.getLength(), cfg.window, cfg.getNumberOfSequences(), cfg.seed);
        tasks.append(generateTask);
    } else if (subTask == generateTask) {
        QList< QByteArray > seqs = generateTask->getResult();
        const QString& name = cfg.getSequenceName();
        DNAAlphabet* alp = cfg.getAlphabet();
        assert(alp);
        IOAdapterFactory * io = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(cfg.getOutUrlString()));

        if (seqs.size() == 1) {
            const QByteArray& seq = seqs.first();
            DNASequence dna(name, seq, alp);
            results.append(dna);
        } else {
            for (int i = 0, n = seqs.size(); i<n; i++ ) {
                const QByteArray& seq = seqs.at(i);
                QString temp = QString("%1 %2").arg(name).arg(i+1);
                DNASequence dna(temp, seq, alp);
                results.append(dna);
            }
        }

        if (cfg.saveDoc) {
            QList<GObject*> sequences;
            foreach(DNASequence s, results) {
                DNASequenceObject* dnaObj = new DNASequenceObject(s.getName(), s);
                sequences.append(dnaObj);
            }
            Document* doc = new Document(cfg.getDocumentFormat(), io, GUrl(cfg.getOutUrlString()), sequences);
            saveTask = new SaveDocumentTask(doc, SaveDoc_Overwrite);
            tasks.append(saveTask);
        }
    } else if (saveTask == subTask) {
        Document* doc = saveTask->getDocument();
        if (!cfg.addToProj) {
            doc->unload();
            delete doc;
        } else {
            Project* prj = AppContext::getProject();
            if (prj) {
                Document* d = prj->findDocumentByURL(doc->getURL());
                if (d==NULL) {
                    prj->addDocument(doc);
                    tasks << new OpenViewTask(doc);
                } else {
                    // if document with same url is already exists in project
                    // it will be reloaded by DocumentUpdater => delete this instance
                    doc->unload();
                    delete doc;
                }
            } else {
                tasks << AppContext::getProjectLoader()->openWithProjectTask(QList<GUrl>() << doc->getURL());
                // open project task will load supplied url
                doc->unload();
                delete doc;
            }
        }
    }
    return tasks;
}

// EvaluateBaseContentTask
EvaluateBaseContentTask::EvaluateBaseContentTask(GObject* obj)
: Task(tr("Evaluate base content task"), TaskFlag_None), _obj(obj), alp(NULL) {
}

void EvaluateBaseContentTask::run() {
    if (_obj->getGObjectType() == GObjectTypes::SEQUENCE) {
        DNASequenceObject* dnaObj = qobject_cast<DNASequenceObject*>(_obj);
        alp = dnaObj->getAlphabet();
        DNASequenceGenerator::evaluateBaseContent(dnaObj->getDNASequence(), result);
    } else if (_obj->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT) {
        MAlignmentObject* maObj = qobject_cast<MAlignmentObject*>(_obj);
        alp = maObj->getAlphabet();
        DNASequenceGenerator::evaluateBaseContent(maObj->getMAlignment(), result);
    } else {
        stateInfo.setError(tr("Base content can be evaluated for sequence or sequence alignment"));
    }
}

// GenerateTask
GenerateDNASequenceTask::GenerateDNASequenceTask(const QMap<char, qreal>& baseContent_, int length_, int window_, int count_, int seed_)
: Task(tr("Generate DNA sequence task"), TaskFlag_None), baseContent(baseContent_), length(length_), window(window_), count(count_), seed(seed_) {
}

void GenerateDNASequenceTask::run() {
    if(seed < 0) {
        qsrand(QDateTime::currentDateTime().toTime_t());
    } else {
        qsrand(seed);
    }
    for (int i=0; i<count; i++) {
        QByteArray seq;
        QByteArray tmp;
        if(window > length) {
            window = length;
        }
        for(int i = 0; i< length/window; i++) {
            DNASequenceGenerator::generateSequence(baseContent, window, tmp);
            seq.append(tmp);
        }
        DNASequenceGenerator::generateSequence(baseContent, length % window, tmp);
        seq.append(tmp);

        result.append(seq);
    }
}

} //namespace
