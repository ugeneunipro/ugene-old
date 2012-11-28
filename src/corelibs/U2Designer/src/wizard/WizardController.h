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

#ifndef _U2_WIZARDCONTROLLER_H_
#define _U2_WIZARDCONTROLLER_H_

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QWizard>

#include <U2Lang/ElementSelectorWidget.h>
#include <U2Lang/Schema.h>
#include <U2Lang/Wizard.h>
#include <U2Lang/WizardPage.h>
#include <U2Lang/WizardWidget.h>
#include <U2Lang/WizardWidgetVisitor.h>

#include "SelectorActors.h"

namespace U2 {

class WidgetController;
class WizardPageController;

class U2DESIGNER_EXPORT WizardController : public QObject {
    Q_OBJECT
public:
    enum U2DESIGNER_EXPORT ApplyResult {
        OK,
        ACTORS_REPLACED,
        BROKEN
    };
public:
    WizardController(Schema *s, Wizard *w);
    virtual ~WizardController();

    QWizard * createGui();
    ApplyResult applyChanges(Metadata &meta);
    const QList<Actor*> & getCurrentActors() const;

    QVariant getWidgetValue(AttributeWidget *widget) const;
    void setWidgetValue(AttributeWidget *widget, const QVariant &value);

    QVariant getSelectorValue(ElementSelectorWidget *widget);
    void setSelectorValue(ElementSelectorWidget *widget, const QVariant &value);

    /** Returns id of the Qt page object by @hrId.
     * @hrId - human-readable id of the page.
     */
    int getQtPageId(const QString &hrId) const;
    const QMap<QString, Variable> & getVariables() const;

    /** Wizard pages are validated in runtime. If some page is not validated
     * then wizard becomes broken
     */
    void setBroken();
    bool isBroken() const;

private:
    bool broken;
    Schema *schema;
    Wizard *wizard;
    QList<WizardPageController*> pageControllers;
    QVariantMap propValues; // protoId.actorId.attrId <-> value
    QList<Actor*> currentActors;
    QMap<QString, int> pageIdMap; // hr-id <-> qt-id
    QMap<QString, Variable> vars;
    QMap<QString, SelectorActors> selectors; // varName <-> actors

private:
    QWizardPage * createPage(WizardPage *page);
    Attribute * getAttribute(AttributeWidget *widget, QString &attrId) const;
    Attribute * getAttributeById(const QString &attrId) const;
    QString getAttributeId(Actor *actor, Attribute *attr) const;
    void registerSelector(ElementSelectorWidget *widget);
    void replaceCurrentActor(const QString &actorId, const QString &selectorValue);
    void assignParameters();
};

/************************************************************************/
/* Utilities */
/************************************************************************/
class GroupBox;
class WidgetCreator : public WizardWidgetVisitor {
public:
    WidgetCreator(WizardController *wc);
    WidgetCreator(WizardController *wc, int labelSize);

    virtual void visit(AttributeWidget *aw);
    virtual void visit(WidgetsArea *wa);
    virtual void visit(GroupWidget *gw);
    virtual void visit(LogoWidget *lw);
    virtual void visit(ElementSelectorWidget *esw);

    QWidget * getResult();
    QList<WidgetController*> & getControllers();
    QBoxLayout * getLayout();

private:
    WizardController *wc;
    int labelSize;
    QWidget *result;
    QList<WidgetController*> controllers;
    QBoxLayout *layout;
    QScrollArea *widgetsArea;

private:
    void setGroupBoxLayout(GroupBox *gb);
    void setupScrollArea(QWidget *scrollContent);
};

class PageContentCreator : public TemplatedPageVisitor {
public:
    PageContentCreator(WizardController *wc);

    virtual void visit(DefaultPageContent *content);

    QLayout * getResult();
    QList<WidgetController*> & getControllers();

private:
    WizardController *wc;
    QLayout *result;
    QList<WidgetController*> controllers;
};

/************************************************************************/
/* GroupBox */
/************************************************************************/
class GroupBox : public QGroupBox {
    Q_OBJECT
public:
    GroupBox(bool collapsible, const QString &title);

    void setLayout(QLayout *l);

    static const int MARGIN;

private slots:
    void sl_collapse();
    void sl_expand();
    void sl_onCheck();

private:
    QWidget *ui;
    QHBoxLayout *hLayout;
    QLabel *tip;
    QToolButton *showHideButton;

private:
    void changeView(const QString &buttonText, const QString &showHide);
};

} // U2

#endif // _U2_WIZARDCONTROLLER_H_
