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

#ifndef _U2_COUNTER_H_
#define _U2_COUNTER_H_

#include <U2Core/global.h>

#include <QtCore/QList>

namespace U2 {

class U2CORE_EXPORT GCounter : public QObject {
    Q_OBJECT
public:
    GCounter(const QString& name, const QString& suffix, double scale = 1);
    virtual ~GCounter();

    static const QList<GCounter*>& allCounters() {return getCounters();}

    QString name;
    QString suffix;
    qint64  totalCount;
    double  counterScale;
    bool    dynamicCounter; //true if created and deleted dynamically at application runtime (not static init time)

    double scaledTotal() const {return totalCount / counterScale;}
    
protected:
    
    static QList<GCounter*>& getCounters();
};

//Marks that counter will be reported by Shtirlitz
//TODO: implement GPerformanceCounter for plugin_perf_monitor?
class U2CORE_EXPORT GReportableCounter : public GCounter {
    Q_OBJECT
public:
    GReportableCounter(const QString& name, const QString& suffix, double scale = 1);
};


class U2CORE_EXPORT SimpleEventCounter {
public:
    SimpleEventCounter(GCounter* tc) : totalCounter(tc), eventCount(1){ assert(tc!=NULL);}
    virtual ~SimpleEventCounter() {totalCounter->totalCount+=eventCount;}
private:
    GCounter*   totalCounter;
    qint64      eventCount;
};

#define GCOUNTER(cvar, tvar, name) \
    static GReportableCounter cvar(name, "", 1); \
    SimpleEventCounter tvar(&cvar)


} //namespace

#endif
