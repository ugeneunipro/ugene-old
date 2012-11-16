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

#include <U2Core/U2SafePoints.h>

#include "HRWizardSerializer.h"

namespace U2 {

const QString HRWizardParser::WIZARD("wizard");
const QString HRWizardParser::NAME("name");
const QString HRWizardParser::PAGE("page");
const QString HRWizardParser::ID("id");
const QString HRWizardParser::NEXT("next");
const QString HRWizardParser::TITLE("title");
const QString HRWizardParser::TEMPLATE("template");
const QString HRWizardParser::TYPE("type");
const QString HRWizardParser::LOGO_PATH("logo-path");
const QString HRWizardParser::DEFAULT("default");
const QString HRWizardParser::HIDEABLE("hideable");
const QString HRWizardParser::LABEL_SIZE("label-size");
const QString HRWizardParser::ELEMENT_ID("element-id");
const QString HRWizardParser::PROTOTYPE("prototype");
const QString HRWizardParser::VALUE("value");
const QString HRWizardParser::PORT_MAPPING("port-mapping");
const QString HRWizardParser::SLOTS_MAPPRING("slots-mapping");
const QString HRWizardParser::SRC_PORT("src-port");
const QString HRWizardParser::DST_PORT("dst-port");

HRWizardParser::HRWizardParser(HRSchemaSerializer::Tokenizer &_tokenizer,
                                       const QMap<QString, Actor*> &_actorMap)
: tokenizer(_tokenizer), actorMap(_actorMap)
{
    wizardName = Wizard::DEFAULT_NAME;
}

HRWizardParser::~HRWizardParser() {
    qDeleteAll(pagesMap);
}

Wizard * HRWizardParser::takeResult() {
    QList<WizardPage*> retPages = pages;
    pages.clear();
    pagesMap.clear();

    Wizard *result = new Wizard(wizardName, retPages);
    foreach (const QString &name, vars.keys()) {
        result->addVariable(vars[name]);
    }
    return result;
}

Wizard * HRWizardParser::parseWizard(U2OpStatus &os) {
    while (tokenizer.look() != HRSchemaSerializer::BLOCK_END) {
        QString tok = tokenizer.take();
        if (PAGE == tok) {
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_START);
            parsePage(os);
            CHECK_OP(os, NULL);
            tokenizer.assertToken(HRSchemaSerializer::BLOCK_END);
        } else if (NAME == tok) {
            tokenizer.assertToken(HRSchemaSerializer::EQUALS_SIGN);
            wizardName = tokenizer.take();
        }
    }

    finilizePagesOrder(os);
    CHECK_OP(os, NULL);

    return takeResult();
}

void HRWizardParser::parsePage(U2OpStatus &os) {
    HRSchemaSerializer::ParsedPairs pairs(tokenizer);

    QString title = pairs.equalPairs.value(TITLE, "");
    if (!pairs.equalPairs.contains(ID)) {
        os.setError(tr("Wizard page %1 does not contain id").arg(title));
        return;
    }
    QString id = pairs.equalPairs.take(ID);
    if (nextIds.keys().contains(id)) {
        os.setError(tr("Several wizard pages have equal ids: %1").arg(id));
        return;
    }

    QString templateId = pairs.equalPairs.value(TEMPLATE, DefaultPageContent::ID);
    QScopedPointer<TemplatedPageContent> content(PageContentFactory::createContent(templateId, os));
    CHECK_OP(os, );
    PageContentParser pcp(pairs, actorMap, vars, os);
    content->accept(&pcp);
    CHECK_OP(os, );

    QScopedPointer<WizardPage> page(new WizardPage(id, title));
    parseNextIds(pairs, page.data(), os);
    CHECK_OP(os, );
    page->setContent(content.take());
    pagesMap[id] = page.take();
}

