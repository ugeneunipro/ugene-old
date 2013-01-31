/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>

#include <QFileDialog>
#include <QtCore/QDir>

#include <QtGui/QColor>

#include <memory>

#define SETTINGS_ROOT          QString("/color_schema_settings/")
#define SETTINGS_SUB_DIRECTORY QString("MSA_schemes")
#define COLOR_SCHEMA_DIR       QString("colors_scheme_dir")
#define AMINO_KEYWORD          QString("AMINO")
#define NUCL_KEYWORD           QString("NUCL")
#define NUCL_DEFAULT_KEYWORD   QString("NUCL_DEFAULT")
#define NUCL_EXTENDED_KEYWORD  QString("NUCL_EXTENDED")

#define NAME_FILTERS           QString(".csmsa") //WARNING if add more then one filter, change corresponding functions


namespace U2{


enum DefaultStrategy{
    DefaultStrategy_Void,
    DefaultStrategy_UgeneColors
};

static QString getColorsDir() {
    QString settingsFile = AppContext::getSettings()->fileName();
    QString settingsDir = QDir(QFileInfo(settingsFile).absolutePath()).filePath(SETTINGS_SUB_DIRECTORY); 
    
    QString res = AppContext::getSettings()->getValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, settingsDir, true).toString();

    return res;
}

static void setColorsDir(const QString &colorsDir) {
    QString settingsFile = AppContext::getSettings()->fileName();
    QString settingsDir = QFileInfo(settingsFile).absolutePath();
    if (settingsDir != colorsDir) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + COLOR_SCHEMA_DIR, colorsDir, true);
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
    bool defaultType = customSchema.defaultAlpType;

    QString keyword(type == DNAAlphabet_AMINO ? AMINO_KEYWORD : (defaultType ? NUCL_DEFAULT_KEYWORD : NUCL_EXTENDED_KEYWORD));

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

static QByteArray uniteAlphabetChars(const QByteArray& firstAlphabetChars, const QByteArray& secondAlphabetChars){
    QByteArray unitedAlphabetChars = firstAlphabetChars;
    for(int i = 0; i < secondAlphabetChars.size(); ++i){
        if(!unitedAlphabetChars.contains(secondAlphabetChars[i])){
            unitedAlphabetChars.append(secondAlphabetChars[i]);
        }
    }
    qSort(unitedAlphabetChars.begin(), unitedAlphabetChars.end());
    return unitedAlphabetChars;
}

static void getDefaultUgeneColors(DNAAlphabetType type, QMap<char, QColor>& alphColors){
    if(type == DNAAlphabet_AMINO){
        alphColors['I'] = "#ff0000"; 
        alphColors['V'] = "#f60009"; 
        alphColors['L'] = "#ea0015"; 
        alphColors['F'] = "#cb0034"; 
        alphColors['C'] = "#c2003d"; 
        alphColors['M'] = "#b0004f"; 
        alphColors['A'] = "#ad0052"; 
        alphColors['G'] = "#6a0095"; 
        alphColors['X'] = "#680097"; 
        alphColors['T'] = "#61009e"; 
        alphColors['S'] = "#5e00a1"; 
        alphColors['W'] = "#5b00a4"; 
        alphColors['Y'] = "#4f00b0"; 
        alphColors['P'] = "#4600b9"; 
        alphColors['H'] = "#1500ea"; 
        alphColors['E'] = "#0c00f3"; 
        alphColors['Z'] = "#0c00f3"; 
        alphColors['Q'] = "#0c00f3"; 
        alphColors['D'] = "#0c00f3"; 
        alphColors['B'] = "#0c00f3"; 
        alphColors['N'] = "#0c00f3"; 
        alphColors['K'] = "#0000ff"; 
        alphColors['R'] = "#0000ff";
    }
    else if(type == DNAAlphabet_NUCL){
        alphColors['A'] = "#FCFF92"; // yellow
        alphColors['C'] = "#70F970"; // green
        alphColors['T'] = "#FF99B1"; // light red
        alphColors['G'] = "#4EADE1"; // light blue
        alphColors['U'] = alphColors['T'].lighter(120);
        alphColors['N'] = "#FCFCFC";
    }
}

