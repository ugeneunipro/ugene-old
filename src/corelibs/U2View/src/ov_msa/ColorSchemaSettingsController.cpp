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
#include <U2Core/Log.h>

#include <QFileDialog>
#include <QtCore/QDir>

#include <QtGui/QColor>

#include <memory>

#define SETTINGS_ROOT          QString("/color_schema_settings/")
#define COLOR_SCHEMA_DIR       QString("colors_schema_dirrs")
#define AMINO_KEYWORD          QString("AMINO")
#define NUCL_KEYWORD           QString("NUCL")

#define NAME_FILTERS           QString(".csmsa")

namespace U2{


static QString getColorsDir() {
    QString settingsFile = AppContext::getSettings()->fileName();
    QString settingsDir = QFileInfo(settingsFile).absolutePath(); 
    
    QString res = AppContext::getSettings()->getValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, settingsDir).toString();

    return res;
}

static void setColorsDir(const QString &colorsDir) {
    QString settingsFile = AppContext::getSettings()->fileName();
    QString settingsDir = QFileInfo(settingsFile).absolutePath();
    if (settingsDir != colorsDir) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, colorsDir);
    }
}


static void setSchemaColors(const CustomColorSchema& customSchema){
    QString dirPath = getColorsDir();
    QDir dir(dirPath);
    if(!dir.exists()){
        dir.mkpath(dirPath);
    }

    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);

    std::auto_ptr<IOAdapter> io(factory->createIOAdapter());

    const QMap<char, QColor> & alphColors = customSchema.alpColors;
    const QString& file  = customSchema.name + NAME_FILTERS;
    DNAAlphabetType type = customSchema.type;
    QString keyword(type == DNAAlphabet_AMINO ? AMINO_KEYWORD : NUCL_KEYWORD);

    io->open(dir.filePath(file), IOAdapterMode_Write);
    // write header    
    QByteArray header;
    header.append(QString("%1\n").arg(keyword));
    io->writeBlock(header);
    // write body
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

static QMap<char, QColor> getDefaultSchemaColors(DNAAlphabetType type){
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    QMap<DNAAlphabetType, QByteArray > alphabetChars;
    foreach(DNAAlphabet* alphabet, alphabets){ // default initialization
        alphabetChars[alphabet->getType()] = alphabet->getAlphabetChars();        
    }
    QMapIterator<DNAAlphabetType, QByteArray > it(alphabetChars);
    QByteArray alphabet;
    while(it.hasNext()){
        it.next();
        if(it.key() == type){
            alphabet = it.value();
            break;            
        }
    }
    QMap<char, QColor> alphColors;
    for(int i = 0; i < alphabet.size(); ++i){
        alphColors[alphabet[i]] = QColor(Qt::white);
    }
    return alphColors;
}

static bool getSchemaColors(CustomColorSchema& customSchema){
    QMap<char, QColor> & alphColors =  customSchema.alpColors;
    const QString& file = customSchema.name + NAME_FILTERS;
    DNAAlphabetType& type = customSchema.type;

    QString dirPath = getColorsDir();
    QDir dir(dirPath);
    if(!dir.exists()){
        coreLog.info(QString("%1: no such directory").arg(dirPath));
        return false;
    }

    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);

    std::auto_ptr<IOAdapter> io(factory->createIOAdapter());
    if(!io->open(dir.filePath(file), IOAdapterMode_Read)){
        coreLog.info(QString("%1: no such schema").arg(customSchema.name));
        return false;
    }
    while(!io->isEof()){
        const int max_size = 1024;

        QByteArray byteLine(max_size + 1, 0);
        int lineLength = io->readLine(byteLine.data(), max_size);
        QString line(byteLine);
        line.remove(lineLength, line.size() - lineLength);
        if(line.isEmpty()){continue;}
        if(line == AMINO_KEYWORD){type = DNAAlphabet_AMINO;}
        else if(line == NUCL_KEYWORD){type = DNAAlphabet_NUCL;}
        else{
            coreLog.info(QString("%1: alphabet of schema not defined").arg(customSchema.name));
            return false;
        }
        alphColors = getDefaultSchemaColors(type);
        break;
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

        if(!lineValid(properties, alphColors )){
            coreLog.info(QString("%1: schema is not valid").arg(customSchema.name));
            return false;
        }
        tmpHelper[properties.first().at(0).toAscii()] = QColor(properties[1]); 
    }
    QMapIterator<char, QColor> it(tmpHelper);
    while(it.hasNext()){
        it.next();
        alphColors[it.key()] = it.value();
    }
    return true;
}



QList<CustomColorSchema> ColorSchemaSettingsUtils::getSchemas(){
    QList<CustomColorSchema> customSchemas;

    QDir dir(getColorsDir());
    if(!dir.exists()){
        return QList<CustomColorSchema>();
    }
    QStringList filters; 
    filters.append(QString("*%1").arg(NAME_FILTERS));

    QStringList schemaFiles = dir.entryList(filters);
    foreach(const QString& schemaName, schemaFiles){
        CustomColorSchema schema;
        schema.name = schemaName.split(".").first();
        bool ok = getSchemaColors(schema);
        if(!ok){continue;}
        customSchemas.append(schema);
    }
    return customSchemas;
}


