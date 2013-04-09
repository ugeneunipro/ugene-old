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

#include "WorkflowUtils.h"

#include <U2Lang/BaseTypes.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/Dataset.h>
#include <U2Lang/Datatype.h>
#include <U2Lang/Descriptor.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/IntegralBus.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/Settings.h>
#include <U2Core/StringAdapter.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

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
const QString WorkflowUtils::HREF_PARAM_ID("param");

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
        o.setProperty(i.key().toLatin1(), i.value());
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

static bool validateParameters(const Schema &schema, QList<ValidateError> &infoList, const Iteration *it, const QMap<ActorId, ActorId> &map) {
    bool good = true;
    foreach (Actor* a, schema.getProcesses()) {
        QStringList l;
        good &= a->validate(l);
        foreach(const QString &s, l) {
            QString error = s;
            QString id = a->getId();
            if (NULL != it) {
                error = QObject::tr("Iteration '%1': %2").arg(it->name).arg(s);
                id = map.key(a->getId());
            }
            ValidateError item;
            item[TEXT_REF] = error;
            item[ACTOR_REF] = id;
            if (NULL != it) {
                item[ITERATION_REF] = it->id;
            }
            infoList << item;
        }
    }
    return good;
}

static bool validateExternalTools(Actor *a, QList<ValidateError> &infoList) {
    bool good = true;
    QStrStrMap tools = a->getProto()->getExternalTools();
    foreach (const QString &toolId, tools.keys()) {
        Attribute *attr = a->getParameter(tools[toolId]);
        ExternalTool *tool = AppContext::getExternalToolRegistry()->getByName(toolId);
        SAFE_POINT(NULL != tool, "NULL tool", false);

        bool fromAttr = (NULL != attr) && !attr->isDefaultValue();
        bool valid = fromAttr ? !attr->isEmpty() : !tool->getPath().isEmpty();
        if (!valid) {
            good = false;
            ValidateError item;
            item[TEXT_REF] = QString("%1 : %2").arg(a->getLabel()).arg(WorkflowUtils::externalToolError(tool->getName()));
            item[ACTOR_REF] = a->getId();
            infoList << item;
        }
    }
    return good;
}

static bool validatePorts(Actor *a, QList<ValidateError> &infoList) {
    bool good = true;
    foreach(Port *p, a->getPorts()) {
        QStringList l;
        good &= p->validate(l);
        if (!l.isEmpty()) {
            foreach(QString s, l) {
                ValidateError item;
                item[TEXT_REF] = QString("%1 : %2").arg(a->getLabel()).arg(s);
                item[PORT_REF] = p->getId();
                item[ACTOR_REF] = a->getId();
                infoList << item;
            }
        }
    }
    return good;
}

static bool validateScript(Actor *a, QList<ValidateError> &infoList) {
    QScopedPointer<WorkflowScriptEngine> engine(new WorkflowScriptEngine(NULL));
    QScriptSyntaxCheckResult syntaxResult = engine->checkSyntax(a->getScript()->getScriptText());

    if (syntaxResult.state() != QScriptSyntaxCheckResult::Valid) {
        ValidateError item;
        item[TEXT_REF] = QObject::tr("%1 : Script syntax check failed! Line: %2, error: %3")
            .arg(a->getLabel())
            .arg(syntaxResult.errorLineNumber())
            .arg(syntaxResult.errorMessage());
        item[ACTOR_REF] = a->getId();
        infoList << item;
        return false;
    }
    return true;
}

bool WorkflowUtils::validate(const Schema &schema, QList<ValidateError> &infoList) {
    bool good = true;
    std::auto_ptr<WorkflowScriptEngine> engine(new WorkflowScriptEngine(NULL));
    foreach (Actor *a, schema.getProcesses()) {
        good &= validatePorts(a, infoList);

        if (a->getProto()->isScriptFlagSet()) {
            good &= validateScript(a, infoList);
        }

        good &= validateExternalTools(a, infoList);
    }

    if (0 == schema.getIterations().size()) {
        good &= validateParameters(schema, infoList, NULL, QMap<ActorId, ActorId>());
    }

    foreach (const Iteration& it, schema.getIterations()) {
        Schema sh;
        U2OpStatusImpl os;
        QMap<ActorId, ActorId> map = HRSchemaSerializer::deepCopy(schema, &sh, os);
        SAFE_POINT_OP(os, false);
        sh.applyConfiguration(it, map);

        good &= validateParameters(sh, infoList, &it, map);
    }
    return good;
}

