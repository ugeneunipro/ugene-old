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

#include "RemoteBLASTTask.h"
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/Counter.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/BaseDocumentFormats.h>

namespace U2 {

class BaseIOAdapters;
class BaseDocumentFormats;


RemoteBLASTToAnnotationsTask::RemoteBLASTToAnnotationsTask( const RemoteBLASTTaskSettings & _cfg, int _qoffs, 
                                AnnotationTableObject* _ao, const QString &_url,const QString & _group):
Task( tr("RemoteBLASTTask"), TaskFlags_NR_FOSCOE ), offsInGlobalSeq(_qoffs), aobj(_ao), group(_group), url(_url) {
    GCOUNTER( cvar, tvar, "RemoteBLASTToAnnotationsTask" );
    
    queryTask = new RemoteBLASTTask(_cfg);
    addSubTask(queryTask);
}


QList<Task*> RemoteBLASTToAnnotationsTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    
    if(subTask->hasError() && subTask == queryTask) {
        stateInfo.setError(subTask->getError());
        return res;
    }

    if (hasError() || isCanceled()) {
        return res;
    }

    if (aobj.isNull()) {
        stateInfo.setError(  tr("The object was removed\n") );
        return res;
    }
    if (subTask == queryTask) {
        //shift annotations according to offset first
        
        RemoteBLASTTask * rrTask = qobject_cast<RemoteBLASTTask *>(queryTask);
        assert( rrTask );
        
        QList<SharedAnnotationData> anns = rrTask->getResultedAnnotations();

        if(!anns.isEmpty()) {		
           // Document* d = AppContext::getProject()->findDocumentByURL(url);
            //assert(d==NULL);
            if(!url.isEmpty()) {
                Document *d = AppContext::getProject()->findDocumentByURL(url);
                if(d==NULL) {
                    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
                    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
                    d = df->createNewLoadedDocument(iof, url, stateInfo);
                    CHECK_OP(stateInfo, res);
                    d->addObject(aobj);
                    AppContext::getProject()->addDocument(d);
                } else {
                    setError(tr("File %1 already exists").arg(url));
                    return res;
                }
            }
            
            for(QMutableListIterator<SharedAnnotationData> it_ad(anns); it_ad.hasNext(); ) {
                AnnotationData * ad = it_ad.next().data();
                U2Region::shift(offsInGlobalSeq, ad->location->regions);
            }

            res.append(new CreateAnnotationsTask(aobj, group, anns));
        }
    }
    return res;
}


RemoteBLASTTask::RemoteBLASTTask( const RemoteBLASTTaskSettings & cfg_) :
Task( tr("RemoteBLASTTask"), TaskFlag_None ), timeout(true), cfg(cfg_)
{
}

void RemoteBLASTTask::prepareQueries() {
    Query q;
    if(cfg.aminoT) {
        q.amino = true;
        QByteArray complQuery( cfg.query.size(), 0 );
        cfg.complT->translate( cfg.query.data(), cfg.query.size(), complQuery.data(), complQuery.size() );
        TextUtils::reverse( complQuery.data(), complQuery.size() );
        for( int i = 0; i < 3; ++i ) {
            QByteArray aminoQuery( cfg.query.size() / 3, 0 );
            cfg.aminoT->translate( cfg.query.data()+i, cfg.query.size()-i, aminoQuery.data(), aminoQuery.size() );
            q.seq = aminoQuery;
            q.offs = i;
            q.complement = false;
            queries.push_back(q);
            QByteArray aminoQueryCompl( cfg.query.size() / 3, 0 );
            cfg.aminoT->translate(complQuery.data()+i, complQuery.size()-i, aminoQueryCompl.data(), aminoQueryCompl.size());
            q.seq = aminoQueryCompl;
            q.offs = i;
            q.complement = true;
            queries.push_back(q);
        }
    }
    else {
        q.seq = cfg.query;
        queries.push_back(q);
    }
}

void RemoteBLASTTask::prepare() {
    prepareQueries();
    algoLog.trace("Sequences prepared");
    for (QList<Query>::iterator it = queries.begin(),end = queries.end();it!=end;it++) {
        DataBaseFactory *dbf = AppContext::getDataBaseRegistry()->getFactoryById(cfg.dbChoosen);
        if(dbf == NULL) {
            stateInfo.setError(tr("Incorrect database"));
            return;
        }
        HttpRequest *tmp = dbf->getRequest(this);
        httpRequest.append(tmp);
    }
    algoLog.trace("Requests formed");
    connect(&timer,SIGNAL(timeout()),SLOT(sl_timeout()));
    timeout = true;
    timer.setSingleShot(true);
    int mult = cfg.aminoT ? 6 : 1; //if 6 requests - 6 times more wait
    timer.start(cfg.retries*1000*60*mult);
}


void RemoteBLASTTask::run() {	
    for( int i = 0;i < queries.count();i++ ) {
        bool error = true;
        if( isCanceled() ) {
            return;
        }
        
        httpRequest[i]->sendRequest(cfg.params,QString(queries[i].seq.data()));
        error = httpRequest[i]->connectionError;
        if(error) {
            stateInfo.setError(httpRequest[i]->getError());
            return;
        }
    
        createAnnotations(queries[i],httpRequest[i]);
    }
}