void HRWizardParser::finilizePagesOrder(U2OpStatus &os) {
    pages = pagesMap.values();
    //QString lastId;
    //QList<QString> ids = nextIds.keys();
    //// Check loops, first page id and last page id
    //foreach (const QString &id, nextIds.keys()) {
    //    if (nextIds[id].isEmpty()) {
    //        if (!lastId.isEmpty()) {
    //            os.setError(tr("Two pages of the wizard are defined as last: %1, %2").arg(lastId).arg(id));
    //            return;
    //        }
    //        lastId = id;
    //    } else {
    //        if (!nextIds.keys().contains(nextIds[id])) {
    //            os.setError(tr("Unknown next page id: %1").arg(nextIds[id]));
    //            return;
    //        }
    //        if (!ids.contains(nextIds[id])) {
    //            os.setError(tr("Two pages of the wizard have equal next page ids: %1").arg(nextIds[id]));
    //            return;
    //        }
    //        ids.removeOne(nextIds[id]);
    //    }
    //}
    //if (ids.isEmpty()) {
    //    os.setError(tr("Some pages of the widget are looped"));
    //    return;
    //} else if (ids.size() > 1) {
    //    os.setError(tr("Several pages of the wizard are defined as first: %1, %2, ...").arg(ids[0]).arg(ids[1]));
    //    return;
    //}

    //// Create pages list saving order
    //QString currentId = ids.first();
    //while ("" != currentId) {
    //    QString nextId = nextIds[currentId];

    //    WizardPage *page = pagesMap[currentId];
    //    if (!nextId.isEmpty()) {
    //        page->setNext(pagesMap[nextId]->getId());
    //    }
    //    pages << page;
    //    currentId = nextId;
    //}
}

void HRWizardParser::parseNextIds(HRSchemaSerializer::ParsedPairs &pairs, WizardPage *page, U2OpStatus &os) {
    if (pairs.equalPairs.contains(NEXT)) {
        QString nextId = pairs.equalPairs.value(NEXT);
        if (page->getId() == nextId) {
            os.setError(tr("Page's id and next id are equal: %1").arg(nextId));
            return;
        }
        if (pairs.blockPairs.contains(NEXT)) {
            os.setError(HRWizardParser::tr("Double definition of next id in the page with id: %1").arg(page->getId()));
            return;
        }
        return;
    }
    if (pairs.blockPairs.contains(NEXT)) {
        HRSchemaSerializer::ParsedPairs predPairs(pairs.blockPairs[NEXT]);
        foreach (const QString &id, predPairs.equalPairs.keys()) {
            Predicate p = Predicate::fromString(predPairs.equalPairs[id], os);
            CHECK_OP(os, );
            page->setNext(id, p, os);
            CHECK_OP(os, );
        }
    }
}

/************************************************************************/
/* WizardWidgetParser */
/************************************************************************/
WizardWidgetParser::WizardWidgetParser(const QString &_data,
    const QMap<QString, Actor*> &_actorMap,
    QMap<QString, Variable> &_vars,
    U2OpStatus &_os)
: data(_data), actorMap(_actorMap), vars(_vars), os(_os)
{

}

void WizardWidgetParser::visit(AttributeWidget *aw) {
    pairs = HRSchemaSerializer::ParsedPairs(data, 0);
    validateAttributePairs();
    CHECK_OP(os, );
    QVariantMap hints;
    foreach (const QString &id, pairs.equalPairs.keys()) {
        hints[id] = pairs.equalPairs[id];
    }
    aw->setWigdetHints(hints);
}

void WizardWidgetParser::visit(WidgetsArea *wa) {
    pairs = HRSchemaSerializer::ParsedPairs(data, 0);

    getTitle(wa);
    getLabelSize(wa);

    foreach (const StringPair &pair, pairs.blockPairsList) {
        WizardWidgetParser wParser(pair.second, actorMap, vars, os);
        QScopedPointer<WizardWidget> w(createWidget(pair.first));
        CHECK_OP(os, );
        w->accept(&wParser);
        CHECK_OP(os, );
        wa->addWidget(w.take());
    }
}