// used in GUI schema validating
bool WorkflowUtils::validate(const Schema &schema, QList<QListWidgetItem*> &infoList) {
    QList<ValidateError> errors;
    bool good = validate(schema, errors);

    foreach (const ValidateError &error, errors) {
        if (!error.contains(ACTOR_REF)) {
            continue;
        }
        Actor *a = schema.actorById(error[ACTOR_REF].toString());
        QListWidgetItem *item = new QListWidgetItem(
            a->getProto()->getIcon(),
            QString("%1 : %2").arg(a->getLabel()).arg(error[TEXT_REF].toString()));
        foreach (int ref, error.keys()) {
            item->setData(ref, error[ref]);
        }
        infoList << item;
    }

    return good;
}

// used in cmdline schema validating
bool WorkflowUtils::validate(const Workflow::Schema &schema, QStringList &errs) {
    QList<ValidateError> errors;
    bool good = validate(schema, errors);

    foreach (const ValidateError &error, errors) {
        if (!error.contains(ACTOR_REF)) {
            continue;
        }
        Actor *a = schema.actorById(error[ACTOR_REF].toString());
        QString message = error[TEXT_REF].toString();
        QString res = QString("%1: %2").arg(a->getLabel()).arg(message);

        QString option;
        foreach (const Attribute *attr, a->getAttributes()) {
            if (message.contains(attr->getDisplayName())) {
                option = a->getParamAliases().value(attr->getId());
            }
        }
        if (!option.isEmpty()) {
            res += tr(" (use --%1 option)").arg(option);
        }
        errs << res;
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
        return idx >= 0 ? candidates.at(idx) : EMPTY_VALUES_DESC;
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
        bt->addInputs(p, false);
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

bool WorkflowUtils::isPathExist(const Port *src, const Port *dest) {
    assert(src->isOutput());
    assert(dest->isInput());
    if (src == dest) {
        return true;
    }
    Port *const src1 = const_cast<Port*>(src);

    foreach (const Link *l, src->getLinks().values(src1)) {
        const Port *p = l->destination();
        if (l->source() != src) {
            continue;
        }
        if (dest == p) {
            return true;
        } else {
            bool result = isPathExist(p, dest);
            if (result) {
                return true;
            }
        }
    }
    return false;
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
    if (value.canConvert< QList<Dataset> >()) {
        QString res;
        foreach (const Dataset &dSet, value.value< QList<Dataset> >()) {
            res += dSet.getName() + "; ";
        }
        return res;
    }
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

QString WorkflowUtils::getParamIdFromHref( const QString& href ) {
    QStringList args = href.split('&');
    const QString& prefix = QString("%1:").arg(HREF_PARAM_ID);
    QString id;
    foreach(QString arg, args) {
        if (arg.startsWith(prefix)) {
            id = arg.mid(prefix.length());
            break;
        }
    }
    return id;
}

static void data2text(WorkflowContext *context, DocumentFormatId formatId, GObject *obj, QString &text) {
    QList<GObject*> objList;
    objList << obj;

    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::STRING);
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
    std::auto_ptr<Document> d(new Document(df, iof, GUrl(), context->getDataStorage()->getDbiRef(), objList));
    d->setDocumentOwnsDbiResources(false);
    StringAdapter *io = dynamic_cast<StringAdapter*>(iof->createIOAdapter());
    io->open(GUrl(), IOAdapterMode_Write);
    U2OpStatusImpl os;

    df->storeDocument(d.get(), io, os);

    text += io->getBuffer();
    io->close();
}

#define STRING_TYPE QVariant::String
#define SEQUENCE_TYPE QVariant::ByteArray
#define MSA_TYPE QVariant::UserType
#define ANNOTATIONS_TYPE QVariant::List

void WorkflowUtils::print(const QString &slotString, const QVariant &data, DataTypePtr type, WorkflowContext *context) {
    QString text = slotString + ":\n";
    if ("string" == type->getId()
        || BaseTypes::STRING_LIST_TYPE() == type) {
        text += data.toString();
    } else if (BaseTypes::DNA_SEQUENCE_TYPE() == type) {
        U2SequenceObject *obj = StorageUtils::getSequenceObject(context->getDataStorage(), data.value<SharedDbiDataHandler>());
        CHECK(NULL != obj, );
        data2text(context, BaseDocumentFormats::FASTA, obj, text);
    } else if (BaseTypes::MULTIPLE_ALIGNMENT_TYPE() == type) {
        MAlignmentObject *obj = StorageUtils::getMsaObject(context->getDataStorage(), data.value<SharedDbiDataHandler>());
        CHECK(NULL != obj, );
        data2text(context, BaseDocumentFormats::CLUSTAL_ALN, obj, text);
    } else if (BaseTypes::ANNOTATION_TABLE_TYPE() == type
        || BaseTypes::ANNOTATION_TABLE_LIST_TYPE() == type) {
        QList<SharedAnnotationData> anns;
        if (BaseTypes::ANNOTATION_TABLE_TYPE() == type) {
            anns = data.value<QList<SharedAnnotationData> >();
        } else {
            anns = QVariantUtils::var2ftl(data.toList());
        }
        AnnotationTableObject *obj = new AnnotationTableObject("Slot annotations");
        foreach(SharedAnnotationData d, anns) {
            obj->addAnnotation(new Annotation(d));
        }
        data2text(context, BaseDocumentFormats::PLAIN_GENBANK, obj, text);
    } else {
        text += "Can not print data of this type: " + type->getDisplayName();
    }
    printf("\n%s\n", text.toLatin1().data());
}

bool WorkflowUtils::validateSchemaForIncluding(const Schema &s, QString &error) {
    // TEMPORARY disallow filter and grouper elements in includes
    static QString errorStr = tr("The %1 element is a %2. Sorry, but current version of "
        "UGENE doesn't support of filters and groupers in the includes.");
    foreach (Actor *actor, s.getProcesses()) {
        ActorPrototype *proto = actor->getProto();
        if (proto->getInfluenceOnPathFlag() || CoreLibConstants::GROUPER_ID == proto->getId()) {
            error = errorStr;
            error = error.arg(actor->getLabel());
            if (proto->getInfluenceOnPathFlag()) {
                error = error.arg(tr("filter"));
            } else {
                error = error.arg(tr("grouper"));
            }
            return false;
        }
    }

    const QList<PortAlias> &portAliases = s.getPortAliases();
    if (portAliases.isEmpty()) {
        error = tr("The schema has not any aliased ports");
        return false;
    }

    foreach (Actor *actor, s.getProcesses()) {
        // check that free input ports are aliased
        foreach (Port *port, actor->getPorts()) {
            if (!port->isInput()) {
                continue;
            }
            if (!port->getLinks().isEmpty()) {
                continue;
            }
            bool aliased = false;
            foreach (const PortAlias &alias, portAliases) {
                if (alias.getSourcePort() == port) {
                    if (alias.getSlotAliases().isEmpty()) {
                        error = tr("The aliased port %1.%2 has no aliased slots").arg(actor->getLabel()).arg(port->getDisplayName());
                        return false;
                    } else {
                        aliased = true;
                        break;
                    }
                }
            }
            if (!aliased) {
                error = tr("The free port %1.%2 is not aliased").arg(actor->getLabel()).arg(port->getId());
                return false;
            }
        }

        // check that every required attribute is aliased or has set value
        const QMap<QString, QString> &paramAliases = actor->getParamAliases();
        foreach (const QString &attrName, actor->getParameters().keys()) {
            Attribute *attr = actor->getParameters().value(attrName);
            if (attr->isRequiredAttribute()) {
                if (!paramAliases.contains(attr->getId())) {
                    QVariant val = attr->getAttributeValueWithoutScript<QVariant>();
                    if (val.isNull()) {
                        error = tr("The required parameter %1.%2 is empty and not aliased").arg(actor->getLabel()).arg(attr->getDisplayName());
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

void WorkflowUtils::extractPathsFromBindings(QStrStrMap &busMap, SlotPathMap &pathMap) {
    QString srcId;
    QStringList path;
    foreach (const QString &dest, busMap.keys()) {
        QStringList srcs = busMap.value(dest).split(";");
        foreach (const QString &src, srcs) {
            BusMap::parseSource(src, srcId, path);
            if (!path.isEmpty()) {
                QPair<QString, QString> slotPair(dest, srcId);
                busMap[dest] = srcId;
                pathMap.insertMulti(slotPair, path);
            }
        }
    }
}

void WorkflowUtils::applyPathsToBusMap(QStrStrMap &busMap, const SlotPathMap &pathMap) {
    foreach (const QString &dest, busMap.keys()) {
        QStringList newSrcs;

        QStringList srcs = busMap.value(dest).split(";");
        QStringList uniqList;
        foreach(QString src, srcs) {
            if (!uniqList.contains(src)) {
                uniqList << src;
            }
        }

        foreach (const QString &src, uniqList) {
            QPair<QString, QString> slotPair(dest, src);
            if (pathMap.contains(slotPair)) {
                QList<QStringList> paths = pathMap.values(slotPair);
                if (!paths.isEmpty()) {
                    foreach (const QStringList &path, paths) {
                        QString newSrc = src + ">" + path.join(",");
                        newSrcs << newSrc;
                    }
                }
            } else {
                newSrcs << src;
            }
        }
        busMap[dest] = newSrcs.join(";");
    }
}

static bool pathExists(Actor *start, Port *end, QStringList path) {
    path.removeOne(start->getId());

    foreach (Port *p, start->getOutputPorts()) {
        foreach (Port *out, p->getLinks().keys()) {
            if (out == end) {
                return path.isEmpty();
            }
            bool res = pathExists(out->owner(), end, path);
            if (res) {
                return true;
            }
        }
    }
    return false;
}

#define WIN_LAUNCH_CMD_COMMAND "cmd /C "
#define START_WAIT_MSEC 3000

bool WorkflowUtils::startExternalProcess(QProcess *process, const QString &program, const QStringList &arguments) {
    process->start(program, arguments);
    bool started = process->waitForStarted(START_WAIT_MSEC);

#ifdef Q_OS_WIN32
    if(!started) {
        QString execStr = WIN_LAUNCH_CMD_COMMAND + program;
        foreach (const QString arg, arguments) {
            execStr += " " + arg;
        }
        process->start(execStr);
        coreLog.trace(tr("Can't run an executable file \"%1\" as it is. Try to run it as a cmd line command: \"%2\"")
            .arg(program).arg(execStr));
        started = process->waitForStarted(START_WAIT_MSEC);
    }
#endif

    return started;
}

QStringList WorkflowUtils::getDatasetsUrls(const QList<Dataset> &sets) {
    QStringList result;
    foreach (const Dataset &dSet, sets) {
        foreach (URLContainer *url, dSet.getUrls()) {
            result << url->getUrl();
        }
    }
    return result;
}

Actor * WorkflowUtils::actorById(const QList<Actor*> &actors, const ActorId &id) {
    foreach(Actor *a, actors) {
        if (a->getId() == id) {
            return a;
        }
    }
    return NULL;
}

QMap<Descriptor, DataTypePtr> WorkflowUtils::getBusType(Port *inPort) {
    QMap<Port*,Link*> links = inPort->getLinks();
    if (links.size() == 1) {
        Port *src = links.keys().first();
        assert(src->isOutput());
        IntegralBusPort *bus = dynamic_cast<IntegralBusPort*>(src);
        assert(NULL != bus);
        DataTypePtr type = bus->getType();
        return type->getDatatypesMap();
    }
    return QMap<Descriptor, DataTypePtr>();
}

bool WorkflowUtils::isBindingValid(const QString &srcSlotId, const QMap<Descriptor, DataTypePtr> &srcBus,
    const QString &dstSlotId, const QMap<Descriptor, DataTypePtr> &dstBus) {
    DataTypePtr srcType;
    // Check that incoming bus contains source slot
    bool found = false;
    foreach (const Descriptor &d,  srcBus.keys()) {
        if (d.getId() == srcSlotId) {
            srcType = srcBus.value(d);
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }

    // Check that source and destination slots have equal types
    foreach (const Descriptor &d, dstBus.keys()) {
        if (d.getId() == dstSlotId) {
            DataTypePtr destType = dstBus.value(d);
            QString stringTypeId("string");
            if (destType == srcType) {
                return true;
            } else if (destType == BaseTypes::ANNOTATION_TABLE_TYPE()) {
                return (srcType == BaseTypes::ANNOTATION_TABLE_LIST_TYPE());
            } else if (destType == BaseTypes::ANNOTATION_TABLE_LIST_TYPE()) {
                return (srcType == BaseTypes::ANNOTATION_TABLE_TYPE());
            } else if (destType->getId() == stringTypeId) {
                return (srcType == BaseTypes::STRING_LIST_TYPE());
            } else if (destType == BaseTypes::STRING_LIST_TYPE()) {
                return (srcType->getId() == stringTypeId);
            }
            break;
        }
    }

    return false;
}

QString WorkflowUtils::createUniqueString(const QString &str, const QString &sep, const QStringList &uniqueStrs) {
    QString result = str;
    int number = 0;
    bool found = false;
    foreach (const QString &uniq, uniqueStrs) {
        if (uniq == str) {
            found = true;
            number = qMax(number, 1);
        } else {
            int idx = uniq.lastIndexOf(sep);
            if (-1 != idx) {
                QString left = uniq.left(idx);
                if (str == left) {
                    QString right = uniq.mid(idx + 1);
                    bool ok = false;
                    int num = right.toInt(&ok);
                    if(ok) {
                        found = true;
                        number = qMax(number, num + 1);
                    }
                }
            }
        }
    }

    if (found) {
        result += sep + QString::number(number);
    }
    return result;
}

QString WorkflowUtils::updateExternalToolPath(const QString &toolName, const QString &path) {
    ExternalToolRegistry *registry = AppContext::getExternalToolRegistry();
    SAFE_POINT(NULL != registry, "NULL external tool registry", "");
    ExternalTool *tool = registry->getByName(toolName);
    SAFE_POINT(NULL != tool, QString("Unknown tool: %1").arg(toolName), "");

    if (QString::compare(path, "default", Qt::CaseInsensitive) != 0) {
        tool->setPath(path);
    }
    return tool->getPath();
}

QString WorkflowUtils::externalToolError(const QString &toolName) {
    return tr("External tool \"%1\" is not set. You can set it in Settings -> Preferences -> External Tools").arg(toolName);
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
    QVariant urlVar = getParameter(id);
    QString url;
    if (urlVar.canConvert< QList<Dataset> >()) {
        QStringList urls = WorkflowUtils::getDatasetsUrls(urlVar.value< QList<Dataset> >());
        url = urls.join(";");
    } else {
        url = getParameter(id).toString();
    }
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
    CHECK(NULL != input, "");
    QList<Actor*> producers = input->getProducers(slot);

    QStringList labels;
    foreach(Actor* a, producers) {
        labels << a->getLabel();
    }
    return labels.join(", ");
}

QString PrompterBaseImpl::getProducersOrUnset(const QString &port, const QString &slot) {
    static const QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString prods = getProducers(port, slot);
    return prods.isEmpty() ? unsetStr : prods;
}

QString PrompterBaseImpl::getHyperlink(const QString& id, const QString& val) {
    return QString("<a href=%1:%2>%3</a>").arg(WorkflowUtils::HREF_PARAM_ID).arg(id).arg(val);
}

QString PrompterBaseImpl::getHyperlink(const QString& id, int val) {
    return getHyperlink(id, QString::number(val));
}

QString PrompterBaseImpl::getHyperlink(const QString& id, qreal val) {
    return getHyperlink(id, QString::number(val));
}

}//namespace
