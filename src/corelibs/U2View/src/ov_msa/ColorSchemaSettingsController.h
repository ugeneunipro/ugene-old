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

#ifndef _U2_COLOR_SCHEMA_SETTINGS_CONTROLLER_H_
#define _U2_COLOR_SCHEMA_SETTINGS_CONTROLLER_H_

#include <ui/ui_ColorSchemaSettingsWidget.h>
#include <ui/ui_CreateMSAScheme.h>

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

    /************************************************************************/
    /* Color Schema Settings Controller                                   */
    /************************************************************************/
#define ColorSchemaSettingsPageId     QString("ColorSchemaSettings")

    class CustomColorSchema{
    public:
        QString name;
        DNAAlphabetType type;
        bool defaultAlpType;
        QMap<char, QColor> alpColors;
    };

    class ColorSchemaSettingsUtils {
    public:
        static QList<CustomColorSchema> getSchemas();
    };

    class ColorSchemaSettingsPageController : public AppSettingsGUIPageController {
        Q_OBJECT
    public:
        ColorSchemaSettingsPageController(QObject* p = NULL);

        virtual AppSettingsGUIPageState* getSavedState();

        virtual void saveState(AppSettingsGUIPageState* s);

        virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
signals:
    void si_customSettingsChanged();
    };

    

    class ColorSchemaSettingsPageState : public AppSettingsGUIPageState {
        Q_OBJECT
    public:
        QString colorsDir;
        QList<CustomColorSchema> customSchemas;
    };

    class ColorSchemaSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_ColorSchemaSettingsWidget {
        Q_OBJECT
    public:
        ColorSchemaSettingsPageWidget(ColorSchemaSettingsPageController* ctrl);

        virtual void setState(AppSettingsGUIPageState* state);

        virtual AppSettingsGUIPageState* getState(QString& err) const;

        private slots:
            void sl_onColorsDirButton();
            void sl_onChangeColorSchema();
            void sl_onAddColorSchema();
            void sl_onDeleteColorSchema();
            void sl_schemaChanged(int);
    private:        
        QList<CustomColorSchema> customSchemas;
    };
    
    class CreateColorSchemaDialog: public QDialog, public Ui_CreateMSAScheme{
        Q_OBJECT
    public:
        CreateColorSchemaDialog(CustomColorSchema*, QStringList usedNames);
        int createNewScheme();
        private slots:
            void sl_createSchema();
            void sl_cancel();
            void sl_schemaNameEdited(const QString&);
            void sl_alphabetChanged(int);
    private:
        bool isNameExist(const QString&);
        bool isSchemaNameValid(const QString&, QString&);
    private:
        QStringList usedNames;
        CustomColorSchema* newSchema;
    };

} // U2

#endif //_U2_COLOR_SCHEMA_SETTINGS_CONTROLLER_H_