void WizardWidgetParser::visit(LogoWidget *lw) {
    pairs = HRSchemaSerializer::ParsedPairs(data, 0);
    if (pairs.equalPairs.contains(HRWizardParser::LOGO_PATH)) {
        lw->setLogoPath(pairs.equalPairs.value(HRWizardParser::LOGO_PATH));
    }
}

void WizardWidgetParser::visit(GroupWidget *gw) {
    visit((WidgetsArea*)gw);
    CHECK_OP(os, );

    QString typeStr = pairs.equalPairs.value(HRWizardParser::TYPE, HRWizardParser::DEFAULT);
    if (HRWizardParser::DEFAULT == typeStr) {
        gw->setType(GroupWidget::DEFAULT);
    } else if (HRWizardParser::HIDEABLE == typeStr) {
        gw->setType(GroupWidget::HIDEABLE);
    }
}

void WizardWidgetParser::visit(ElementSelectorWidget *esw) {
    pairs = HRSchemaSerializer::ParsedPairs(data, 0);
    if (!pairs.equalPairs.contains(HRWizardParser::ELEMENT_ID)) {
        os.setError(HRWizardParser::tr("Element id is undefined in the element selector"));
        return;
    }
    QString actorId = pairs.equalPairs[HRWizardParser::ELEMENT_ID];
    if (!actorMap.contains(actorId)) {
        os.setError(HRWizardParser::tr("Undefined actor id: %1").arg(actorId));
        return;
    }
    esw->setActorId(actorId);
    if (pairs.equalPairs.contains(AttributeWidgetHints::LABEL)) {
        esw->setLabel(pairs.equalPairs[AttributeWidgetHints::LABEL]);
    }
    ActorPrototype *srcProto = actorMap[actorId]->getProto();
    foreach (const StringPair &pair, pairs.blockPairsList) {
        if (pair.first != HRWizardParser::VALUE) {
            os.setError(HRWizardParser::tr("Unknown block name in element selector definition: %1").arg(pair.first));
            return;
        }
        SelectorValue value = parseSelectorValue(srcProto, pair.second);
        CHECK_OP(os, );
        esw->addValue(value);
    }
    addVariable(Variable(actorId));
    CHECK_OP(os, );
}

SelectorValue WizardWidgetParser::parseSelectorValue(ActorPrototype *srcProto, const QString &valueDef) {
    HRSchemaSerializer::ParsedPairs pairs(valueDef, 0);
    if (!pairs.equalPairs.contains(HRWizardParser::ID)) {
        os.setError(HRWizardParser::tr("Id is undefined in some selector value definition"));
        return SelectorValue("", "");
    }
    QString id = pairs.equalPairs[HRWizardParser::ID];
    if (!pairs.equalPairs.contains(HRWizardParser::PROTOTYPE)) {
        os.setError(HRWizardParser::tr("Prototype is undefined in the selector value definition: %1").arg(id));
        return SelectorValue("", "");
    }
    QString protoId = pairs.equalPairs[HRWizardParser::PROTOTYPE];
    SelectorValue result(id, protoId);
    result.setName(pairs.equalPairs[HRWizardParser::NAME]);
    if (srcProto->getId() == protoId) {
        if (!pairs.blockPairsList.isEmpty()) {
            os.setError(HRWizardParser::tr("The same prototype could not be mapped: %1").arg(protoId));
        }
        return result;
    }
    foreach (const StringPair &pair, pairs.blockPairsList) {
        if (pair.first != HRWizardParser::PORT_MAPPING) {
            os.setError(HRWizardParser::tr("Unknown block name in selector value definition: %1").arg(pair.first));
            return result;
        }
        PortMapping mapping = parsePortMapping(pair.second);
        CHECK_OP(os, result);
        result.addPortMapping(mapping);
    }
    return result;
}

