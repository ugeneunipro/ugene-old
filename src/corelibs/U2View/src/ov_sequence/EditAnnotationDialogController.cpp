/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QMenu>
#include <QMessageBox>

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationData.h>

#include <U2Formats/GenbankLocationParser.h>
#include <U2Formats/GenbankFeatures.h>

#include <U2Gui/CreateAnnotationWidgetController.h>

#include "EditAnnotationDialogController.h"
#include <U2Gui/HelpButton.h>

/* TRANSLATOR U2::EditAnnotationDialogController */

namespace U2 {

EditAnnotationDialogController::EditAnnotationDialogController( const AnnotationData &a, U2Region _seqRange, QWidget* p)
    : QDialog(p), seqRange(_seqRange)
{

    setupUi(this);
    new HelpButton(this, buttonBox, "14058956");

    nameEdit->setText(a.name);
    locationEdit->setText(Genbank::LocationParser::buildLocationString(&a));
    location = a.location;

    QMenu* menu = EditAnnotationDialogController::createAnnotationNamesMenu(this, this);
    showNameGroupsButton->setMenu(menu);
    showNameGroupsButton->setPopupMode(QToolButton::InstantPopup);

    connect(locationEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_onTextChanged(const QString&)));
    connect(complementButton, SIGNAL(clicked()), SLOT(sl_complementLocation()));
    connect(locationEdit, SIGNAL(returnPressed()), SLOT(accept()));
    connect(nameEdit, SIGNAL(returnPressed()), SLOT(accept()));
}

void EditAnnotationDialogController::sl_onTextChanged(const QString& changedText) {
    QByteArray locEditText = changedText.toLatin1();
    Genbank::LocationParser::parseLocation(locEditText.constData(), changedText.length(), location);
    if (location->isEmpty()) {
        if (changedText.isEmpty()) {
            statusLabel->setText("<b><font color=\"#A6392E\">" + tr("Location is empty!") + "</font></b>");
        } else{
            statusLabel->setText("<b><font color=\"#A6392E\">" + tr("Invalid location!") + "</font><b>");
        }
    } else {
        statusLabel->setText("");
    }
}

void EditAnnotationDialogController::accept(){
    QByteArray locEditText = locationEdit->text().toLatin1();
    Genbank::LocationParser::parseLocation(locEditText.constData(), locationEdit->text().length(), location);

    if ( location->isEmpty()) {
        QMessageBox::critical( this, tr( "Error!" ), tr( "Annotation location empty or invalid!" ) );
        QDialog::reject();
        return;
    }

    U2Region cRegion = U2Region::containingRegion(location->regions);
    bool validRegions = seqRange.contains(cRegion);

    if (!Annotation::isValidAnnotationName(nameEdit->text())) {
        QMessageBox::critical( this, tr( "Error!" ), tr( "Wrong annotation name!" ) );
        QDialog::reject();
    }else if (!validRegions){
        QMessageBox::critical( this, tr( "Error!" ), tr( "New annotation locations is out of sequence bounds!" ) );
        QDialog::reject();
    }else{
        currentName = nameEdit->text();
        QDialog::accept();
    }
}

static bool caseInsensitiveLessThan(const QString &s1, const QString &s2) {
    return s1.toLower() < s2.toLower();
}

QMenu* EditAnnotationDialogController::createAnnotationNamesMenu( QWidget* p, QObject* receiver )
{
    assert(p!=NULL && receiver!=NULL);

    QMenu* m = new QMenu(p);
    const QMultiMap<QString, GBFeatureKey>& nameGroups = GBFeatureUtils::getKeyGroups();
    QStringList groupNames = nameGroups.uniqueKeys();
    qSort(groupNames.begin(), groupNames.end(), caseInsensitiveLessThan);
    foreach(const QString& groupName, groupNames) {
        QMenu* groupMenu = m->addMenu(groupName);
        QList<GBFeatureKey> keys = nameGroups.values(groupName);
        QStringList names;
        foreach(GBFeatureKey k, keys) {
            names.append(GBFeatureUtils::getKeyInfo(k).text);
        }
        qSort(names.begin(), names.end(), caseInsensitiveLessThan);
        foreach(const QString& name, names) {
            QAction* a = new QAction(name, groupMenu);
            connect(a, SIGNAL(triggered()), receiver, SLOT(sl_setPredefinedAnnotationName()));
            groupMenu->addAction(a);
        }
    }
    return m;
}

void EditAnnotationDialogController::sl_setPredefinedAnnotationName() {
    QAction* a = qobject_cast<QAction*>(sender());
    QString text = a->text();
    nameEdit->setText(text);
}

void EditAnnotationDialogController::sl_complementLocation() {
    QString text = locationEdit->text();
    if (text.startsWith("complement(") && text.endsWith(")")) {
        locationEdit->setText(text.mid(11, text.length()-12));
    } else {
        locationEdit->setText("complement(" + text + ")");
    }
}

}// ns

