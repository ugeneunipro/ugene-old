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


/************************************************************************/
/* Color Schema Settings Controller                                   */
/************************************************************************/



#include "ColorSchemaSettingsController.h"

#include "ColorSchemaDialogController.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Settings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/IOAdapter.h>

#include <QFileDialog>
#include <QtCore/QDir>

#include <QtGui/QColor>

#include <memory>

#define SETTINGS_ROOT          QString("/color_schema_settings/")
#define COLOR_SCHEMA_DIR       QString("colors_schema_dir")
#define AMINO_SETTINGS         QString("amino.txt")
#define NUCL_SETTINGS          QString("nucl.txt")

namespace U2{

static QString getColorsDir() {
    QString defaultDir = AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath();
    QString res = AppContext::getSettings()->getValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, defaultDir).toString();

    return res;
}

static void setColorsDir(const QString &colorsDir) {
    QString defaultDir = AppContext::getSettings()->getValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, defaultDir).toString();
    if (defaultDir != colorsDir) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, colorsDir);
    }
}

static void setAlphabetColors(const QMap<char, QColor> & alphColors, const QString& settingsFile){
    QString dirPath = getColorsDir();
    QDir dir(dirPath);
    if(!dir.exists()){
        dir.mkpath(dirPath);
    }

    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);

    std::auto_ptr<IOAdapter> io(factory->createIOAdapter());

    io->open(dir.filePath(settingsFile), IOAdapterMode_Write);
    QMapIterator<char, QColor> it(alphColors);
    while(it.hasNext()){
        it.next();
        QByteArray line;
        line.append(QString("%1=%2\n").arg(it.key()).arg(it.value().name()));
        io->writeBlock(line);
    }
}

static bool lineValid(const QStringList& properties, const QMap<char, QColor> & alphColors){
    if(properties.size() != 2){return false;}
    if(properties[0].size() != 1 || (!alphColors.contains(properties[0][0].toAscii()))){return false;}
    if(!QColor(properties[1]).isValid()){return false;}

    return true;
}

static void getAlphabetColors(QMap<char, QColor> & alphColors, const QString& settingsFile){
    QString dirPath = getColorsDir();
    QDir dir(dirPath);
    if(!dir.exists()){
        dir.mkpath(dirPath);
    }

    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);

    std::auto_ptr<IOAdapter> io(factory->createIOAdapter());
    if(!io->open(dir.filePath(settingsFile), IOAdapterMode_Read)){
        setAlphabetColors(alphColors, settingsFile);
        return;
    }
    QMap<char, QColor> tmpHelper;
    while(!io->isEof()){
        const int max_size = 1024;

        QByteArray byteLine(max_size + 1, 0);
        int lineLength = io->readLine(byteLine.data(), max_size);
        QString line(byteLine);
        line.remove(lineLength, line.size() - lineLength);
        if(line.isEmpty()){continue;}
        QStringList properties = line.split(QString("="), QString::SkipEmptyParts);
        if(!lineValid(properties, alphColors )){return;}
        tmpHelper[properties.first().at(0).toAscii()] = QColor(properties[1]); 
    }
    QMapIterator<char, QColor> it(tmpHelper);
    while(it.hasNext()){
        it.next();
        alphColors[it.key()] = it.value();
    }
}


QMap<DNAAlphabetType, QMap<char, QColor> > ColorSchemaSettingsUtils::getColors(){
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    QMap<DNAAlphabetType, QMap<char, QColor> > alphabetsColors;
    foreach(DNAAlphabet* alphabet, alphabets){ // default initialization
        QByteArray alphabetChars = alphabet->getAlphabetChars();
        for(int i = 0; i < alphabetChars.size(); ++i){
            alphabetsColors[alphabet->getType()][alphabetChars.at(i)] = QColor(Qt::white);
        }
    }

    getAlphabetColors(alphabetsColors[DNAAlphabet_AMINO], AMINO_SETTINGS);
    getAlphabetColors(alphabetsColors[DNAAlphabet_NUCL], NUCL_SETTINGS);

    return alphabetsColors;
}

ColorSchemaSettingsPageController::ColorSchemaSettingsPageController(QObject* p) 
: AppSettingsGUIPageController(tr("MSA color schema"), ColorSchemaSettingsPageId, p) {}


AppSettingsGUIPageState* ColorSchemaSettingsPageController::getSavedState() {
    ColorSchemaSettingsPageState* state = new ColorSchemaSettingsPageState();
    state->colorsDir = getColorsDir();
    state->customColors = ColorSchemaSettingsUtils::getColors();
    return state;
}

void ColorSchemaSettingsPageController::saveState(AppSettingsGUIPageState* s) {
    ColorSchemaSettingsPageState* state = qobject_cast<ColorSchemaSettingsPageState*>(s);

    setColorsDir(state->colorsDir);
    setAlphabetColors(state->customColors[DNAAlphabet_AMINO] , AMINO_SETTINGS);
    setAlphabetColors(state->customColors[DNAAlphabet_NUCL] , NUCL_SETTINGS);
}

AppSettingsGUIPageWidget* ColorSchemaSettingsPageController::createWidget(AppSettingsGUIPageState* state) {
    ColorSchemaSettingsPageWidget* r = new ColorSchemaSettingsPageWidget(this);
    r->setState(state);
    return r;
}

ColorSchemaSettingsPageWidget::ColorSchemaSettingsPageWidget(ColorSchemaSettingsPageController* ) {
    setupUi(this);
    connect(colorsDirButton, SIGNAL(clicked()), SLOT(sl_onColorsDirButton()));
    connect(changeSchemaButton, SIGNAL(clicked()), SLOT(sl_onChangeColorSchema()));
}

void ColorSchemaSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    ColorSchemaSettingsPageState* state = qobject_cast<ColorSchemaSettingsPageState*>(s);
    colorsDirEdit->setText(state->colorsDir);
    customColors = state->customColors;
}

AppSettingsGUIPageState* ColorSchemaSettingsPageWidget::getState(QString& ) const {
    ColorSchemaSettingsPageState* state = new ColorSchemaSettingsPageState();
    state->colorsDir = colorsDirEdit->text();
    state->customColors = customColors;
    return state;
}

void ColorSchemaSettingsPageWidget::sl_onColorsDirButton() {
    QString path = colorsDirEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), path,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        colorsDirEdit->setText(dir);
    }
}

void ColorSchemaSettingsPageWidget::sl_onChangeColorSchema(){    

    QMap<char, QColor> alpColors;
    DNAAlphabetType type;

    if(nucleotideMode->isChecked()){
        type = DNAAlphabet_NUCL;
    }
    else if(aminoMode->isChecked()){
        type = DNAAlphabet_AMINO;        
    }
    alpColors = customColors[type];

    ColorSchemaDialogController controller(alpColors);    
    int r = controller.adjustAlphabetColors();
    if(r == QDialog::Rejected){return;}

    QMapIterator<char, QColor> it(alpColors);
    while(it.hasNext()){
        it.next();
        customColors[type][it.key()] = it.value();
    }
}

} // namespase