PortMapping WizardWidgetParser::parsePortMapping(const QString &mappingDef) {
    HRSchemaSerializer::ParsedPairs pairs(mappingDef, 0);
    if (!pairs.equalPairs.contains(HRWizardParser::SRC_PORT)) {
        os.setError(HRWizardParser::tr("Undefined source port id for some port mapping"));
        return PortMapping("", "");
    }
    if (!pairs.equalPairs.contains(HRWizardParser::DST_PORT)) {
        os.setError(HRWizardParser::tr("Undefined destination port id for some port mapping"));
        return PortMapping("", "");
    }
    QString srcPortId = pairs.equalPairs[HRWizardParser::SRC_PORT];
    QString dstPortId = pairs.equalPairs[HRWizardParser::DST_PORT];
    PortMapping result(srcPortId, dstPortId);
    foreach (const StringPair &pair, pairs.blockPairsList) {
        if (pair.first != HRWizardParser::SLOTS_MAPPRING) {
            os.setError(HRWizardParser::tr("Unknown block name in port mapping definition: %1").arg(pair.first));
            return result;
        }
        parseSlotsMapping(result, pair.second);
        CHECK_OP(os, result);
    }
    return result;
}

void WizardWidgetParser::parseSlotsMapping(PortMapping &pm, const QString &mappingDef) {
    HRSchemaSerializer::ParsedPairs pairs(mappingDef, 0);
    foreach (const StringPair &pair, pairs.equalPairsList) {
        QString srcSlotId = pair.first;
        QString dstSlotId = pair.second;
        pm.addSlotMapping(SlotMapping(srcSlotId, dstSlotId));
    }
}

void WizardWidgetParser::validateAttributePairs() {
    if (pairs.equalPairs.contains(AttributeWidgetHints::TYPE)) {
        QString typeStr = pairs.equalPairs[AttributeWidgetHints::TYPE];
        if ((AttributeWidgetHints::DEFAULT != typeStr) &&
            (AttributeWidgetHints::DATASETS != typeStr)) {
            os.setError(HRWizardParser::tr("Unknown widget type: %1").arg(typeStr));
            return;
        }
    }
}

void WizardWidgetParser::getLabelSize(WidgetsArea *wa) {
    if (pairs.equalPairs.contains(HRWizardParser::LABEL_SIZE)) {
        QString &sizeStr = pairs.equalPairs[HRWizardParser::LABEL_SIZE];
        bool ok = true;
        int size = sizeStr.toInt(&ok);
        if (!ok) {
            os.setError(QObject::tr("Wrong label size value: %1").arg(sizeStr));
            return;
        }
        wa->setLabelSize(size);
    }
}

void WizardWidgetParser::getTitle(WidgetsArea *wa) {
    if (pairs.equalPairs.contains(HRWizardParser::TITLE)) {
        wa->setTitle(pairs.equalPairs[HRWizardParser::TITLE]);
    }
}

WizardWidget * WizardWidgetParser::createWidget(const QString &id) {
    if (LogoWidget::ID == id) {
        return new LogoWidget();
    } else if (GroupWidget::ID == id) {
        return new GroupWidget();
    } else if (ElementSelectorWidget::ID == id) {
        return new ElementSelectorWidget();
    } else {
        QStringList vals = id.split(HRSchemaSerializer::DOT, QString::SkipEmptyParts);
        if (2 != vals.size()) {
            os.setError(HRWizardParser::tr("Unknown widget name: %1").arg(id));
            return NULL;
        }
        return new AttributeWidget(vals[0], vals[1]);
    }
}

void WizardWidgetParser::addVariable(const Variable &v) {
    if (vars.contains(v.getName())) {
        os.setError(QObject::tr("The variable is already defined: %1").arg(v.getName()));
        return;
    }
    vars[v.getName()] = v;
}

/************************************************************************/
/* PageContentParser */
/************************************************************************/
PageContentParser::PageContentParser(HRSchemaSerializer::ParsedPairs &_pairs,
    const QMap<QString, Actor*> &_actorMap,
    QMap<QString, Variable> &_vars,
    U2OpStatus &_os)
: pairs(_pairs), actorMap(_actorMap), vars(_vars), os(_os)
{

}

