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

#include "CircularViewImageExportTask.h"
#include "CircularView.h"


#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPrinter>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QVBoxLayout>
#else
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#endif
#include <QtXml/QDomDocument>
#include <QtSvg/QSvgGenerator>
#include <iostream>
#include <U2Core/U2SafePoints.h>

namespace U2 {

void CircularViewImageExportToSVGTask::run() {
    SAFE_POINT_EXT(settings.isSVGFormat(),
               setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("CircularViewImageExportToSVGTask")), );

    QPainter painter;
    QSvgGenerator generator;
    generator.setFileName(settings.fileName);
    generator.setSize(cvWidget->size());
    generator.setViewBox(cvWidget->rect());

    painter.begin(&generator);
    cvWidget->paint(painter, cvWidget->width(), cvWidget->height(),
                    cvExportSettings.includeSelection,
                    cvExportSettings.includeMarker);
    bool result = painter.end();
    //fix for UGENE-76
    QDomDocument doc("svg");
    QFile file(settings.fileName);
    bool ok=file.open(QIODevice::ReadOnly);
    if (!ok && !result){
       result=false;
    }
    ok=doc.setContent(&file);
    if (!ok && !result) {
        file.close();
        result=false;
    }
    if(result){
        file.close();
        QDomNodeList radialGradients=doc.elementsByTagName("radialGradient");
        for(int i=0;i<radialGradients.length();i++){
            if(radialGradients.at(i).isElement()){
                QDomElement tag=radialGradients.at(i).toElement();
                if(tag.hasAttribute("xml:id")){
                    QString id=tag.attribute("xml:id");
                    tag.removeAttribute("xml:id");
                    tag.setAttribute("id",id);
                }
            }
        }
        file.open(QIODevice::WriteOnly);
        file.write(doc.toByteArray());
        file.close();
    }
    CHECK_EXT( result, setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

void CircularViewImageExportToPDFTask::run() {
    SAFE_POINT_EXT( settings.isPDFFormat(),
                    setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("CircularViewImageExportToPDFTask")), );

    QPainter painter;
    QPrinter printer;
    printer.setOutputFileName(settings.fileName);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.begin(&printer);
    cvWidget->paint(painter, cvWidget->width(), cvWidget->height(),
                    cvExportSettings.includeSelection,
                    cvExportSettings.includeMarker);

    CHECK_EXT( painter.end(), setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

void CircularViewImageExportToBitmapTask::run() {
    SAFE_POINT_EXT( settings.isBitmapFormat(),
                    setError(WRONG_FORMAT_MESSAGE.arg(settings.format).arg("CircularViewImageExportToBitmapTask")), );

    QPixmap *im = new QPixmap(settings.imageSize);
    im->fill(Qt::white);
    QPainter *painter = new QPainter(im);
    cvWidget->paint(*painter, settings.imageSize.width(), settings.imageSize.height(),
                    cvExportSettings.includeSelection,
                    cvExportSettings.includeMarker);

    CHECK_EXT( im->save(settings.fileName, qPrintable(settings.format), settings.imageQuality),
               setError(EXPORT_FAIL_MESSAGE.arg(settings.fileName)), );
}

CircularViewImageExportTaskFactory::CircularViewImageExportTaskFactory(CircularView *cv)
    : ImageExportTaskFactory(),
      cvWidget(cv)
{
    SAFE_POINT( cv != NULL, "Circular View is NULL!", );
    shortDescription = QObject::tr("Circular view");
    initSettingsWidget();
}

CircularViewImageExportTaskFactory::CircularViewImageExportTaskFactory(const QList<CircularView *> &list,
                                                                       CircularView* defaultCV)
    : ImageExportTaskFactory(),
      cvWidget(defaultCV),
      cvList(list) {
    SAFE_POINT( !list.isEmpty(), tr("List of Circular Views is empty!"), );
    if (defaultCV == NULL) {
        cvWidget = list.first();
    }

    shortDescription = QObject::tr("Circular view");
    initSettingsWidget();
}

int CircularViewImageExportTaskFactory::getImageWidth() const {
    return cvWidget->width();
}

int CircularViewImageExportTaskFactory::getImageHeight() const {
    return cvWidget->height();
}

void CircularViewImageExportTaskFactory::initSettingsWidget() {
    QVBoxLayout* layout = new QVBoxLayout();

    if (cvList.size() > 1) {
        QLabel* label = new QLabel(tr("Sequence"));
        sequenceComboBox = new QComboBox();
        foreach(CircularView* cv, cvList) {
            SAFE_POINT( cv->getSequenceContext() != NULL, tr("Sequence context is NULL!"), );
            SAFE_POINT( cv->getSequenceContext()->getSequenceGObject() != NULL, tr("Sequece Gobject is NULL"), );
            QString seqName = cv->getSequenceContext()->getSequenceGObject()->getGObjectName();
            sequenceComboBox->addItem(seqName);
            if (cv == cvWidget) {
                sequenceComboBox->setCurrentIndex( sequenceComboBox->count() - 1 );
            }
        }
        sequenceComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sequenceComboBox->setObjectName("Exported_sequence_combo");

        QHBoxLayout* seqLayout = new QHBoxLayout();
        seqLayout->addWidget(label);
        seqLayout->addWidget(sequenceComboBox);

        layout->addLayout(seqLayout);
    }

    includeMarkerCheckbox = new QCheckBox(QObject::tr("Include position marker"));
    includeSelectionCheckbox = new QCheckBox(QObject::tr("Include selection"));

    includeMarkerCheckbox->setChecked(false);
    includeSelectionCheckbox->setChecked(true);

    layout->addWidget(includeMarkerCheckbox);
    layout->addWidget(includeSelectionCheckbox);

    settingsWidget = new QWidget();
    settingsWidget->setLayout(layout);
}

Task* CircularViewImageExportTaskFactory::getExportToSVGTask(const ImageExportTaskSettings &settings) const {
    CircularViewImageExportSettings cvSettings(includeMarkerCheckbox->isChecked(),
                                               includeSelectionCheckbox->isChecked());
    updateCvWidget();
    return new CircularViewImageExportToSVGTask(cvWidget, cvSettings, settings);
}
Task* CircularViewImageExportTaskFactory::getExportToPDFTask(const ImageExportTaskSettings &settings) const {
    CircularViewImageExportSettings cvSettings(includeMarkerCheckbox->isChecked(),
                                               includeSelectionCheckbox->isChecked());
    updateCvWidget();
    return new CircularViewImageExportToPDFTask(cvWidget, cvSettings, settings);
}
Task* CircularViewImageExportTaskFactory::getExportToBitmapTask(const ImageExportTaskSettings &settings) const {
    CircularViewImageExportSettings cvSettings(includeMarkerCheckbox->isChecked(),
                                               includeSelectionCheckbox->isChecked());
    updateCvWidget();
    return new CircularViewImageExportToBitmapTask(cvWidget, cvSettings, settings);
}

void CircularViewImageExportTaskFactory::updateCvWidget() const {
    if (cvList.size() > 1) {
        SAFE_POINT(sequenceComboBox != NULL, "Sequence combo box is NULL", );
        cvWidget = cvList[ sequenceComboBox->currentIndex() ];
    }
}

} // namespace

