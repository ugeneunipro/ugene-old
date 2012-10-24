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

#include "WizardPage.h"

namespace U2 {

/**********************************
* WizardPage
*********************************/
WizardPage::WizardPage(const QString &_id, const QString &_title)
: id(_id), next(NULL), title(_title)
{
}

WizardPage::~WizardPage() {
    delete content;
}

void WizardPage::setNext(WizardPage *value) {
    next = value;
}

WizardPage * WizardPage::getNext() const {
    return next;
}

const QString & WizardPage::getId() const {
    return id;
}

const QString & WizardPage::getTitle() const {
    return title;
}

void WizardPage::setContent(TemplatedPageContent *value) {
    content = value;
}

TemplatedPageContent * WizardPage::getContent() {
    return content;
}

/**********************************
* TemplatedPage
*********************************/
TemplatedPageContent::TemplatedPageContent(const QString &_templateId)
: templateId(_templateId)
{

}

TemplatedPageContent::~TemplatedPageContent() {

}

const QString & TemplatedPageContent::getTemplateId() const {
    return templateId;
}

TemplatedPageContent * PageContentFactory::createContent(const QString &id, U2OpStatus &os) {
    if (DefaultPageContent::ID == id) {
        return new DefaultPageContent();
    }
    os.setError(QObject::tr("Unknown page template id: %1").arg(id));
    return NULL;
}

/**********************************
* DefaultTemplatePage
*********************************/
const QString DefaultPageContent::ID("default");
const QString DefaultPageContent::PARAMETERS("parameters-area");

DefaultPageContent::DefaultPageContent()
: TemplatedPageContent(ID)
{
    logoArea = new LogoWidget();
    paramsArea = new WidgetsArea(PARAMETERS);
}

DefaultPageContent::~DefaultPageContent() {
    delete logoArea;
    delete paramsArea;
}

void DefaultPageContent::accept(TemplatedPageVisitor *visitor) {
    visitor->visit(this);
}

void DefaultPageContent::addParamWidget(WizardWidget *widget) {
    paramsArea->addWidget(widget);
}

void DefaultPageContent::setLogoPath(const QString &path) {
    logoArea->setLogoPath(path);
}

LogoWidget * DefaultPageContent::getLogoArea() {
    return logoArea;
}

WidgetsArea * DefaultPageContent::getParamsArea() {
    return paramsArea;
}

} // U2