void PageContentParser::visit(DefaultPageContent *content) {
    foreach (const StringPair &pair, pairs.blockPairsList) {
        WizardWidgetParser wParser(pair.second, actorMap, vars, os);
        if (LogoWidget::ID == pair.first) {
            content->getLogoArea()->accept(&wParser);
        } else if (DefaultPageContent::PARAMETERS == pair.first) {
            content->getParamsArea()->accept(&wParser);
        }
        CHECK_OP(os, );
    }
}

/************************************************************************/
/* HRWizardSerializer */
/************************************************************************/
QString HRWizardSerializer::serialize(Wizard *wizard, int depth) {
    QString wizardData;

    if (Wizard::DEFAULT_NAME != wizard->getName()) {
        wizardData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::NAME,
            wizard->getName(), depth + 1);
    }

    foreach (WizardPage *page, wizard->getPages()) {
        wizardData += serializePage(page, depth + 1);
    }

    return HRSchemaSerializer::makeBlock(HRWizardParser::WIZARD,
        HRSchemaSerializer::NO_NAME, wizardData, depth);
}

QString HRWizardSerializer::serializePage(WizardPage *page, int depth) {
    QString pageData;

    pageData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::ID, page->getId(), depth + 1);
    pageData += HRWizardSerializer::serializeNextId(page, depth + 1);
    if (!page->getTitle().isEmpty()) {
        pageData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::TITLE,
            page->getTitle(), depth + 1);
    }
    if (DefaultPageContent::ID != page->getContent()->getTemplateId()) {
        pageData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::TEMPLATE,
            page->getContent()->getTemplateId(), depth + 1);
    }

    PageContentSerializer cs(depth + 1);
    page->getContent()->accept(&cs);
    pageData += cs.getResult();

    return HRSchemaSerializer::makeBlock(HRWizardParser::PAGE,
        HRSchemaSerializer::NO_NAME, pageData, depth);
}

QString HRWizardSerializer::serializeNextId(WizardPage *page, int depth) {
    if (page->nextIdMap().isEmpty()) {
        if (page->plainNextId().isEmpty()) {
            return "";
        }
        return HRSchemaSerializer::makeEqualsPair(HRWizardParser::NEXT, page->plainNextId(), depth);
    }
    QString nextData;
    foreach (const Predicate &p, page->nextIdMap().keys()) {
        QString id = page->nextIdMap()[p];
        nextData += HRSchemaSerializer::makeEqualsPair(id, p.toString(), depth + 1);
    }
    return HRSchemaSerializer::makeBlock(HRWizardParser::NEXT,
        HRSchemaSerializer::NO_NAME, nextData, depth);
}

/************************************************************************/
/* WizardWidgetSerializer */
/************************************************************************/
WizardWidgetSerializer::WizardWidgetSerializer(int _depth)
: depth(_depth)
{

}

void WizardWidgetSerializer::visit(AttributeWidget *aw) {
    QString wData;

    foreach (const QString &id, aw->getWigdetHints().keys()) {
        bool writeData = true;
        if (AttributeWidgetHints::TYPE == id) {
            writeData = (AttributeWidgetHints::DEFAULT != aw->getProperty(id));
        }
        if (writeData) {
            wData += HRSchemaSerializer::makeEqualsPair(id, aw->getProperty(id), depth + 1);
        }
    }

    QString name = aw->getActorId() + HRSchemaSerializer::DOT + aw->getAttributeId();
    result = HRSchemaSerializer::makeBlock(name, HRSchemaSerializer::NO_NAME,
        wData, depth);
}

void WizardWidgetSerializer::visit(WidgetsArea *wa) {
    QString wData;
    // write title
    if (!wa->getTitle().isEmpty()) {
        wData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::TITLE,
            wa->getTitle(), depth + 1);
    }
    // write label size
    if (wa->hasLabelSize()) {
        wData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::LABEL_SIZE,
            QString::number(wa->getLabelSize()), depth + 1);
    }
    // write additional info
    wData += addInfo;
    // write widgets
    foreach (WizardWidget *w, wa->getWidgets()) {
        WizardWidgetSerializer ws(depth + 1);
        w->accept(&ws);
        wData += ws.getResult();
    }

    result = HRSchemaSerializer::makeBlock(wa->getName(),
        HRSchemaSerializer::NO_NAME, wData, depth);
}