QList<SharedAnnotationData> RemoteBLASTTask::getResultedAnnotations() const {
    return resultAnnotations;
}

void RemoteBLASTTask::createAnnotations(const Query &q, HttpRequest *t) {
    QList<SharedAnnotationData> annotations = t->getAnnotations();
    if(annotations.isEmpty()) return;

    if(cfg.filterResult) {
        annotations = filterAnnotations(annotations);
    }

    for (int i = 0; i < annotations.size(); i++) {
        SharedAnnotationData & d = annotations[i];
        for( QVector<U2Region>::iterator jt = d->location->regions.begin(), eend = d->location->regions.end(); eend != jt; ++jt ) {
            qint64& s = jt->startPos;
            qint64& l = jt->length;

            if( q.complement ) {
                s = q.seq.size() - s - l;
                d->setStrand(d->getStrand().isCompementary() ? U2Strand::Direct : U2Strand::Complementary);
            }
            if( q.amino ) {
                s = s * 3 + (q.complement ? 2 - q.offs : q.offs);
                l = l * 3;
            }
        }
    }

    resultAnnotations << annotations;
}

QList<SharedAnnotationData> RemoteBLASTTask::filterAnnotations(QList<SharedAnnotationData> annotations) {
    QString selectiveQual = cfg.useEval ? "e-value" : "score";
    QList<SharedAnnotationData> resultList;

    if(cfg.filterResult & FilterResultByAccession) {
        QStringList accessions;
        foreach(const SharedAnnotationData &ann, annotations) {
            QString acc = ann->findFirstQualifierValue("accession");
            if(accessions.contains(acc)) {
                QString eval = ann->findFirstQualifierValue(selectiveQual);
                foreach(const SharedAnnotationData &a, resultList) {
                    if(a->findFirstQualifierValue("accession") == acc) {
                        if(cfg.useEval ? a->findFirstQualifierValue(selectiveQual).toDouble() < eval.toDouble() :
                        a->findFirstQualifierValue(selectiveQual).toDouble() > eval.toDouble()) {
                            resultList.removeOne(a);
                            resultList << ann;
                        }
                        break;
                    }
                } 
            } else {
                resultList << ann;
                accessions << acc;
            }
        }
        annotations = resultList;
    }
    
    if(cfg.filterResult & FilterResultByDef) {
        resultList.clear();
        QStringList defs;
        foreach(const SharedAnnotationData &ann, annotations) {
            QString def = ann->findFirstQualifierValue("def");
            if(defs.contains(def)) {
                QString eval = ann->findFirstQualifierValue(selectiveQual);
                foreach(const SharedAnnotationData &a, resultList) {
                    if(a->findFirstQualifierValue("def") == def) {
                        if(cfg.useEval ? a->findFirstQualifierValue(selectiveQual).toDouble() < eval.toDouble() :
                        a->findFirstQualifierValue(selectiveQual).toDouble() > eval.toDouble()) {
                            resultList.removeOne(a);
                            resultList << ann;
                        }
                        break;
                    }
                } 
            } else {
                resultList << ann;
                defs << def;
            }
        }
        annotations = resultList;
    }
    
    if(cfg.filterResult & FilterResultById) {
        resultList.clear();
        QStringList ids;
        foreach(const SharedAnnotationData &ann, annotations) {
            QString id = ann->findFirstQualifierValue("id");
            if(ids.contains(id)) {
                QString eval = ann->findFirstQualifierValue(selectiveQual);
                foreach(const SharedAnnotationData &a, resultList) {
                    if(a->findFirstQualifierValue("id") == id) {
                        if(cfg.useEval ? a->findFirstQualifierValue(selectiveQual).toDouble() < eval.toDouble() :
                        a->findFirstQualifierValue(selectiveQual).toDouble() > eval.toDouble()) {
                            resultList.removeOne(a);
                            resultList << ann;
                        }
                        break;
                    }
                } 
            } else {
                resultList << ann;
                ids << id;
            }
        }
    }

    
    return resultList;
}

RemoteCDSearch::RemoteCDSearch(const CDSearchSettings& settings) {
    RemoteBLASTTaskSettings cfg;
    cfg.dbChoosen = "cdd";

    addParametr(cfg.params, ReqParams::cdd_hits, 500);
    addParametr(cfg.params, ReqParams::cdd_eValue, settings.ev);

    if (settings.dbName == CDDNames::CDD_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "CDD");
    } else if (settings.dbName == CDDNames::PFAM_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "oasis_pfam");
    } else if (settings.dbName == CDDNames::SMART_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "oasis_smart");
    } else if (settings.dbName == CDDNames::COG_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "oasis_cog");
    } else if (settings.dbName == CDDNames::KOG_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "oasis_kog");
    } else if (settings.dbName == CDDNames::PRK_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "oasis_prk");
    } else if (settings.dbName == CDDNames::TIGR_DB()) {
        addParametr(cfg.params, ReqParams::cdd_db, "oasis_tigr");
    } else {
        assert(0);
    }

    cfg.query = settings.query;
    cfg.retries = 60;
    cfg.filterResult = 0;

    task = new RemoteBLASTTask(cfg);
}

}
