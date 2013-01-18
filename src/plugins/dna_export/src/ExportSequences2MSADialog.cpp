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

#include "ExportSequences2MSADialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

const QString NEW_LINE_SYMBOL = "\n";
const QString COLOR_NAME_FOR_WARNING_MESSAGES = "orange";
const QString STYLESHEET_COLOR_DEFINITION = "color: ";
const QString STYLESHEET_DEFINITIONS_SEPARATOR = ";";

ExportSequences2MSADialog::ExportSequences2MSADialog(QWidget* p, const QString& defaultUrl): QDialog(p) {
    setupUi(this);    
    addToProjectFlag = true;
    useGenbankHeader = false;

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFileName = defaultUrl;
    conf.defaultFormatId = BaseDocumentFormats::CLUSTAL_ALN;
    saveContoller = new SaveDocumentGroupController(conf, this);

    connect(formatCombo, SIGNAL(currentIndexChanged(QString)), SLOT(sl_formatChanged(QString)));
    messageLabel->setStyleSheet(
        "color: " + L10N::errorColorLabelStr() + ";"
        "font: bold;");
    messageLabel->hide();
    sl_formatChanged(QString());
}


void ExportSequences2MSADialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = saveContoller->getSaveFileName();
    format = saveContoller->getFormatIdToSave();
    addToProjectFlag = addToProjectBox->isChecked();
    useGenbankHeader = genbankBox->isChecked();

    QDialog::accept();
}

void ExportSequences2MSADialog::setOkButtonText(const QString& text) const {
    okButton->setText(text);
}

void ExportSequences2MSADialog::setFileLabelText(const QString& text) const {
    fileLabel->setText(text);
}

void ExportSequences2MSADialog::sl_formatChanged(QString newFormat) {
    Q_UNUSED(newFormat);
    if (formatCombo->currentText() == "CLUSTALW") {
        showHideMessage(true, CutNames);
    } else {
        showHideMessage(false, CutNames);
    }
}

void ExportSequences2MSADialog::showHideMessage(bool show, MessageFlag messageFlag) {
    if (show) {
        if (!messageFlags.contains(messageFlag)) {
            messageFlags.append(messageFlag);
        }
    }
    else {
        messageFlags.removeAll(messageFlag);
    }

    if (!messageFlags.isEmpty()) {
        static QString storedTextColor = currentColorOfMessageText();
        if(storedTextColor != currentColorOfMessageText())
            changeColorOfMessageText(storedTextColor);

        QString text = "";
        foreach (MessageFlag flag, messageFlags) {
            switch (flag) {
                case CutNames:
                    if (!text.isEmpty()) {
                        text += "\n";
                    }
                    changeColorOfMessageText(COLOR_NAME_FOR_WARNING_MESSAGES);
                    text += QString(tr("All sequence`s names will be cut to 39 symbols"));
                    break;

                default:
                    assert(0);
            }
        }
        messageLabel->setText(text);
        messageLabel->show();
    }
    else {
        messageLabel->hide();
        messageLabel->setText("");
    }
}

void ExportSequences2MSADialog::changeColorOfMessageText(const QString &newColorName)
{
    QString currentStyleSheet = messageLabel->styleSheet();
    currentStyleSheet.replace(currentColorOfMessageText(), newColorName);
    messageLabel->setStyleSheet(currentStyleSheet);
}

QString ExportSequences2MSADialog::currentColorOfMessageText() const
{
    const QString currentStyleSheet = messageLabel->styleSheet();
    const int startOfColorDefinitionPosition = currentStyleSheet.indexOf(STYLESHEET_COLOR_DEFINITION);
    const int endOfColorDefinitionPosition = currentStyleSheet.indexOf(STYLESHEET_DEFINITIONS_SEPARATOR,
        startOfColorDefinitionPosition);
    const QString currentMessageTextColor = currentStyleSheet.mid(startOfColorDefinitionPosition
                                                                  + STYLESHEET_COLOR_DEFINITION.length(),
                                                                  endOfColorDefinitionPosition
                                                                  - startOfColorDefinitionPosition
                                                                  - STYLESHEET_COLOR_DEFINITION.length());
    return currentMessageTextColor;
}

}//namespace
