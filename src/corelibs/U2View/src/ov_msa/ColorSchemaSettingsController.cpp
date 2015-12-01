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

#include <QColor>
#include <QDir>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/U2FileDialog.h>

#include "ColorSchemaDialogController.h"
#include "ColorSchemaSettingsController.h"

namespace U2 {

enum DefaultStrategy{
    DefaultStrategy_Void,
    DefaultStrategy_UgeneColors
};

static void clearColorsDir() {
    QString path = ColorSchemaSettingsUtils::getColorsDir();
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}

static void setSchemaColors(const CustomColorSchema& customSchema){
    QString dirPath = ColorSchemaSettingsUtils::getColorsDir();
    QDir dir(dirPath);
    if(!dir.exists()){
        dir.mkpath(dirPath);
    }

    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);

    QScopedPointer<IOAdapter> io(factory->createIOAdapter());

    const QMap<char, QColor> & alphColors = customSchema.alpColors;
    const QString& file  = customSchema.name + COLOR_SCHEME_NAME_FILTERS;
    DNAAlphabetType type = customSchema.type;
    bool defaultType = customSchema.defaultAlpType;

    QString keyword(type == DNAAlphabet_AMINO ? COLOR_SCHEME_AMINO_KEYWORD : (defaultType ? COLOR_SCHEME_NUCL_DEFAULT_KEYWORD : COLOR_SCHEME_NUCL_EXTENDED_KEYWORD));

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


const QString ColorSchemaSettingsPageController::helpPageId = QString("17465980");

ColorSchemaSettingsPageController::ColorSchemaSettingsPageController(MSAColorSchemeRegistry* mcsr, QObject* p)
: AppSettingsGUIPageController(tr("Alignment Color Scheme"), ColorSchemaSettingsPageId, p) {
       connect(this, SIGNAL(si_customSettingsChanged()), mcsr, SLOT(sl_onCustomSettingsChanged()));
}


AppSettingsGUIPageState* ColorSchemaSettingsPageController::getSavedState() {
    ColorSchemaSettingsPageState* state = new ColorSchemaSettingsPageState();
    state->colorsDir = ColorSchemaSettingsUtils::getColorsDir();
    state->customSchemas = ColorSchemaSettingsUtils::getSchemas();

    return state;
}

void ColorSchemaSettingsPageController::saveState(AppSettingsGUIPageState* s) {
    ColorSchemaSettingsPageState* state = qobject_cast<ColorSchemaSettingsPageState*>(s);

    ColorSchemaSettingsUtils::setColorsDir(state->colorsDir);
    clearColorsDir();
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

} // namespase