ColorSchemaSettingsPageController::ColorSchemaSettingsPageController(QObject* p) 
: AppSettingsGUIPageController(tr("MSA color schema"), ColorSchemaSettingsPageId, p) {}


AppSettingsGUIPageState* ColorSchemaSettingsPageController::getSavedState() {
    ColorSchemaSettingsPageState* state = new ColorSchemaSettingsPageState();
    state->colorsDir = getColorsDir();       
    state->customSchemas = ColorSchemaSettingsUtils::getSchemas(); 
    return state;
}

void ColorSchemaSettingsPageController::saveState(AppSettingsGUIPageState* s) {
    ColorSchemaSettingsPageState* state = qobject_cast<ColorSchemaSettingsPageState*>(s);

    setColorsDir(state->colorsDir);
    foreach(const CustomColorSchema& schema, state->customSchemas){
        setSchemaColors(schema);
    }
    emit si_customSettingsChanged();
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
    connect(addSchemaButton, SIGNAL(clicked()), SLOT(sl_onAddColorSchema()));
}

void ColorSchemaSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    ColorSchemaSettingsPageState* state = qobject_cast<ColorSchemaSettingsPageState*>(s);
    colorsDirEdit->setText(state->colorsDir);
    customSchemas = state->customSchemas; 
    colorSchemas->clear();
    connect(schemaName, SIGNAL(textEdited ( const QString&)), SLOT(sl_schemaNameEdited(const QString&)));
    sl_schemaNameEdited(schemaName->text());
    
    foreach(const CustomColorSchema& customSchema, customSchemas){
        colorSchemas->addItem(new QListWidgetItem(customSchema.name, colorSchemas));           
    }
}

AppSettingsGUIPageState* ColorSchemaSettingsPageWidget::getState(QString& ) const {
    ColorSchemaSettingsPageState* state = new ColorSchemaSettingsPageState();
    state->colorsDir = colorsDirEdit->text();
    state->customSchemas = customSchemas;
    return state;
}

bool ColorSchemaSettingsPageWidget::isSchemaNameValid(const QString& text, QString& description){
    if(text.isEmpty()){
        description = "Name of schema is empty";
        return false;
    }
    for(int i = 0; i < text.length(); ++i){
        if(!text[i].isDigit() && !text[i].isLetter() && text[i] != QChar('_') && !text[i].isSpace()){
            description = "Name must consist of only from letter, digits, spaces and ""_"" simbols";
            return false;
        }        
    }
    if(isNameExist(text)){
        description = "Color schema with the same name already exist";
        return false;
    }
    return true;
}

bool ColorSchemaSettingsPageWidget::isNameExist(const QString& text){
    foreach(const CustomColorSchema& schema, customSchemas){
        if(schema.name == text){
            return true;
        }
    }
    return false;
}

void ColorSchemaSettingsPageWidget::sl_schemaNameEdited(const QString& text){
    QString description;
    if(isSchemaNameValid(text, description)){
            validLabel->setText("<font color='blue'>Ok!</font>");        
    }
    else{
        validLabel->setText("<font color='red'> Not valid: " + description + "</font>");
    }
}

void ColorSchemaSettingsPageWidget::sl_onColorsDirButton() {
    QString path = colorsDirEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), path,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        colorsDirEdit->setText(dir);
    }
}

void ColorSchemaSettingsPageWidget::sl_onAddColorSchema(){
    if(!isSchemaNameValid(schemaName->text(), QString())){return;}

    DNAAlphabetType type;

    if(aminoMode->isChecked()){
        type = DNAAlphabet_AMINO;
    }
    else if(nucleotideMode->isChecked()){
        type = DNAAlphabet_NUCL;
    }

    QMap<char, QColor> alpColors = getDefaultSchemaColors(type);

    ColorSchemaDialogController controller(alpColors);    
    int r = controller.adjustAlphabetColors();
    if(r == QDialog::Rejected){return;}

    CustomColorSchema schema;
    schema.name = schemaName->text() ;    
    schema.type = type;
    QMapIterator<char, QColor> it(alpColors);
    while(it.hasNext()){
        it.next();
        schema.alpColors[it.key()] = it.value();
    }
    customSchemas.append(schema);

    colorSchemas->addItem(new QListWidgetItem(schema.name, colorSchemas));
}

void ColorSchemaSettingsPageWidget::sl_onChangeColorSchema(){    
    QMap<char, QColor> alpColors;

    QListWidgetItem* item = colorSchemas->currentItem();
    if(item == NULL){return;}

    QString schemaName = item->text();
    for(int i = 0; i < customSchemas.size(); ++i){
        CustomColorSchema& customSchema = customSchemas[i];
        if(customSchema.name == schemaName){
            alpColors = customSchema.alpColors;
            ColorSchemaDialogController controller(alpColors);    
            int r = controller.adjustAlphabetColors();
            if(r == QDialog::Rejected){return;}

            QMapIterator<char, QColor> it(alpColors);
            while(it.hasNext()){
                it.next();
                customSchema.alpColors[it.key()] = it.value();
            }
            break;
        }
    }

    
}

} // namespase