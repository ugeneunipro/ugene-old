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

#ifndef _U2_DOT_PLOT_TASKS_H_
#define _U2_DOT_PLOT_TASKS_H_

#include "DotPlotClasses.h"
#include <U2Core/Task.h>

#include <QtCore/QTextStream>

namespace U2 {

class Document;
class DNASequenceObject;

// save dotplot to the file
class SaveDotPlotTask : public Task {
    Q_OBJECT
public:
    SaveDotPlotTask(const QString &file, QList<DotPlotResults> *dotPlotDirectList, QList<DotPlotResults> *dotPlotInverseList, DNASequenceObject *seqX, DNASequenceObject *seqY, int mLen, int ident)
        : Task(tr("DotPlot saving"), TaskFlags_FOSCOE), filename(file), directList(dotPlotDirectList), inverseList(dotPlotInverseList), sequenceX(seqX), sequenceY(seqY), minLen(mLen), identity(ident)
    {
        tpm = Task::Progress_Manual;
    };

    void run();

    static DotPlotDialogs::Errors checkFile(const QString &filename);

private:
    QString filename;
    QList<DotPlotResults> *directList, *inverseList;
    DNASequenceObject *sequenceX, *sequenceY;
    int minLen, identity;

    void saveDotPlot(QTextStream &stream);
};

// load dotplot from file
class LoadDotPlotTask : public Task {
    Q_OBJECT
public:

    LoadDotPlotTask(const QString &file, QList<DotPlotResults> *dotPlotDirectList, QList<DotPlotResults> *dotPlotInverseList, DNASequenceObject *seqX, DNASequenceObject *seqY, int *mLen, int *ident, bool *dir, bool *inv)
        : Task(tr("DotPlot loading"), TaskFlags_FOSCOE), filename(file), directList(dotPlotDirectList), inverseList(dotPlotInverseList), sequenceX(seqX), sequenceY(seqY), minLen(mLen), identity(ident), direct(dir), inverted(inv)
    {
        tpm = Task::Progress_Manual;
    };

    void run();

    static DotPlotDialogs::Errors checkFile(const QString &filename, const QString &seqXName, const QString &seqYName);

private:
    QString filename;
    QList<DotPlotResults> *directList, *inverseList;
    DNASequenceObject *sequenceX, *sequenceY;
    int *minLen, *identity;
    bool *direct, *inverted;

    bool loadDotPlot(QTextStream &stream, int fileSize);
};

// dotplot wizard: load needed files, open sequence view and build dotplot
class DotPlotLoadDocumentsTask : public Task {
    Q_OBJECT
public:

    DotPlotLoadDocumentsTask(QString firstF, int firstG, QString secondF, int secondG);
    ~DotPlotLoadDocumentsTask();

    void run(){};
    void prepare();

    QList<Document*> getDocuments() const {return docs;}

private:
    QString firstFile, secondFile;
    int firstGap, secondGap;
    QList<Document*> docs;

    Document* loadFile(QString inFile, int gapSize);

signals:
    void si_stateChanged(Task* task);
};

} // namespace

#endif // _U2_DOT_PLOT_TASKS_H_
