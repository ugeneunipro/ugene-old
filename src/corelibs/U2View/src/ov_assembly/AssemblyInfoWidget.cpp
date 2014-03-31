/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "AssemblyInfoWidget.h"
#include "AssemblyBrowser.h"

#include <U2Core/FormatUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ShowHideSubgroupWidget.h>


namespace U2 {


namespace {
    const int MAX_FIELD_LEN = 12;

    QString shorten(QString longStr, int maxLen = MAX_FIELD_LEN) {
        if(longStr.length() > maxLen) {
            return QString("%1... <a href='%2'>Copy</a>").arg(longStr.left(maxLen)).arg(longStr);
        } else {
            return longStr;
        }
    }
    QString shorthen(QByteArray longStr, int maxLen = MAX_FIELD_LEN) {
        return shorten(QString(longStr), maxLen);
    }

    QFormLayout * buildFormLayout(QWidget * w) {
        QFormLayout * layout = new QFormLayout;
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);
        w->setLayout(layout);
        return layout;
    }

    QLabel * buildLabel(QString text, QWidget *p = NULL) {
        text = QString("<b>%1:&nbsp;&nbsp;</b>").arg(text);
        QLabel * label = new QLabel(text, p);
        return label;
    }

    QLineEdit * buildLineEdit(QString text, QWidget *p = NULL) {
        QLineEdit * lineEdit = new QLineEdit(text, p);
        lineEdit->setStyleSheet("border: none; background-color: transparent;");
        lineEdit->setReadOnly(true);
        lineEdit->home(false);
        return lineEdit;
    }
}

AssemblyInfoWidget::AssemblyInfoWidget(AssemblyBrowser *browser, QWidget *p)
    : QWidget(p)
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    U2OpStatus2Log st;
    QSharedPointer<AssemblyModel> model = browser->getModel();

    QWidget * asmWidget = new QWidget(this);
    {
        QFormLayout * layout = buildFormLayout(asmWidget);

        QString name = model->getAssembly().visualName;
        QString length = FormatUtils::insertSeparators(model->getModelLength(st));
        QString reads = FormatUtils::insertSeparators(model->getReadsNumber(st));

        layout->addRow(buildLabel(tr("Name"),   asmWidget), buildLineEdit(name,   asmWidget));
        layout->addRow(buildLabel(tr("Length"), asmWidget), buildLineEdit(length, asmWidget));
        layout->addRow(buildLabel(tr("Reads"),  asmWidget), buildLineEdit(reads,  asmWidget));
    }
    QWidget * infoGroup = new ShowHideSubgroupWidget("INFO", tr("Assembly Information"), asmWidget, true);
    mainLayout->addWidget(infoGroup);

    QByteArray md5 = model->getReferenceMd5(st);
    QByteArray species = model->getReferenceSpecies(st);
    QString uri = model->getReferenceUri(st);

    if( !(md5+species+uri).isEmpty() ) {QWidget * refWidget = new QWidget(this);
        QFormLayout * layout = buildFormLayout(refWidget);
        if(!md5.isEmpty()) {
            layout->addRow(buildLabel(tr("MD5"), refWidget), buildLineEdit(QString(md5), refWidget));
        }
        if(!species.isEmpty()) {
            layout->addRow(buildLabel(tr("Species"), refWidget), buildLineEdit(QString(species), refWidget));
        }
        if(!uri.isEmpty()) {
            layout->addRow(buildLabel(tr("URI"), refWidget), buildLineEdit(uri, refWidget));
        }

        QWidget * refGroup = new ShowHideSubgroupWidget("REFERENCE", tr("Reference Information"), refWidget, false);
        mainLayout->addWidget(refGroup);
    }
}

//
// AssemblyInfoWidgetFactory
////////////////////////////////////
const QString AssemblyInfoWidgetFactory::GROUP_ID = "OP_ASS_INFO";
const QString AssemblyInfoWidgetFactory::GROUP_ICON_STR = ":core/images/chart_bar.png";
const QString AssemblyInfoWidgetFactory::GROUP_TITLE = QString(tr("Assembly Statistics"));
const QString AssemblyInfoWidgetFactory::GROUP_DOC_PAGE = "4227559";


AssemblyInfoWidgetFactory::AssemblyInfoWidgetFactory()
{
    objectViewOfWidget = ObjViewType_AssemblyBrowser;
}


QWidget* AssemblyInfoWidgetFactory::createWidget(GObjectView* objView)
{
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    AssemblyBrowser* assemblyBrowser = qobject_cast<AssemblyBrowser*>(objView);
    SAFE_POINT(NULL != assemblyBrowser,
        QString("Internal error: unable to cast object view to Assembly Browser for group '%1'.").arg(GROUP_ID),
        NULL);

    AssemblyInfoWidget* widget = new AssemblyInfoWidget(assemblyBrowser, assemblyBrowser->getMainWidget());
    return widget;
}


OPGroupParameters AssemblyInfoWidgetFactory::getOPGroupParameters()
{
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE, GROUP_DOC_PAGE);
}


} // namespace
