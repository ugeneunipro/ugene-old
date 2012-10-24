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
    return new Wizard(wizardName, retPages);
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
    nextIds[id] = pairs.equalPairs.value(NEXT, "");
    if (id == nextIds[id]) {
        os.setError(tr("Page's id and next id are equal: %1").arg(id));
        return;
    }

    QString templateId = pairs.equalPairs.value(TEMPLATE, DefaultPageContent::ID);
    QScopedPointer<TemplatedPageContent> content(PageContentFactory::createContent(templateId, os));
    CHECK_OP(os, );
    PageContentParser pcp(pairs, actorMap, os);
    content->accept(&pcp);
    CHECK_OP(os, );

    WizardPage *page = new WizardPage(id, title);
    page->setContent(content.take());
    pagesMap[id] = page;
}

void HRWizardParser::finilizePagesOrder(U2OpStatus &os) {
    QString lastId;
    QList<QString> ids = nextIds.keys();
    // Check loops, first page id and last page id
    foreach (const QString &id, nextIds.keys()) {
        if (nextIds[id].isEmpty()) {
            if (!lastId.isEmpty()) {
                os.setError(tr("Two pages of the wizard are defined as last: %1, %2").arg(lastId).arg(id));
                return;
            }
            lastId = id;
        } else {
            if (!nextIds.keys().contains(nextIds[id])) {
                os.setError(tr("Unknown next page id: %1").arg(nextIds[id]));
                return;
            }
            if (!ids.contains(nextIds[id])) {
                os.setError(tr("Two pages of the wizard have equal next page ids: %1").arg(nextIds[id]));
                return;
            }
            ids.removeOne(nextIds[id]);
        }
    }
    if (ids.isEmpty()) {
        os.setError(tr("Some pages of the widget are looped"));
        return;
    } else if (ids.size() > 1) {
        os.setError(tr("Several pages of the wizard are defined as first: %1, %2, ...").arg(ids[0]).arg(ids[1]));
        return;
    }

    // Create pages list saving order
    QString currentId = ids.first();
    while ("" != currentId) {
        QString nextId = nextIds[currentId];

        WizardPage *page = pagesMap[currentId];
        if (!nextId.isEmpty()) {
            page->setNext(pagesMap[nextId]);
        }
        pages << page;
        currentId = nextId;
    }
}

/************************************************************************/
/* WizardWidgetParser */
/************************************************************************/
WizardWidgetParser::WizardWidgetParser(const QString &_data, const QMap<QString, Actor*> &_actorMap, U2OpStatus &_os)
: data(_data), actorMap(_actorMap), os(_os)
{

}

void WizardWidgetParser::visit(AttributeWidget *aw) {
    HRSchemaSerializer::Tokenizer tokenizer;
    tokenizer.tokenize(data, 0);
    pairs = HRSchemaSerializer::ParsedPairs(tokenizer);
    validateAttributePairs();
    CHECK_OP(os, );
    QVariantMap hints;
    foreach (const QString &id, pairs.equalPairs.keys()) {
        hints[id] = pairs.equalPairs[id];
    }
    aw->setWigdetHints(hints);
}

void WizardWidgetParser::visit(WidgetsArea *w) {
    HRSchemaSerializer::Tokenizer tokenizer;
    tokenizer.tokenize(data, 0);
    pairs = HRSchemaSerializer::ParsedPairs(tokenizer);

    getTitle(w);
    getLabelSize(w);

    foreach (const StringPair &pair, pairs.blockPairsList) {
        WizardWidgetParser wParser(pair.second, actorMap, os);
        if (LogoWidget::ID == pair.first) {
            QScopedPointer<LogoWidget> logo(new LogoWidget());
            logo->accept(&wParser);
            CHECK_OP(os, );
            w->addWidget(logo.take());
        } else if (GroupWidget::ID == pair.first) {
            QScopedPointer<GroupWidget> group(new GroupWidget());
            group->accept(&wParser);
            CHECK_OP(os, );
            w->addWidget(group.take());
        } else {
            QStringList vals = pair.first.split(HRSchemaSerializer::DOT, QString::SkipEmptyParts);
            if (2 != vals.size()) {
                os.setError(HRWizardParser::tr("Unknown widget name: %1").arg(pair.first));
                return;
            }
            QString actorId = vals[0];
            QString attrId = vals[1];
            if (!actorMap.contains(actorId)) {
                os.setError(HRWizardParser::tr("Unknown actor id: %1").arg(actorId));
                return;
            }
            if (!actorMap[actorId]->hasParameter(attrId)) {
                os.setError(HRWizardParser::tr("Actor '%1' does not have this parameter: %2").arg(actorId).arg(attrId));
                return;
            }
            QScopedPointer<AttributeWidget> attr(new AttributeWidget(actorMap[actorId], actorMap[actorId]->getParameter(attrId)));
            attr->accept(&wParser);
            CHECK_OP(os, );
            w->addWidget(attr.take());
        }
    }
}

void WizardWidgetParser::visit(LogoWidget *lw) {
    HRSchemaSerializer::Tokenizer tokenizer;
    tokenizer.tokenize(data, 0);
    pairs = HRSchemaSerializer::ParsedPairs(tokenizer);
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

/************************************************************************/
/* PageContentParser */
/************************************************************************/
PageContentParser::PageContentParser(HRSchemaSerializer::ParsedPairs &_pairs,
const QMap<QString, Actor*> &_actorMap, U2OpStatus &_os)
: pairs(_pairs), actorMap(_actorMap), os(_os)
{

}

void PageContentParser::visit(DefaultPageContent *content) {
    foreach (const StringPair &pair, pairs.blockPairsList) {
        WizardWidgetParser wParser(pair.second, actorMap, os);
        if (LogoWidget::ID == pair.first) {
            content->getLogoArea()->accept(&wParser);
        } else if (DefaultPageContent::PARAMETERS == pair.first) {
            content->getParamsArea()->accept(&wParser);
        }
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
    if (NULL != page->getNext()) {
        pageData += HRSchemaSerializer::makeEqualsPair(HRWizardParser::NEXT,
            page->getNext()->getId(), depth + 1);
    }
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
            wData += HRSchemaSerializer::makeEqualsPair(AttributeWidgetHints::TYPE,
                aw->getProperty(id), depth + 1);
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
