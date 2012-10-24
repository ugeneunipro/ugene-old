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

#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>

#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include <U2Lang/WizardPage.h>

#include "PropertyWizardController.h"

#include "WizardController.h"

namespace U2 {

WizardController::WizardController(Wizard *w)
: wizard(w)
{

}

WizardController::~WizardController() {

}

QWizard * WizardController::createGui() {
    controllers.clear();
    QWizard *result = new QWizard((QWidget*)AppContext::getMainWindow()->getQMainWindow());
    foreach (WizardPage *page, wizard->getPages()) {
        result->addPage(createPage(page));
    }
    result->setWizardStyle(QWizard::ClassicStyle);
    result->setModal(true);
    result->setAutoFillBackground(true);
    result->setWindowTitle(wizard->getName());
    return result;
}

void WizardController::assignParameters() {
    foreach (PropertyWizardController *pwc, controllers) {
        pwc->assignPropertyValue();
    }
}

QWizardPage * WizardController::createPage(WizardPage *page) {
    QWizardPage *result = new QWizardPage();
    result->setTitle(page->getTitle());
    result->setFinalPage(NULL == page->getNext());

    PageContentCreator pcc;
    page->getContent()->accept(&pcc);
    result->setLayout(pcc.getResult());
    controllers << pcc.getControllers();

    result->adjustSize();
    return result;
}

/************************************************************************/
/* WidgetCreator */
/************************************************************************/
WidgetCreator::WidgetCreator()
: labelSize(0), result(NULL)
{

}

WidgetCreator::WidgetCreator(int _labelSize)
: labelSize(_labelSize), result(NULL)
{

}

void WidgetCreator::visit(AttributeWidget *aw) {
    QString type = aw->getProperty(AttributeWidgetHints::TYPE);
    PropertyWizardController *controller = NULL;

    if (AttributeWidgetHints::DEFAULT == type) {
        controller = new DefaultPropertyController(aw, labelSize);
    } else if (AttributeWidgetHints::DATASETS == type) {
        controller = new InUrlDatasetsController(aw);
    } else {
        SAFE_POINT(false, QString("Unknown widget type: %1").arg(type), );
    }

    controllers << controller;
    U2OpStatusImpl os;
    result = controller->createGUI(os);
}

void WidgetCreator::visit(WidgetsArea *wa) {
    result = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    result->setLayout(layout);
    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    foreach (WizardWidget *w, wa->getWidgets()) {
        WidgetCreator wc(wa->getLabelSize());
        w->accept(&wc);
        if (NULL != wc.getResult()) {
            wc.getResult()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            layout->addWidget(wc.getResult());
            controllers << wc.getControllers();
        }
    }
    layout->addSpacerItem(spacer);
}

void WidgetCreator::visit(GroupWidget *gw) {
    visit((WidgetsArea*)gw);
    bool collapsible = (gw->getType() == GroupWidget::HIDEABLE);
    GroupBox *gb = new GroupBox(collapsible, gw->getTitle());
    setGroupBoxLayout(gb);
}

void WidgetCreator::visit(LogoWidget *lw) {
    result = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    result->setLayout(layout);

    QLabel *label = new QLabel(result);
    if (lw->isDefault()) {
        label->setPixmap(QPixmap(QString(":U2Designer/images/logo.png")));
    } else {
        label->setPixmap(QPixmap(lw->getLogoPath()));
    }
    layout->addWidget(label);
}

QWidget * WidgetCreator::getResult() {
    return result;
}

QList<PropertyWizardController*> & WidgetCreator::getControllers() {
    return controllers;
}

void WidgetCreator::setGroupBoxLayout(GroupBox *gb) {
    gb->setLayout(result->layout());
    result->setLayout(NULL);
    delete result;
    result = gb;
}

/************************************************************************/
/* PageContentCreator */
/************************************************************************/
PageContentCreator::PageContentCreator()
: result(NULL)
{

}

void PageContentCreator::visit(DefaultPageContent *content) {
    QHBoxLayout *layout = new QHBoxLayout();
    result = layout;
    { // create logo
        WidgetCreator logoWC;
        content->getLogoArea()->accept(&logoWC);
        if (NULL != logoWC.getResult()) {
            layout->addWidget(logoWC.getResult());
            controllers << logoWC.getControllers();
        }
    }
    { // create parameters
        WidgetCreator paramsWC;
        content->getParamsArea()->accept(&paramsWC);
        if (NULL != paramsWC.getResult()) {
            paramsWC.getResult()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            layout->addWidget(paramsWC.getResult());
            controllers << paramsWC.getControllers();
        }
    }
}

QLayout * PageContentCreator::getResult() {
    return result;
}

QList<PropertyWizardController*> & PageContentCreator::getControllers() {
    return controllers;
}

/************************************************************************/
/* GroupBox */
/************************************************************************/
const int GroupBox::MARGIN = 5;

GroupBox::GroupBox(bool collapsible, const QString &title)
: QGroupBox(title), hLayout(NULL), tip(NULL), showHideButton(NULL)
{
    ui = new QWidget(this);
    ui->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    QVBoxLayout *layout = new QVBoxLayout();
    QGroupBox::setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (collapsible) {
        hLayout = new QHBoxLayout();
        tip = new QLabel(this);
        hLayout->addWidget(tip);
        showHideButton = new QToolButton(this);
        showHideButton->setFixedSize(19, 19);
        hLayout->addWidget(showHideButton);
        layout->addLayout(hLayout);
    }

    layout->addWidget(ui);

    if (collapsible) {
        sl_collapse();
        connect(showHideButton, SIGNAL(clicked()), this, SLOT(sl_onCheck()));
    }
}

void GroupBox::sl_collapse() {
    ui->hide();
    changeView("+", tr("Show"), MARGIN);
}

void GroupBox::sl_expand() {
    ui->show();
    changeView("-", tr("Hide"), 0);
}

void GroupBox::setLayout(QLayout *l) {
    ui->setLayout(l);
    CHECK(NULL != hLayout, );
    QMargins m = l->contentsMargins();
    m.setBottom(hLayout->contentsMargins().bottom());
    m.setTop(hLayout->contentsMargins().top());
    hLayout->setContentsMargins(m);
}

void GroupBox::sl_onCheck() {
    if (ui->isVisible()) {
        sl_collapse();
    } else {
        sl_expand();
    }
}

void GroupBox::changeView(const QString &buttonText, const QString &showHide, int bottomMargin) {
    CHECK(NULL != showHideButton, );
    showHideButton->setText(buttonText);

    CHECK(NULL != tip, );
    QString parametersStr = tr("additional");
    if (!title().isEmpty()) {
        parametersStr = title().toLower();
    }
    tip->setText(showHide + " " + parametersStr + tr(" parameters"));
    showHideButton->setToolTip(tip->text());

    CHECK(NULL != hLayout, );
    QMargins m = hLayout->contentsMargins();
    m.setBottom(bottomMargin);
    if (title().isEmpty()) {
        m.setTop(MARGIN);
    }
    hLayout->setContentsMargins(m);
}

} // U2
