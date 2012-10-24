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
#include <QToolButton>
#include <QWizard>

#include <U2Lang/Wizard.h>
#include <U2Lang/WizardPage.h>
#include <U2Lang/WizardWidget.h>

namespace U2 {

class PropertyWizardController;

class U2DESIGNER_EXPORT WizardController : public QObject {
    Q_OBJECT
public:
    WizardController(Wizard *w);
    virtual ~WizardController();

    QWizard * createGui();
    void assignParameters();

private:
    Wizard *wizard;
    QList<PropertyWizardController*> controllers;

private:
    QWizardPage * createPage(WizardPage *page);
};

/************************************************************************/
/* Utilities */
/************************************************************************/
class GroupBox;
class WidgetCreator : public WizardWidgetVisitor {
public:
    WidgetCreator();
    WidgetCreator(int labelSize);

    virtual void visit(AttributeWidget *aw);
    virtual void visit(WidgetsArea *wa);
    virtual void visit(GroupWidget *gw);
    virtual void visit(LogoWidget *lw);

    QWidget * getResult();

    QList<PropertyWizardController*> & getControllers();

private:
    int labelSize;
    QWidget *result;
    QList<PropertyWizardController*> controllers;

private:
    void setGroupBoxLayout(GroupBox *gb);
};

class PageContentCreator : public TemplatedPageVisitor {
public:
    PageContentCreator();

    virtual void visit(DefaultPageContent *content);

    QLayout * getResult();
    QList<PropertyWizardController*> & getControllers();

private:
    QLayout *result;
    QList<PropertyWizardController*> controllers;
};

/************************************************************************/
/* GroupBox */
/************************************************************************/
class GroupBox : public QGroupBox {
    Q_OBJECT
public:
    GroupBox(bool collapsible, const QString &title);

    void setLayout(QLayout *l);

private slots:
    void sl_collapse();
    void sl_expand();
    void sl_onCheck();

private:
    QWidget *ui;
    QHBoxLayout *hLayout;
    QLabel *tip;
    QToolButton *showHideButton;

    static const int MARGIN;

private:
    void changeView(const QString &buttonText, const QString &showHide, int bottomMargin);
};

} // U2

#endif // _U2_WIZARDCONTROLLER_H_