void WizardWidgetSerializer::visit(GroupWidget *gw) {
    if (GroupWidget::HIDEABLE == gw->getType()) {
        addInfo = HRSchemaSerializer::makeEqualsPair(HRWizardParser::TYPE,
            HRWizardParser::HIDEABLE, depth + 1);
    }
    visit((WidgetsArea*)gw);
}

void WizardWidgetSerializer::visit(LogoWidget *lw) {
    QString wData;
    if (!lw->isDefault()) {
        wData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::LOGO_PATH,
            lw->getLogoPath(), depth + 1);
    }
    result = HRSchemaSerializer::makeBlock(LogoWidget::ID,
        HRSchemaSerializer::NO_NAME, wData, depth);
}

void WizardWidgetSerializer::visit(ElementSelectorWidget *esw) {
    QString wData;
    wData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::ELEMENT_ID,
        esw->getActorId(), depth + 1);
    if (!esw->getLabel().isEmpty()) {
        wData += HRSchemaSerializer::makeEqualsPair(AttributeWidgetHints::LABEL,
            esw->getLabel(), depth + 1);
    }
    foreach (const SelectorValue &value, esw->getValues()) {
        wData += serializeSelectorValue(value, depth + 1);
    }
    result = HRSchemaSerializer::makeBlock(ElementSelectorWidget::ID,
        HRSchemaSerializer::NO_NAME, wData, depth);
}

QString WizardWidgetSerializer::serializeSlotsMapping(const QList<SlotMapping> &mappings, int depth) const {
    QString smData;
    foreach (const SlotMapping &mapping, mappings) {
        smData += HRSchemaSerializer::makeEqualsPair(mapping.getSrcId(),
            mapping.getDstId(), depth + 1);
    }
    return HRSchemaSerializer::makeBlock(HRWizardParser::SLOTS_MAPPRING,
        HRSchemaSerializer::NO_NAME, smData, depth);
}

QString WizardWidgetSerializer::serializePortMapping(const PortMapping &mapping, int depth) const {
    QString pmData;
    pmData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::SRC_PORT,
        mapping.getSrcId(), depth + 1);
    pmData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::DST_PORT,
        mapping.getDstId(), depth + 1);
    pmData += serializeSlotsMapping(mapping.getMappings(), depth + 1);
    return HRSchemaSerializer::makeBlock(HRWizardParser::PORT_MAPPING,
        HRSchemaSerializer::NO_NAME, pmData, depth);
}

QString WizardWidgetSerializer::serializeSelectorValue(const SelectorValue &value, int depth) const {
    QString vData;
    vData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::ID,
        value.getValue(), depth + 1);
    vData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::PROTOTYPE,
        value.getProtoId(), depth + 1);
    if (!value.getName().isEmpty()) {
        vData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::NAME,
            value.getName(), depth + 1);
    }
    foreach (const PortMapping &mapping, value.getMappings()) {
        vData += serializePortMapping(mapping, depth + 1);
    }
    return HRSchemaSerializer::makeBlock(HRWizardParser::VALUE,
        HRSchemaSerializer::NO_NAME, vData, depth);
}

const QString & WizardWidgetSerializer::getResult() {
    return result;
}

/************************************************************************/
/* PageContentSerializer */
/************************************************************************/
PageContentSerializer::PageContentSerializer(int _depth)
: depth(_depth)
{

}

void PageContentSerializer::visit(DefaultPageContent *content) {
    WizardWidgetSerializer pws(depth);
    content->getParamsArea()->accept(&pws);
    result += pws.getResult();

    if (!content->getLogoArea()->isDefault()) {
        WizardWidgetSerializer lws(depth);
        content->getLogoArea()->accept(&lws);
        result += lws.getResult();
    }
}

const QString & PageContentSerializer::getResult() const {
    return result;
}

} // U2
