/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <cfloat>
#include <QtGui/QMessageBox>
#include <U2View/AnnotatedDNAView.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/L10n.h>
#include "Primer3Dialog.h"

namespace U2 {

Primer3Dialog::Primer3Dialog(const Primer3TaskSettings &defaultSettings, ADVSequenceObjectContext *context):
        QDialog(context->getAnnotatedDNAView()->getWidget()),
        defaultSettings(defaultSettings)
{
    ui.setupUi(this);

    {
        CreateAnnotationModel createAnnotationModel;
        createAnnotationModel.data->name = "top primers";
        createAnnotationModel.sequenceObjectRef = GObjectReference(context->getSequenceGObject());
        createAnnotationModel.hideAnnotationName = false;
        createAnnotationModel.hideLocation = true;
        createAnnotationWidgetController = new CreateAnnotationWidgetController(createAnnotationModel,this);
        ui.annotationWidgetLayout->addWidget(createAnnotationWidgetController->getWidget());
    }

    if(!context->getSequenceSelection()->getSelectedRegions().isEmpty())
    {
        selection = context->getSequenceSelection()->getSelectedRegions().first();
    }
    sequenceLength = context->getSequenceLength();
    rs=new RegionSelector(this, sequenceLength, false, context->getSequenceSelection());
    ui.rangeSelectorLayout->addWidget(rs);

    repeatLibraries.append(QPair<QString, QByteArray>(tr("NONE"), ""));
    repeatLibraries.append(QPair<QString, QByteArray>(tr("HUMAN"), "primer3/humrep_and_simple.txt"));
    repeatLibraries.append(QPair<QString, QByteArray>(tr("RODENT_AND_SIMPLE"), "primer3/rodrep_and_simple.txt"));
    repeatLibraries.append(QPair<QString, QByteArray>(tr("RODENT"), "primer3/rodent_ref.txt"));
    repeatLibraries.append(QPair<QString, QByteArray>(tr("DROSOPHILA"), "primer3/drosophila.w.transposons.txt"));

    for(int i=0;i < repeatLibraries.size();i++)
    {
        if(!repeatLibraries[i].second.isEmpty())
        repeatLibraries[i].second = QFileInfo(QString(PATH_PREFIX_DATA) + ":" + repeatLibraries[i].second).absoluteFilePath().toAscii();
    }

    {
        QPair<QString, QByteArray> library;
        foreach(library, repeatLibraries)
        {
            ui.combobox_PRIMER_MISPRIMING_LIBRARY->addItem(library.first);
            ui.combobox_PRIMER_INTERNAL_OLIGO_MISHYB_LIBRARY->addItem(library.first);
        }
    }
    reset();
}

Primer3TaskSettings Primer3Dialog::getSettings()const
{
    return settings;
}

const CreateAnnotationModel &Primer3Dialog::getCreateAnnotationModel()const
{
    return createAnnotationWidgetController->getModel();
}

U2Region Primer3Dialog::getRegion(bool *ok) const
{
    return rs->getRegion(ok);
}

void Primer3Dialog::prepareAnnotationObject()
{
   createAnnotationWidgetController->prepareAnnotationObject();
}

QString Primer3Dialog::intervalListToString(QList<QPair<int, int> > intervalList, QString delimiter) {
    QString result;
    bool first = true;
    QPair<int, int> interval;
    foreach(interval, intervalList)
    {
        if(!first)
        {
            result += " ";
        }
        result += QString::number(interval.first);
        result += delimiter;
        result += QString::number(interval.second);
        first = false;
    }
    return result;
}

bool Primer3Dialog::parseIntervalList(QString inputString, QString delimiter, QList<QPair<int, int> > *outputList) {
    QList<QPair<int, int> > result;
    QStringList intervalStringList = inputString.split(QRegExp("\\s+"),QString::SkipEmptyParts);
    foreach(QString intervalString, intervalStringList)
    {
        QStringList valueStringList = intervalString.split(delimiter);
        if(2 != valueStringList.size())
        {
            return false;
        }
        int firstValue = 0;
        {
            bool ok = false;
            firstValue = valueStringList[0].toInt(&ok);
            if(!ok)
            {
                return false;
            }
        }
        int secondValue = 0;
        {
            bool ok = false;
            secondValue = valueStringList[1].toInt(&ok);
            if(!ok)
            {
                return false;
            }
        }
        result.append(qMakePair(firstValue, secondValue));
    }
    *outputList = result;
    return true;
}

void Primer3Dialog::reset()
{
    foreach(QString key, defaultSettings.getIntPropertyList())
    {
        int value = 0;
        if(defaultSettings.getIntProperty(key,&value))
        {
            QSpinBox *spinBox = findChild<QSpinBox *>("edit_" + key);
            if(NULL != spinBox)
            {
                spinBox->setValue(value);
            }
        }
    }
    foreach(QString key, defaultSettings.getDoublePropertyList())
    {
        double value = 0;
        if(defaultSettings.getDoubleProperty(key,&value))
        {
            QDoubleSpinBox *spinBox = findChild<QDoubleSpinBox *>("edit_" + key);
            if(NULL != spinBox)
            {
                spinBox->setValue(value);
            }
        }
    }
    foreach(QString key, defaultSettings.getAlignPropertyList())
    {
        short value = 0;
        if(defaultSettings.getAlignProperty(key,&value))
        {
            QDoubleSpinBox *spinBox = findChild<QDoubleSpinBox *>("edit_" + key);
            if(NULL != spinBox)
            {
                spinBox->setValue((double)value/100);
            }
        }
    }
    ui.edit_PRIMER_START_CODON_POSITION->clear();
    ui.edit_PRIMER_PRODUCT_MIN_TM->clear();
    ui.edit_PRIMER_PRODUCT_OPT_TM->clear();
    ui.edit_PRIMER_PRODUCT_MAX_TM->clear();
    ui.edit_PRIMER_OPT_GC_PERCENT->clear();
    ui.edit_PRIMER_INSIDE_PENALTY->clear();
    ui.edit_PRIMER_INTERNAL_OLIGO_OPT_GC_PERCENT->clear();
    ui.edit_EXCLUDED_REGION->setText(intervalListToString(defaultSettings.getExcludedRegion(),","));
    ui.edit_PRIMER_INTERNAL_OLIGO_EXCLUDED_REGION->setText(intervalListToString(defaultSettings.getInternalOligoExcludedRegion(),","));
    ui.edit_TARGET->setText(intervalListToString(defaultSettings.getTarget(),","));
    ui.edit_PRIMER_PRODUCT_SIZE_RANGE->setText(intervalListToString(defaultSettings.getProductSizeRange(),"-"));
    ui.edit_PRIMER_LEFT_INPUT->setText(defaultSettings.getLeftInput());
    ui.edit_PRIMER_RIGHT_INPUT->setText(defaultSettings.getRightInput());
    ui.edit_PRIMER_INTERNAL_OLIGO_INPUT->setText(defaultSettings.getInternalInput());
    {
        QString qualityString;
        bool first = true;
        foreach(int qualityValue, defaultSettings.getSequenceQuality())
        {
            if(!first)
            {
                qualityString += " ";
            }
            qualityString += QString::number(qualityValue);
            first = false;
        }
        ui.edit_PRIMER_SEQUENCE_QUALITY->setPlainText(qualityString);
    }
    {
        int value = 0;
        defaultSettings.getIntProperty("PRIMER_TM_SANTALUCIA",&value);
        ui.combobox_PRIMER_TM_SANTALUCIA->setCurrentIndex(value);
    }
    {
        int value = 0;
        defaultSettings.getIntProperty("PRIMER_SALT_CORRECTIONS",&value);
        ui.combobox_PRIMER_SALT_CORRECTIONS->setCurrentIndex(value);
    }
    {
        int value = 0;
        defaultSettings.getIntProperty("PRIMER_LIBERAL_BASE",&value);
        ui.checkbox_PRIMER_LIBERAL_BASE->setChecked(value);
    }
    {
        int value = 0;
        defaultSettings.getIntProperty("PRIMER_LIB_AMBIGUITY_CODES_CONSENSUS",&value);
        ui.checkbox_PRIMER_LIB_AMBIGUITY_CODES_CONSENSUS->setChecked(value);
    }
    {
        int value = 0;
        defaultSettings.getIntProperty("PRIMER_LOWERCASE_MASKING",&value);
        ui.checkbox_PRIMER_LOWERCASE_MASKING->setChecked(value);
    }

    {
        switch(defaultSettings.getTask())
        {
        case pick_pcr_primers:
            ui.checkbox_PICK_LEFT->setChecked(true);
            ui.checkbox_PICK_RIGHT->setChecked(true);
            ui.checkbox_PICK_HYBRO->setChecked(false);
            break;
        case pick_left_only:
            ui.checkbox_PICK_LEFT->setChecked(true);
            ui.checkbox_PICK_RIGHT->setChecked(false);
            ui.checkbox_PICK_HYBRO->setChecked(false);
            break;
        case pick_right_only:
            ui.checkbox_PICK_LEFT->setChecked(false);
            ui.checkbox_PICK_RIGHT->setChecked(true);
            ui.checkbox_PICK_HYBRO->setChecked(false);
            break;
        case pick_hyb_probe_only:
            ui.checkbox_PICK_LEFT->setChecked(false);
            ui.checkbox_PICK_RIGHT->setChecked(false);
            ui.checkbox_PICK_HYBRO->setChecked(true);
            break;
        case pick_pcr_primers_and_hyb_probe:
            ui.checkbox_PICK_LEFT->setChecked(true);
            ui.checkbox_PICK_RIGHT->setChecked(true);
            ui.checkbox_PICK_HYBRO->setChecked(true);
            break;
        }
    }
    ui.edit_PRIMER_LEFT_INPUT->setEnabled(ui.checkbox_PICK_LEFT->isChecked());
    ui.edit_PRIMER_RIGHT_INPUT->setEnabled(ui.checkbox_PICK_RIGHT->isChecked());
    ui.edit_PRIMER_INTERNAL_OLIGO_INPUT->setEnabled(ui.checkbox_PICK_HYBRO->isChecked());


    ui.combobox_PRIMER_MISPRIMING_LIBRARY->setCurrentIndex(0);
    ui.combobox_PRIMER_INTERNAL_OLIGO_MISHYB_LIBRARY->setCurrentIndex(0);
    {
        for(int i=0;i < repeatLibraries.size();i++)
        {
            if(repeatLibraries[i].second == settings.getRepeatLibrary())
            {
                ui.combobox_PRIMER_MISPRIMING_LIBRARY->setCurrentIndex(i);
            }
            if(repeatLibraries[i].second == settings.getMishybLibrary())
            {
                ui.combobox_PRIMER_INTERNAL_OLIGO_MISHYB_LIBRARY->setCurrentIndex(i);
            }
        }
    }
}

bool Primer3Dialog::doDataExchange()
{
    settings = defaultSettings;
    foreach(QString key, settings.getIntPropertyList())
    {
        QSpinBox *spinBox = findChild<QSpinBox *>("edit_" + key);
        if(NULL != spinBox)
        {
            settings.setIntProperty(key,spinBox->value());
        }
    }
    foreach(QString key, settings.getDoublePropertyList())
    {
        QDoubleSpinBox *spinBox = findChild<QDoubleSpinBox *>("edit_" + key);
        if(NULL != spinBox)
        {
            settings.setDoubleProperty(key,spinBox->value());
        }
    }
    foreach(QString key, settings.getAlignPropertyList())
    {
        QDoubleSpinBox *spinBox = findChild<QDoubleSpinBox *>("edit_" + key);
        if(NULL != spinBox)
        {
            settings.setAlignProperty(key,(short)(spinBox->value()*100));
        }
    }
    if(!ui.edit_PRIMER_START_CODON_POSITION->text().isEmpty())
    {
        bool ok = false;
        int value = ui.edit_PRIMER_START_CODON_POSITION->text().toInt(&ok);
        if(ok)
        {
            settings.setIntProperty("PRIMER_START_CODON_POSITION", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_START_CODON_POSITION, tr("Start Codon Position"));
            return false;
        }
    }
    if(!ui.edit_PRIMER_PRODUCT_MIN_TM->text().isEmpty())
    {
        bool ok = false;
        double value = ui.edit_PRIMER_PRODUCT_MIN_TM->text().toDouble(&ok);
        if(ok)
        {
            settings.setDoubleProperty("PRIMER_PRODUCT_MIN_TM", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_PRODUCT_MIN_TM, tr("Product Min Tm"));
            return false;
        }
    }
    if(!ui.edit_PRIMER_PRODUCT_OPT_TM->text().isEmpty())
    {
        bool ok = false;
        double value = ui.edit_PRIMER_PRODUCT_OPT_TM->text().toDouble(&ok);
        if(ok)
        {
            settings.setDoubleProperty("PRIMER_PRODUCT_OPT_TM", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_PRODUCT_OPT_TM, tr("Product Opt Tm"));
            return false;
        }
    }
    if(!ui.edit_PRIMER_PRODUCT_MAX_TM->text().isEmpty())
    {
        bool ok = false;
        double value = ui.edit_PRIMER_PRODUCT_MAX_TM->text().toDouble(&ok);
        if(ok)
        {
            settings.setDoubleProperty("PRIMER_PRODUCT_MAX_TM", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_PRODUCT_MAX_TM, tr("Product Max Tm"));
            return false;
        }
    }
    if(!ui.edit_PRIMER_OPT_GC_PERCENT->text().isEmpty())
    {
        bool ok = false;
        short value = (short)(ui.edit_PRIMER_OPT_GC_PERCENT->text().toDouble(&ok)*100);
        if(ok)
        {
            settings.setAlignProperty("PRIMER_OPT_GC_PERCENT", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_OPT_GC_PERCENT, tr("Opt GC%"));
            return false;
        }
    }
    if(!ui.edit_PRIMER_INSIDE_PENALTY->text().isEmpty())
    {
        bool ok = false;
        double value = ui.edit_PRIMER_INSIDE_PENALTY->text().toDouble(&ok);
        if(ok)
        {
            settings.setDoubleProperty("PRIMER_INSIDE_PENALTY", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_INSIDE_PENALTY, tr("Inside Penalty"));
            return false;
        }
    }
    if(!ui.edit_PRIMER_INTERNAL_OLIGO_OPT_GC_PERCENT->text().isEmpty())
    {
        bool ok = false;
        short value = (short)(ui.edit_PRIMER_INTERNAL_OLIGO_OPT_GC_PERCENT->text().toDouble(&ok)*100);
        if(ok)
        {
            settings.setAlignProperty("PRIMER_INTERNAL_OLIGO_OPT_GC_PERCENT", value);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_INTERNAL_OLIGO_OPT_GC_PERCENT, tr("Internal Oligo Opt Tm"));
            return false;
        }
    }
    {
        QList<QPair<int, int> > list;
        if(parseIntervalList(ui.edit_EXCLUDED_REGION->text(), ",", &list))
        {
            settings.setExcludedRegion(list);
        }
        else
        {
            showInvalidInputMessage(ui.edit_EXCLUDED_REGION, tr("Excluded Regions"));
            return false;
        }
    }
    {
        QList<QPair<int, int> > list;
        if(parseIntervalList(ui.edit_PRIMER_INTERNAL_OLIGO_EXCLUDED_REGION->text(), ",", &list))
        {
            settings.setInternalOligoExcludedRegion(list);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_INTERNAL_OLIGO_EXCLUDED_REGION, tr("Internal Oligo Excluded Regions"));
            return false;
        }
    }
    {
        QList<QPair<int, int> > list;
        if(parseIntervalList(ui.edit_TARGET->text(), ",", &list))
        {
            settings.setTarget(list);
        }
        else
        {
            showInvalidInputMessage(ui.edit_TARGET, tr("Targets"));
            return false;
        }
    }
    {
        QList<QPair<int, int> > list;
        if(parseIntervalList(ui.edit_PRIMER_PRODUCT_SIZE_RANGE->text(), "-", &list))
        {
            settings.setProductSizeRange(list);
        }
        else
        {
            showInvalidInputMessage(ui.edit_PRIMER_PRODUCT_SIZE_RANGE, tr("Product Size Ranges"));
            return false;
        }
    }
    if(ui.checkbox_PICK_LEFT->isChecked())
    {
        settings.setLeftInput(ui.edit_PRIMER_LEFT_INPUT->text().toAscii());
    }
    else
    {
        settings.setLeftInput("");
    }
    if(ui.checkbox_PICK_RIGHT->isChecked())
    {
        settings.setRightInput(ui.edit_PRIMER_RIGHT_INPUT->text().toAscii());
    }
    else
    {
        settings.setRightInput("");
    }
    if(ui.checkbox_PICK_HYBRO->isChecked())
    {
        settings.setInternalInput(ui.edit_PRIMER_INTERNAL_OLIGO_INPUT->text().toAscii());
    }
    else
    {
        settings.setInternalInput("");
    }
    {
        QVector<int> qualityList;
        QStringList stringList = ui.edit_PRIMER_SEQUENCE_QUALITY->toPlainText().split(QRegExp("\\s+"),QString::SkipEmptyParts);
        foreach(QString string, stringList)
        {
            bool ok = false;
            int value = string.toInt(&ok);
            if(!ok)
            {
                showInvalidInputMessage(ui.edit_PRIMER_SEQUENCE_QUALITY, tr("Sequence Quality"));
                return false;
            }
            qualityList.append(value);
        }
        if(!qualityList.isEmpty() && (qualityList.size() != (rs->getRegion().length)))//todo add check on wrong region
        {
            QMessageBox::critical(this, windowTitle(), tr("Sequence quality list length must be equal to the sequence length"));
            return false;
        }
        settings.setSequenceQuality(qualityList);
    }
    settings.setIntProperty("PRIMER_TM_SANTALUCIA", ui.combobox_PRIMER_TM_SANTALUCIA->currentIndex());
    settings.setIntProperty("PRIMER_SALT_CORRECTIONS", ui.combobox_PRIMER_SALT_CORRECTIONS->currentIndex());
    settings.setIntProperty("PRIMER_LIBERAL_BASE", ui.checkbox_PRIMER_LIBERAL_BASE->isChecked());
    settings.setIntProperty("PRIMER_LIB_AMBIGUITY_CODES_CONSENSUS", ui.checkbox_PRIMER_LIB_AMBIGUITY_CODES_CONSENSUS->isChecked());
    settings.setIntProperty("PRIMER_LOWERCASE_MASKING", ui.checkbox_PRIMER_LOWERCASE_MASKING->isChecked());
    if(ui.checkbox_PICK_HYBRO->isChecked())
    {
        if(ui.checkbox_PICK_LEFT->isChecked() && ui.checkbox_PICK_RIGHT->isChecked())
        {
            settings.setTask(pick_pcr_primers_and_hyb_probe);
        }
        else if(!ui.checkbox_PICK_LEFT->isChecked() && !ui.checkbox_PICK_RIGHT->isChecked())
        {
            settings.setTask(pick_hyb_probe_only);
        }
        else
        {
            QMessageBox::critical(this, windowTitle(), tr("Can't pick hyb oligo and only one primer"));
            return false;
        }
    }
    else
    {
        if(ui.checkbox_PICK_LEFT->isChecked() && ui.checkbox_PICK_RIGHT->isChecked())
        {
            settings.setTask(pick_pcr_primers);
        }
        else if(ui.checkbox_PICK_LEFT->isChecked())
        {
            settings.setTask(pick_left_only);
        }
        else if(ui.checkbox_PICK_RIGHT->isChecked())
        {
            settings.setTask(pick_right_only);
        }
        else
        {
            QMessageBox::critical(this, windowTitle(), tr("Nothing to pick"));
            return false;
        }
    }
    {
        int index = ui.combobox_PRIMER_MISPRIMING_LIBRARY->currentIndex();
        settings.setRepeatLibrary(repeatLibraries[index].second);
    }
    {
        int index = ui.combobox_PRIMER_INTERNAL_OLIGO_MISHYB_LIBRARY->currentIndex();
        settings.setMishybLibrary(repeatLibraries[index].second);
    }
    return true;
}

void Primer3Dialog::showInvalidInputMessage(QWidget *field, QString fieldLabel)
{
    ui.tabWidget->setCurrentWidget(field->parentWidget());
    field->setFocus(Qt::OtherFocusReason);
    QMessageBox::critical(this, windowTitle(), tr("The field '%1' has invalid value").arg(fieldLabel));
}

void Primer3Dialog::on_pbPick_clicked()
{
    bool isRegionOk=false;
    rs->getRegion(&isRegionOk);
    if(!isRegionOk){
        rs->showErrorMessage();
        return;
    }
    if(doDataExchange())
    {
        accept();
    }
}

void Primer3Dialog::on_pbReset_clicked()
{
    reset();
    rs->reset();
}

QString Primer3Dialog::checkModel()
{
    return createAnnotationWidgetController->validate();
}

} // namespace U2
