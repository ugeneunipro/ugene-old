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

#include "DocumentFormatSelectorController.h"

#include <U2Core/U2SafePoints.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DocumentImport.h>
#include <U2Gui/HelpButton.h>

#include <QtGui/QMessageBox>

namespace U2{


DocumentFormatSelectorController::DocumentFormatSelectorController(QList<FormatDetectionResult>& results, QWidget *p) 
: QDialog(p), formatDetectionResults(results)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "4227131");

    setObjectName("DocumentFormatSelectorDialog");
}



int DocumentFormatSelectorController::selectResult(const GUrl& url, QByteArray& rawData, QList<FormatDetectionResult>& results){
    SAFE_POINT(!results.isEmpty(), "Results list is empty!", -1);
    if (results.size() == 1) {
        return 0;
    }

    DocumentFormatSelectorController d(results, QApplication::activeModalWidget());
    d.optionsBox->setTitle(tr("Options for %1").arg(url.fileName()));
    QByteArray safeData = rawData;
    if (TextUtils::contains(TextUtils::BINARY, safeData.constData(), safeData.size())) {
        TextUtils::replace(safeData.data(), safeData.length(), TextUtils::BINARY, '?');
    }
    d.previewEdit->setPlainText(safeData);

    QVBoxLayout *vbox = new QVBoxLayout();
    QList<DocumentFormatId> detectedIds;
    for (int i =0; i < results.size(); i++) {
        const FormatDetectionResult& r = results[i];
        if (NULL != r.format) {
            detectedIds.append(r.format->getFormatId());
        }
        QString text;
        if (r.format != NULL) {
            text = tr("<b>%1</b> format. Score: %2 <i>(%3)</i>").arg(r.format->getFormatName()).arg(r.score()).arg(score2Text(r.score()));
        } else if (r.importer != NULL) {
            text = tr("<b><font color=red>Import: </font>%1</b>. Score: %2 (<i>%3</i>)").arg(r.importer->getImporterName()).arg(r.score()).arg(score2Text(r.score()));
        } else {
            assert(0);
            continue;
        }
        QHBoxLayout* hbox = new QHBoxLayout();
        QRadioButton* rb = new QRadioButton();
        rb->setChecked(i == 0);
        QLabel* label = new QLabel(text);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setSizePolicy(QSizePolicy::Expanding, label->sizePolicy().verticalPolicy());
        QToolButton* moreButton = new QToolButton();
        moreButton->setText("more..");
        moreButton->setEnabled(!r.getFormatDescriptionText().isEmpty());
        d.moreButtons << moreButton;
        QObject::connect(moreButton, SIGNAL(clicked()), &d, SLOT(sl_moreFormatInfo()));

        hbox->addWidget(rb);
        hbox->addWidget(label);
        hbox->addStretch(2);
        hbox->addWidget(moreButton);
        vbox->addLayout(hbox);
        d.radioButtons << rb;
    }
    //additional option: user selecting format
    {
        QString text(tr("Choose format manually"));
        QHBoxLayout* hbox = new QHBoxLayout();
        QRadioButton* rb = new QRadioButton();
        QLabel* label = new QLabel(text);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setSizePolicy(QSizePolicy::Expanding, label->sizePolicy().verticalPolicy());
        d.userSelectedFormat = new QComboBox();
        foreach(DocumentFormatId id, AppContext::getDocumentFormatRegistry()->getRegisteredFormats()){
            QString formatName = AppContext::getDocumentFormatRegistry()->getFormatById(id)->getFormatName();
            if(!detectedIds.contains(id)){
                d.userSelectedFormat->insertItem(0, formatName, id);
            }
        }

        hbox->addWidget(rb);
        hbox->addWidget(label);
        hbox->addStretch(2);
        hbox->addWidget(d.userSelectedFormat);
        vbox->addLayout(hbox);
        d.radioButtons << rb;
    }
    vbox->addStretch();
    d.optionsBox->setLayout(vbox);

    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return -1;
    }
    int idx = d.getSelectedFormatIdx();
    if(idx == results.size()){
        FormatDetectionResult *r = new FormatDetectionResult();
        DocumentFormatId id = d.userSelectedFormat->itemData(d.userSelectedFormat->currentIndex()).toString();
        r->format = AppContext::getDocumentFormatRegistry()->getFormatById(id);
        results.insert(idx, *r);
    }
    return idx;
}

QString DocumentFormatSelectorController::score2Text(int score) {
    if (score <= FormatDetection_NotMatched) {
        return tr("Not matched");
    }
    if (score <= FormatDetection_VeryLowSimilarity) {
        return tr("Very low similarity");
    }
    if (score <= FormatDetection_LowSimilarity) {
        return tr("Low similarity");
    }
    if (score <= FormatDetection_AverageSimilarity) {
        return tr("Average similarity");
    }
    if (score <= FormatDetection_HighSimilarity) {
        return tr("High similarity");
    }
    if (score < FormatDetection_Matched) {
        return tr("Very high similarity");
    }
    return tr("Perfect match");
}

void DocumentFormatSelectorController::sl_moreFormatInfo() {
    QToolButton* tb = qobject_cast<QToolButton*>(sender());
    SAFE_POINT(tb != NULL, "Failed to derive selected format info!", );
    int idx = moreButtons.indexOf(tb);
    const FormatDetectionResult& dr = formatDetectionResults[idx];
    QMessageBox::information(this, tr("Format details for '%1' format").arg(dr.getFormatOrImporterName()), dr.getFormatDescriptionText());
}


int DocumentFormatSelectorController::getSelectedFormatIdx() const {
    int idx = 0;
    for (int i =0; i < radioButtons.size(); i++) {
        if (radioButtons[i]->isChecked()) {
            idx = i;
            break;
        }
    }
    return idx;
}

} //namespace
