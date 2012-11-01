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

#ifndef _U2_HRWIZARDSERIALIZER_H_
#define _U2_HRWIZARDSERIALIZER_H_

#include <U2Core/U2OpStatus.h>

#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/Wizard.h>
#include <U2Lang/WizardPage.h>
#include <U2Lang/WizardWidget.h>

namespace U2 {

class U2LANG_EXPORT HRWizardParser : public QObject {
    Q_OBJECT
public:
    HRWizardParser(HRSchemaSerializer::Tokenizer &tokenizer, const QMap<QString, Actor*> &actorMap);
    virtual ~HRWizardParser();

    Wizard * parseWizard(U2OpStatus &os);

private:
    HRSchemaSerializer::Tokenizer &tokenizer;
    const QMap<QString, Actor*> &actorMap;
    QString wizardName;
    QList<WizardPage*> pages; // this list keeps the order of pages too

    QMap<QString, QString> nextIds; // id <-> nextId
    QMap<QString, WizardPage*> pagesMap; // id <-> page

private:
    void parsePage(U2OpStatus &os);
    void finilizePagesOrder(U2OpStatus &os);
    Wizard * takeResult();

public:
    static const QString WIZARD;
    static const QString NAME;
    static const QString PAGE;
    static const QString ID;
    static const QString NEXT;
    static const QString TITLE;
    static const QString TEMPLATE;
    static const QString TYPE;
    static const QString LOGO_PATH;
    static const QString DEFAULT;
    static const QString HIDEABLE;
    static const QString LABEL_SIZE;
};

class U2LANG_EXPORT HRWizardSerializer {
public:
    QString serialize(Wizard *wizard, int depth);

private:
    QString serializePage(WizardPage *page, int depth);
};

/************************************************************************/
/* Parsing utilities */
/************************************************************************/
class WizardWidgetParser : public WizardWidgetVisitor {
public:
    WizardWidgetParser(const QString &data, const QMap<QString, Actor*> &actorMap, U2OpStatus &os);

    virtual void visit(AttributeWidget *aw);
    virtual void visit(WidgetsArea *w);
    virtual void visit(LogoWidget *lw);
    virtual void visit(GroupWidget *gw);

private:
    QString data;
    const QMap<QString, Actor*> &actorMap;
    U2OpStatus &os;

    HRSchemaSerializer::ParsedPairs pairs;

private:
    void validateAttributePairs();
    void getLabelSize(WidgetsArea *wa);
    void getTitle(WidgetsArea *wa);
};

class PageContentParser : public TemplatedPageVisitor {
public:
    PageContentParser(HRSchemaSerializer::ParsedPairs &pairs,
        const QMap<QString, Actor*> &actorMap, U2OpStatus &os);

    virtual void visit(DefaultPageContent *content);

private:
    HRSchemaSerializer::ParsedPairs &pairs;
    const QMap<QString, Actor*> &actorMap;
    U2OpStatus &os;
};

/************************************************************************/
/* Serializing utilities */
/************************************************************************/
class WizardWidgetSerializer : public WizardWidgetVisitor {
public:
    WizardWidgetSerializer(int depth);

    virtual void visit(AttributeWidget *aw);
    virtual void visit(WidgetsArea *wa);
    virtual void visit(GroupWidget *gw);
    virtual void visit(LogoWidget *lw);

    const QString & getResult();

private:
    int depth;
    QString result;
    QString addInfo;
};

class PageContentSerializer : public TemplatedPageVisitor {
public:
    PageContentSerializer(int depth);

    virtual void visit(DefaultPageContent *content);

    const QString & getResult() const;

private:
    int depth;
    QString result;
};

} // U2

#endif // _U2_HRWIZARDSERIALIZER_H_
