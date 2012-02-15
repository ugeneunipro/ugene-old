/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _PRIMER3_TASK_H_
#define _PRIMER3_TASK_H_

#include <memory>

#include <U2Core/Task.h>
#include <U2Core/AnnotationTableObject.h>

#include "Primer3TaskSettings.h"

#include "primer3.h"

namespace U2 {

class Primer
{
public:
    Primer();
    Primer(const primer_rec &primerRec);

    int getStart()const;
    int getLength()const;
    double getMeltingTemperature()const;
    double getGcContent()const;
    short getSelfAny()const;
    short getSelfEnd()const;
    double getEndStabilyty()const;

    void setStart(int start);
    void setLength(int length);
    void setMeltingTemperature(double meltingTemperature);
    void setGcContent(double gcContent);
    void setSelfAny(short selfAny);
    void setSelfEnd(short selfEnd);
    void setEndStability(double endStability);
private:
    int start;
    int length;
    double meltingTemperature;
    double gcContent;
    short selfAny;
    short selfEnd;
    double endStability;
};

class PrimerPair
{
public:
    PrimerPair();
    PrimerPair(const primer_pair &primerPair, int offset = 0);
    PrimerPair(const PrimerPair &primerPair);
    const PrimerPair &operator=(const PrimerPair &primerPair);

    Primer *getLeftPrimer()const;
    Primer *getRightPrimer()const;
    Primer *getInternalOligo()const;
    short getComplAny()const;
    short getComplEnd()const;
    int getProductSize()const;

    void setLeftPrimer(Primer *leftPrimer);
    void setRightPrimer(Primer *rightPrimer);
    void setInternalOligo(Primer *internalOligo);
    void setComplAny(short complAny);
    void setComplEnd(short complEnd);
    void setProductSize(int productSize);

    bool operator<(const PrimerPair &pair)const;
private:
    // don't forget to change copy constructor and assignment operator when changing this!
    std::auto_ptr<Primer> leftPrimer;
    std::auto_ptr<Primer> rightPrimer;
    std::auto_ptr<Primer> internalOligo;
    short complAny;
    short complEnd;
    int productSize;
    double quality;
    double complMeasure;
};

class Primer3Task : public Task
{
    Q_OBJECT
public:
    Primer3Task(const Primer3TaskSettings &settings);

    void run();
    Task::ReportResult report();
    void sumStat(Primer3TaskSettings *st);

    QList<PrimerPair> getBestPairs()const;
private:
    Primer3TaskSettings settings;
    QList<PrimerPair> bestPairs;

    int offset;
};

class Primer3ToAnnotationsTask;

class Primer3SWTask : public Task
{
    Q_OBJECT
public:
    Primer3SWTask(const Primer3TaskSettings &settings);

    void prepare();
    Task::ReportResult report();

    QList<PrimerPair> getBestPairs()const;
private:
    static const int CHUNK_SIZE = 1024*256;

    QList<Primer3Task *> regionTasks;
    Primer3TaskSettings settings;
    QList<PrimerPair> bestPairs;

    friend class Primer3ToAnnotationsTask;
};

class Primer3ToAnnotationsTask : public Task {
    Q_OBJECT
public:
    Primer3ToAnnotationsTask( const Primer3TaskSettings &settings,
        AnnotationTableObject* aobj_, const QString & groupName_, const QString & annName_ );

    void prepare();
    void run(){}
    virtual QString generateReport() const;
    Task::ReportResult report();

private:
    SharedAnnotationData oligoToAnnotation(QString title, const Primer &primer, U2Strand strand);

    Primer3TaskSettings settings;

    AnnotationTableObject * aobj;
    QString groupName;
    QString annName;

    Primer3SWTask *searchTask;
};

} //namespace

#endif //_PRIMER3_TASK_H_
