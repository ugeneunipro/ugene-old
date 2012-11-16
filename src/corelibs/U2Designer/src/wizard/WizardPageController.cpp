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

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "WidgetController.h"
#include "WDWizardPage.h"
#include "WizardController.h"

#include "WizardPageController.h"

namespace U2 {

WizardPageController::WizardPageController(WizardController *_wc, WizardPage *_page)
: wc(_wc), page(_page)
{

}

WizardPageController::~WizardPageController() {

}

void WizardPageController::applyLayout(WDWizardPage *wPage) {
    qDeleteAll(controllers);
    controllers.clear();
    QLayout *old = wPage->layout();
    wPage->setLayout(NULL);
    removeLayout(old);

    U2OpStatusImpl os;
    page->validate(wc->getCurrentActors(), os);
    if (os.hasError()) {
        setError(wPage);
        return;
    }

    PageContentCreator pcc(wc);
    page->getContent()->accept(&pcc);
    wPage->setLayout(pcc.getResult());
    controllers << pcc.getControllers();

    wPage->setTitle(page->getTitle());
    wPage->setFinalPage(page->isFinal());
}

void WizardPageController::setError(WDWizardPage *wPage) {
    QLayout *l = new QHBoxLayout(wPage);
    QString text = QObject::tr("The page is broken. Please, close the wizard and report us the error: ugene@unipro.ru");
    wPage->setFinalPage(true);
    l->addWidget(new QLabel(text));
    wPage->setLayout(l);
    wc->setBroken();
}

void WizardPageController::removeLayout(QLayout *l) {
    if (NULL != l) {
        QLayoutItem* item;
        while (NULL != (item = l->takeAt(0))) {
            if (item->widget()) {
                item->widget()->setParent(NULL);
            }
            delete item;
        }
        delete l;
    }
}

int WizardPageController::nextId() const {
    QString id = page->getNextId(wc->getVariables());
    if (id.isEmpty()) {
        return -1;
    }
    return wc->getQtId(id);
}

} // U2