static QMap<char, QColor> getDefaultSchemaColors(DNAAlphabetType type, bool defaultAlpType, DefaultStrategy strategy){
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    QMap<DNAAlphabetType, QByteArray > alphabetChars;
    foreach(DNAAlphabet* alphabet, alphabets){ // default initialization
        if(defaultAlpType == alphabet->isDefault()){
            alphabetChars[alphabet->getType()] = uniteAlphabetChars(alphabetChars.value(alphabet->getType()) ,alphabet->getAlphabetChars());
        }
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
    getDefaultUgeneColors(type, alphColors);
    return alphColors;
}

static bool getSchemaColors(CustomColorSchema& customSchema){
    QMap<char, QColor> & alphColors =  customSchema.alpColors;
    const QString& file = customSchema.name + NAME_FILTERS;
    DNAAlphabetType& type = customSchema.type;
    bool& defaultAlpType = customSchema.defaultAlpType = true;

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
        else if(line.contains(NUCL_KEYWORD)){
            type = DNAAlphabet_NUCL;
            if(line == NUCL_DEFAULT_KEYWORD){
                defaultAlpType = true;
            }
            else if(line == NUCL_EXTENDED_KEYWORD){
                defaultAlpType = false;
            }
            else{
                coreLog.info(QString("%1: mode of nucleic alphabet of schema not defined, use default mode").arg(customSchema.name));
            }
        }
        else{
            coreLog.info(QString("%1: alphabet of schema not defined").arg(customSchema.name));
            return false;
        }
        alphColors = getDefaultSchemaColors(type, defaultAlpType, DefaultStrategy_Void);
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
: AppSettingsGUIPageController(tr("Alignment color scheme"), ColorSchemaSettingsPageId, p) {}


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
    connect(deleteSchemaButton, SIGNAL(clicked()), SLOT(sl_onDeleteColorSchema()));
    connect(colorSchemas, SIGNAL(currentRowChanged(int)), SLOT(sl_schemaChanged(int)));

    sl_schemaChanged(colorSchemas->currentRow());
}

void ColorSchemaSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    ColorSchemaSettingsPageState* state = qobject_cast<ColorSchemaSettingsPageState*>(s);
    colorsDirEdit->setText(state->colorsDir);
    customSchemas = state->customSchemas; 
    colorSchemas->clear();    
    
    foreach(const CustomColorSchema& customSchema, customSchemas){
        colorSchemas->addItem(new QListWidgetItem(customSchema.name, colorSchemas));           
    }
}

AppSettingsGUIPageState* ColorSchemaSettingsPageWidget::getState(QString& ) const {
    ColorSchemaSettingsPageState* state = new ColorSchemaSettingsPageState();
    state->colorsDir = colorsDirEdit->text();
    state->customSchemas = customSchemas;
    int sz = customSchemas.size();
    return state;
}

void ColorSchemaSettingsPageWidget::sl_schemaChanged(int index){
    if(index < 0 || index >= colorSchemas->count()){
        changeSchemaButton->setDisabled(true);
        deleteSchemaButton->setDisabled(true);
    }else{
        changeSchemaButton->setEnabled(true);
        deleteSchemaButton->setEnabled(true);
    }
}


void ColorSchemaSettingsPageWidget::sl_onColorsDirButton() {
    QString path = colorsDirEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), path,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        colorsDirEdit->setText(dir);
        setColorsDir(dir);
        customSchemas.clear();
        colorSchemas->clear();
        customSchemas = ColorSchemaSettingsUtils::getSchemas();
        foreach(CustomColorSchema schema, customSchemas){
            colorSchemas->addItem(new QListWidgetItem(schema.name, colorSchemas));
        }
    }
}

