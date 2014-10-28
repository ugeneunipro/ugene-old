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

#ifndef _U2_BED_TOOLS_WORKERS_LIBRARY_
#define _U2_BED_TOOLS_WORKERS_LIBRARY_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Core/GUrl.h>

#include "BaseBedToolsWorker.h"

namespace U2 {
namespace LocalWorkflow {

//////////////////////////////////////////////////
//Slopbed
class SlopbedPrompter;
typedef PrompterBase<SlopbedPrompter> SlopbedBase;
class SlopbedPrompter : public SlopbedBase {
    Q_OBJECT
public:
    SlopbedPrompter(Actor* p = 0) : SlopbedBase(p) {}
protected:
    QString composeRichDoc();
}; //SlopbedPrompter

class SlopbedWorker: public BaseBedToolsWorker {
    Q_OBJECT
public:
    SlopbedWorker(Actor *a);
protected:
    QVariantMap getCustomParameters() const;
    QString getDefaultFileName() const;
    Task *getTask(const BedToolsSetting &settings) const;
}; //SlopbedWorker

class SlopbedWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    SlopbedWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new SlopbedWorker(a); }
}; //SlopbedWorkerFactory

class SlopbedTask : public BaseBedToolsTask{
    Q_OBJECT
public:
    SlopbedTask (const BedToolsSetting &settings);

protected:
    QStringList getParameters(U2OpStatus& os);
};

//////////////////////////////////////////////////
//genomecov
class GenomecovPrompter;
typedef PrompterBase<GenomecovPrompter> GenomecovBase;
class GenomecovPrompter : public GenomecovBase {
    Q_OBJECT
public:
    GenomecovPrompter(Actor* p = 0) : GenomecovBase(p) {}
protected:
    QString composeRichDoc();
}; //GenomecovPrompter

class GenomecovWorker: public BaseBedToolsWorker {
    Q_OBJECT
public:
    GenomecovWorker(Actor *a);
protected:
    QVariantMap getCustomParameters() const;
    QString getDefaultFileName() const;
    Task *getTask(const BedToolsSetting &settings) const;
}; //GenomecovWorker

class GenomecovWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    GenomecovWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new GenomecovWorker(a); }
}; //GenomecovWorkerFactory

class GenomecovTask : public BaseBedToolsTask{
    Q_OBJECT
public:
    GenomecovTask (const BedToolsSetting &settings);

protected:
    QStringList getParameters(U2OpStatus& os);
};

} //LocalWorkflow
} //U2

#endif //_U2_BED_TOOLS_WORKERS_LIBRARY_
