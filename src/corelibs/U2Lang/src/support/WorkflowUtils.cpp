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

#include "WorkflowUtils.h"

#include <U2Lang/Descriptor.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Core/GObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Settings.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentUtils.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QListWidgetItem>

namespace U2 {

/*****************************
 * WorkflowUtils
 *****************************/
const QStringList WorkflowUtils::WD_FILE_EXTENSIONS = initExtensions();
const QString WorkflowUtils::WD_XML_FORMAT_EXTENSION("uws");

QStringList WorkflowUtils::initExtensions() {
    QStringList exts;
    exts << "uwl";
    return exts;
}

QString WorkflowUtils::getRichDoc(const Descriptor& d) {
    if(d.getDisplayName().isEmpty()) {
        if( d.getDocumentation().isEmpty() ) {
            return QString();
        } else {
            return QString("%1").arg(d.getDocumentation());
        }
    } else {
        if( d.getDocumentation().isEmpty() ) {
            return QString("<b>%1</b>").arg(d.getDisplayName());
        } else {
            return QString("<b>%1</b> : %2").arg(d.getDisplayName()).arg(d.getDocumentation());
        }
    }
}

QString WorkflowUtils::getDropUrl(QList<DocumentFormat*>& fs, const QMimeData* md) {
    QString url;
    const GObjectMimeData* gomd = qobject_cast<const GObjectMimeData*>(md);
    const DocumentMimeData* domd = qobject_cast<const DocumentMimeData*>(md);
    if (gomd) {
        GObject* obj = gomd->objPtr.data();
        if (obj) {
            fs << obj->getDocument()->getDocumentFormat();
            url = obj->getDocument()->getURLString();
        }
    } else if (domd) {
        Document* doc = domd->objPtr.data();
        if (doc) {
            fs << doc->getDocumentFormat();
            url = doc->getURLString();
        }
    } else if (md->hasUrls()) {
        QList<QUrl> urls = md->urls();
        if (urls.size() == 1) {
            url = urls.first().toLocalFile();
            QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(url);
            foreach(const FormatDetectionResult& di, formats) {
                fs << di.format;
            }
        }
    }
    return url;
}

void WorkflowUtils::setQObjectProperties(QObject &o , const QVariantMap & params) {
    QMapIterator<QString, QVariant> i(params);
    while (i.hasNext()) {
        i.next();
        //log.debug("set param " + i.key() + "="+i.value().toString());
        o.setProperty(i.key().toAscii(), i.value());
    }
}

QStringList WorkflowUtils::expandToUrls(const QString& s) {
    QStringList urls = s.split(";");
    QStringList result;
    QRegExp wcard("[*?\\[\\]]");
    foreach(QString url, urls) 
    {
        int idx = url.indexOf(wcard);
        if (idx >= 0) {
            int dirIdx = url.lastIndexOf('/', idx);
            QDir dir;
            if (dirIdx >= 0) {
                dir = QDir(url.left(dirIdx));
                url = url.right(url.length() - dirIdx - 1);
            }

            foreach(QFileInfo fi, dir.entryInfoList((QStringList() << url), QDir::Files|QDir::NoSymLinks)) {
                result << fi.absoluteFilePath();
            }
        } else {
            //if (QFile::exists(url)) 
            {
                result << url;
            }
        }
    }
    return result;
}

bool WorkflowUtils::validate(const Schema& schema, QList<QListWidgetItem*>* infoList) {
    bool good = true;
    foreach (Actor* a, schema.getProcesses()) {
        foreach(Port* p, a->getPorts()) {
            QStringList l;
            bool ag = p->validate(l);
            good &= ag;
            if (infoList && !l.isEmpty()) {
                foreach(QString s, l) {
                    QListWidgetItem* item = new QListWidgetItem(a->getProto()->getIcon(), 
                        QString("%1 : %2").arg(a->getLabel()).arg(s));
                    item->setData(PORT_REF, p->getId());
                    item->setData(ACTOR_REF, a->getId());
                    infoList->append(item);
                }
            }
        }
    }

    foreach (const Iteration& it, schema.getIterations()) {
        Schema sh;
        QMap<ActorId, ActorId> map = HRSchemaSerializer::deepCopy(schema, &sh);
        sh.applyConfiguration(it, map);

        foreach (Actor* a, sh.getProcesses()) {
            QStringList l;
            bool ag = a->validate(l);
            good &= ag;
            if (infoList && !l.isEmpty()) {
                foreach(QString s, l) {
                    QListWidgetItem* item = new QListWidgetItem(a->getProto()->getIcon(), 
                        tr("Iteration '%3', %1 : %2").arg(a->getLabel()).arg(s).arg(it.name));
                    item->setData(ACTOR_REF, map.key(a->getId()));
                    item->setData(ITERATION_REF, it.id);
                    infoList->append(item);
                }
            }
        }
    }
    return good;
}

bool WorkflowUtils::validate(const Schema& schema, QList<QMap<int, QVariant> >* infoList) {
    bool good = true;
    foreach (Actor* a, schema.getProcesses()) {
        foreach(Port* p, a->getPorts()) {
            QStringList l;
            bool ag = p->validate(l);
            good &= ag;
            if (infoList && !l.isEmpty()) {
                foreach(QString s, l) {
                    QMap<int, QVariant> item;
                    item[TEXT_REF] = QString("%1 : %2").arg(a->getLabel()).arg(s);
                    item[PORT_REF] = p->getId();
                    item[ACTOR_REF] = a->getId();
                    infoList->append(item);
                }
            }
        }
    }

    foreach (const Iteration& it, schema.getIterations()) {
        Schema sh;
        QMap<ActorId, ActorId> map = HRSchemaSerializer::deepCopy(schema, &sh);
        sh.applyConfiguration(it, map);

        foreach (Actor* a, sh.getProcesses()) {
            QStringList l;
            bool ag = a->validate(l);
            good &= ag;
            if (infoList && !l.isEmpty()) {
                foreach(QString s, l) {
                    QMap<int, QVariant> item;
                    item[TEXT_REF] = tr("Iteration '%3', %1 : %2").arg(a->getLabel()).arg(s).arg(it.name);
                    item[ACTOR_REF] = map.key(a->getId());
                    item[ITERATION_REF] = it.id;
                    infoList->append(item);
                }
            }
        }
    }
    return good;
}

// used in cmdline schema validating
bool WorkflowUtils::validate( const Workflow::Schema& schema, QStringList & errs ) {
    bool good = true;
    foreach (Actor* a, schema.getProcesses()) {
        foreach(Port* p, a->getPorts()) {
            QStringList l;
            good &= p->validate(l);
            foreach(const QString & s, l) {
                errs.append(QString("%1 : %2").arg(a->getLabel()).arg(s));
            }
        }
    }
    
    foreach (const Iteration& it, schema.getIterations()) {
        Schema sh;
        QMap<ActorId, ActorId> map = HRSchemaSerializer::deepCopy(schema, &sh);
        sh.applyConfiguration(it, map);
        
        foreach (Actor* a, sh.getProcesses()) {
            foreach( Attribute * attr, a->getParameters() ) {
                assert(attr != NULL);
                if( attr->isRequiredAttribute() && (attr->isEmpty() || attr->isEmptyString()) ) {
                    good = false;
                    errs.append(tr("%2: Required parameter is not set: %1 (use --%3 option)").
                        arg(attr->getDisplayName()).arg(a->getLabel()).arg(a->getParamAliases().value(attr->getId())));
                }
            }
            ConfigurationValidator * baseValidator = a->getValidator();
            ScreenedParamValidator * screenedParamValidator = dynamic_cast<ScreenedParamValidator*>(baseValidator);
            if(screenedParamValidator != NULL) {
                QString err = screenedParamValidator->validate(a);
                if( !err.isEmpty() ) {
                    good = false;
                    errs.append( QString("%3: %1 (use --%2 option)").arg(err).arg(
                        a->getParamAliases().value(a->getParameter(screenedParamValidator->getId())->getId())).arg(a->getLabel()));
                }
            } else if( baseValidator != NULL ) {
                QStringList l;
                good &= baseValidator->validate(a, l);
                foreach(const QString & s, l) {
                    errs.append(QString("%1: %2").arg(a->getLabel()).arg(s));
                }
            }
        }
    }
    return good;
}

QList<Descriptor> WorkflowUtils::findMatchingTypes(DataTypePtr set, DataTypePtr elementDataType) {
    QList<Descriptor> result;
    foreach(const Descriptor& d, set->getAllDescriptors()) {
        if (set->getDatatypeByDescriptor(d) == elementDataType) {
            result.append(d);
        }
    }
    return result;
}

QStringList WorkflowUtils::findMatchingTypesAsStringList(DataTypePtr set, DataTypePtr elementDatatype) {
    QList<Descriptor> descList = findMatchingTypes(set, elementDatatype);
    QStringList res;
    foreach( const Descriptor & desc, descList ) {
        res << desc.getId();
    }
    return res;
}

const Descriptor EMPTY_VALUES_DESC("", WorkflowUtils::tr("<empty>"), WorkflowUtils::tr("Default value"));

QList<Descriptor> WorkflowUtils::findMatchingCandidates(DataTypePtr from, DataTypePtr elementDatatype) {
    QList<Descriptor> candidates = findMatchingTypes(from, elementDatatype);
    if (elementDatatype->isList()) {
        candidates += findMatchingTypes(from, elementDatatype->getDatatypeByDescriptor());
    } else {
        candidates.append(EMPTY_VALUES_DESC);
    }
    return candidates;
}

QList<Descriptor> WorkflowUtils::findMatchingCandidates(DataTypePtr from, DataTypePtr to, const Descriptor & key) {
    return findMatchingCandidates(from, to->getDatatypeByDescriptor(key));
}

Descriptor WorkflowUtils::getCurrentMatchingDescriptor(const QList<Descriptor> & candidates, DataTypePtr to, 
                                                       const Descriptor & key, const QStrStrMap & bindings) {
    DataTypePtr elementDatatype = to->getDatatypeByDescriptor(key);
    if (elementDatatype->isList()) {
        QString currentVal = bindings.value(key.getId());
        if (!currentVal.isEmpty()) {
            return Descriptor(currentVal, tr("<List of values>"), tr("List of values"));
        } else {
            return EMPTY_VALUES_DESC;
        }
    } else {
        int idx = bindings.contains(key.getId()) ? candidates.indexOf(bindings.value(key.getId())) : 0;
        return idx >= 0 ? candidates.at(idx) : candidates.first();
    }
}

DataTypePtr WorkflowUtils::getToDatatypeForBusport(IntegralBusPort * p) {
    assert(p != NULL);
    DataTypePtr to;
    DataTypePtr t = to = p->getType();
    if (!t->isMap()) {
        QMap<Descriptor, DataTypePtr> map;
        map.insert(*p, t);
        to = new MapDataType(Descriptor(), map);
        //IntegralBusType* bt = new IntegralBusType(Descriptor(), QMap<Descriptor, DataTypePtr>());
        //bt->addOutput(t, p);
    }
    return to;
}

DataTypePtr WorkflowUtils::getFromDatatypeForBusport(IntegralBusPort * p, DataTypePtr to) {
    assert(p != NULL);
    
    DataTypePtr from;
    if (p->isOutput() || p->getWidth() == 0) {
        //nothing to edit, go info mode
        from = to;
    } else {
        //port is input and has links, go editing mode
        IntegralBusType* bt = new IntegralBusType(Descriptor(), QMap<Descriptor, DataTypePtr>());
        bt->addInputs(p);
        from = bt;
    }
    return from;
}

QString WorkflowUtils::findPathToSchemaFile(const QString & name) {
    // full path given
    if( QFile::exists( name ) ) {
        return name;
    }
    // search schema in data dir
    QString filenameWithDataPrefix = QString( PATH_PREFIX_DATA ) + ":" + "cmdline/" + name;
    if( QFile::exists( filenameWithDataPrefix ) ) {
        return filenameWithDataPrefix;
    }
    foreach(const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS) {
        QString filenameWithDataPrefixAndExt = QString( PATH_PREFIX_DATA ) + ":" + "cmdline/" + name + "." + ext;
        if( QFile::exists( filenameWithDataPrefixAndExt ) ) {
            return filenameWithDataPrefixAndExt;
        }
    }
    
    // if no such file found -> search name in settings. user saved schemas
    Settings * settings = AppContext::getSettings();
    assert( settings != NULL );
    
    // FIXME: same as WorkflowSceneIOTasks::SCHEMA_PATHS_SETTINGS_TAG
    QVariantMap pathsMap = settings->getValue( "workflow_settings/schema_paths" ).toMap();
    QString path = pathsMap.value( name ).toString();
    if( QFile::exists( path ) ) {
        return path;
    }
    return QString();
}

void WorkflowUtils::getLinkedActorsId(Actor *a, QList<QString> &linkedActors) {
    if(!linkedActors.contains(a->getId())) {
        linkedActors.append( a->getId() );
        foreach(Port *p, a->getPorts()) {
            foreach(Port *pp, p->getLinks().keys()) {
                getLinkedActorsId(pp->owner(), linkedActors);
            }
        }
    } else {
        return;
    }
}

Descriptor WorkflowUtils::getSlotDescOfDatatype(const DataTypePtr & dt) {
    QString dtId = dt->getId();
    if(dtId == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
        return BaseSlots::DNA_SEQUENCE_SLOT();
    }
    if(dtId == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
        return BaseSlots::ANNOTATION_TABLE_SLOT();
    }
    if(dtId == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
        return BaseSlots::MULTIPLE_ALIGNMENT_SLOT();
    }
    if(dtId == BaseTypes::STRING_TYPE()->getId()) {
        return BaseSlots::TEXT_SLOT();
    }
    assert(false);
    return *dt;
}

static QStringList initLowerToUpperList() {
    QStringList res;
    res << "true";
    res << "false";
    return res;
}
static const QStringList lowerToUpperList = initLowerToUpperList();

QString WorkflowUtils::getStringForParameterDisplayRole(const QVariant & value) {
    QString str = value.toString();
    if(lowerToUpperList.contains(str)) {
        return str.at(0).toUpper() + str.mid(1);
    }
    return str;
}

Actor * WorkflowUtils::findActorByParamAlias(const QList<Actor*> & procs, const QString & alias, QString & attrName, bool writeLog) {
    QList<Actor*> actors;
    foreach(Actor * actor, procs) {
        assert( actor != NULL );
        if(actor->getParamAliases().values().contains(alias)) {
            actors << actor;
        }
    }

    if (actors.isEmpty()) {
        return NULL;
    } else if( actors.size() > 1 ) {
        if(writeLog) {
            coreLog.error(WorkflowUtils::tr("%1 actors in schema have '%2' alias").arg(actors.size()).arg(alias));
        }
    }
    
    Actor * ret = actors.first();
    attrName = ret->getParamAliases().key( alias );
    return ret;
}

/*****************************
 * PrompterBaseImpl
 *****************************/
QVariant PrompterBaseImpl::getParameter(const QString& id) {
    if (map.contains(id)) {
        return map.value(id);
    } else {
        return target->getParameter(id)->getAttributePureValue();
    }
}

QString PrompterBaseImpl::getURL(const QString& id, bool * empty ) {
    QString url = getParameter(id).toString();
    if( empty != NULL ) { *empty = false; }
    if( !target->getParameter(id)->getAttributeScript().isEmpty() ) {
        url = "got from user script";
    } else if (url.isEmpty()) {
        url = "<font color='red'>"+tr("unset")+"</font>";
        if( empty != NULL ) { *empty = true; }
    } else if (url.indexOf(";") != -1) {
        url = tr("the list of files");
    } else {
        url = QFileInfo(url).fileName();
    }
    return url;
}

QString PrompterBaseImpl::getRequiredParam(const QString& id) {
    QString url = getParameter(id).toString();
    if (url.isEmpty()) {
        url = "<font color='red'>"+tr("unset")+"</font>";
    }
    return url;
}

QString PrompterBaseImpl::getScreenedURL(IntegralBusPort* input, const QString& id, const QString& slot) {
    bool empty = false;
    QString attrUrl = QString("<u>%1</u>").arg(getURL(id, &empty));
    if( !empty ) {
        return attrUrl;
    }
    
    Actor * origin = input->getProducer( slot );
    QString slotUrl;
    if( origin != NULL ) {
        slotUrl = tr("file(s) alongside of input sources of <u>%1</u>").arg(origin->getLabel());
        return slotUrl;
    }
    
    assert( !attrUrl.isEmpty() );
    return attrUrl;
}

QString PrompterBaseImpl::getProducers( const QString& port, const QString& slot )
{
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(port));
    QList<Actor*> producers = input->getProducers(slot);

    QStringList labels;
    foreach(Actor* a, producers) {
        labels << a->getLabel();
    }
    return labels.join(", ");
}

QString PrompterBaseImpl::getHyperlink(const QString& id, const QString& val) {
    return QString("<a href=param:%1>%2</a>").arg(id).arg(val);
}

QString PrompterBaseImpl::getHyperlink(const QString& id, int val) {
    return getHyperlink(id, QString::number(val));
}

QString PrompterBaseImpl::getHyperlink(const QString& id, qreal val) {
    return getHyperlink(id, QString::number(val));
}

}//namespace