void ColorSchemaSettingsPageWidget::sl_onAddColorSchema(){
    
    QStringList usedNames;
    foreach(const CustomColorSchema& customScheme, customSchemas){
        usedNames << customScheme.name;
    }
    CustomColorSchema schema;

    CreateColorSchemaDialog d(&schema, usedNames);
    int r = d.createNewScheme();

    if(r == QDialog::Rejected){return;}
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

void ColorSchemaSettingsPageWidget::sl_onDeleteColorSchema(){
    QListWidgetItem* item = colorSchemas->currentItem();
    SAFE_POINT(item != NULL, "current item for deletion is NULL",);

    int size1 = customSchemas.size(), size2 = colorSchemas->count();

    QString schemaName = item->text();
    for(int i = 0; i < customSchemas.size(); ++i){
        CustomColorSchema& customSchema = customSchemas[i];
        if(customSchema.name == schemaName){
            QFile::remove(getColorsDir() + QDir::separator() + schemaName + NAME_FILTERS);
            customSchemas.removeAt(i);
            colorSchemas->removeItemWidget(item);
            delete item;
            return;            
        }
    }
    SAFE_POINT(true != true, "something wrong causes color scheme deletion, this code must be unreacheble",);
}

/*Create MSA scheme dialog*/

CreateColorSchemaDialog::CreateColorSchemaDialog(CustomColorSchema* _newSchema, QStringList _usedNames) : newSchema(_newSchema), usedNames(_usedNames){
    setupUi(this);

    alphabetComboBox->insertItem(0, QString(tr("Amino acid")), DNAAlphabet_AMINO);
    alphabetComboBox->insertItem(1, QString(tr("Nucleotide")), DNAAlphabet_NUCL);

    connect(alphabetComboBox, SIGNAL(currentIndexChanged(int)), SLOT(sl_alphabetChanged(int)));
    extendedModeBox->setVisible(false);

    connect(schemeName, SIGNAL(textEdited ( const QString&)), SLOT(sl_schemaNameEdited(const QString&)));
    connect(createButton, SIGNAL(clicked()), SLOT(sl_createSchema()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(sl_cancel()));

    QSet<QString > excluded;
    foreach(const QString& usedName, usedNames){
        excluded.insert(usedName);
    }
    schemeName->setText(GUrlUtils::rollFileName("Custom color schema", excluded));
}

bool CreateColorSchemaDialog::isNameExist(const QString& text){
    foreach(const QString& usedName, usedNames){
        if(usedName == text){
            return true;
        }
    }
    return false;
}

bool CreateColorSchemaDialog::isSchemaNameValid(const QString& text, QString& description){
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

void CreateColorSchemaDialog::sl_schemaNameEdited(const QString& text){
    QString description;
    if(isSchemaNameValid(text, description)){
        validLabel->clear();
    }
    else{
        validLabel->setText("<font color='red'> Not valid: " + description + "</font>");
    }
}

void CreateColorSchemaDialog::sl_alphabetChanged(int index){
    if (index < 0 || index >= alphabetComboBox->count()){return;}
        
    if(static_cast<DNAAlphabetType>(alphabetComboBox->itemData(index).toInt()) == DNAAlphabet_AMINO){
        extendedModeBox->setVisible(false);
        extendedModeBox->setChecked(false);
    }
    else{
        extendedModeBox->setVisible(true);
    }
}

int CreateColorSchemaDialog::createNewScheme(){
    return exec();
}

void CreateColorSchemaDialog::sl_createSchema(){
    QString description;
    if(!isSchemaNameValid(schemeName->text(), description)){return;}

    int index = alphabetComboBox->currentIndex();
    if(index < 0 || index >= alphabetComboBox->count()){return;}

    DNAAlphabetType type;
    bool defaultAlpType = true;

    if(static_cast<DNAAlphabetType>(alphabetComboBox->itemData(index).toInt()) == DNAAlphabet_AMINO){
        type = DNAAlphabet_AMINO;
    }
    else if(static_cast<DNAAlphabetType>(alphabetComboBox->itemData(index).toInt()) == DNAAlphabet_NUCL){
        type = DNAAlphabet_NUCL;
        if(extendedModeBox->isChecked()){
            defaultAlpType = false;
        }
    }

    QMap<char, QColor> alpColors = getDefaultSchemaColors(type, defaultAlpType, DefaultStrategy_UgeneColors);

    ColorSchemaDialogController controller(alpColors);    
    int r = controller.adjustAlphabetColors();
    if(r == QDialog::Rejected){return;}
    
    newSchema->name = schemeName->text() ;    
    newSchema->type = type;
    newSchema->defaultAlpType = defaultAlpType;

    QMapIterator<char, QColor> it(alpColors);
    while(it.hasNext()){
        it.next();
        newSchema->alpColors[it.key()] = it.value();
    }
    accept();
}

void CreateColorSchemaDialog::sl_cancel(){
    reject();
}


} // namespase