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

#ifndef _U2_GALAXY_CONFIG_TASK_H_
#define _U2_GALAXY_CONFIG_TASK_H_

#include <U2Core/Task.h>
#include <U2Lang/ActorContext.h>
#include <U2Lang/ActorPrototypeRegistry.h>

#include <QXmlStreamWriter>

namespace U2 {

class GalaxyConfigTask : public Task {
    Q_OBJECT
public:
    static const QString GALAXY_CONFIG_OPTION;

    GalaxyConfigTask();
    ~GalaxyConfigTask();

private:
    bool tryToFindInPath( const QString &objectName, QString &objectPath );
    bool tryToFindByLocate( const QString &objectName, QString &objectPath );
    bool fileExists( const QString &objectPath, const QString &suffix );
    bool findPathToObject( const QString &objectName, QString &objectPath );
    bool getPath( const QString &whatPath, QString &resultPath );
    bool getSchemeName();
    bool getGalaxyPath();

    bool getSchemeContent();
    bool getHelpMessage();
    bool getWorkflowName();

    bool getParameterValue( const QString &keyword, const int searchFrom, QString &parameterValue, int &nextSearchFrom );
    bool defineAliases();

    void writeToolUnit();

    ActorPrototype* getElementFromActorPrototypeRegistry( const QString &elementName );

    void pushAttributeTypeToPositionsList( const QString &elementAttribute, const int elementPosition );
    bool divideElementsByType();
    void writeSelectedElements( const QList <int> &elementsPositions );
    void writeAllElements();
    bool writeCommandUnit();

    void getConstraint( const QString &typeName, QString &resultType );
    void writeFormatAttribute( const QString &resultType );
    void writeLabelAttribute( const QStringList &elementParameters, const ActorPrototype *element );
    bool writeInputElements();
    bool isDelegateComboBox( PropertyDelegate *pd );
    bool isDelegateSpinBox( PropertyDelegate *pd );
    bool convertAttributeType( QString &attributeType, PropertyDelegate *pd );
    bool writeTypeForOptionElement( const QStringList &elementParameters, const ActorPrototype *element );
    void writeSelectAttribute( const PropertyDelegate *pd );
    void writeMinAndMaxAttributes( const PropertyDelegate *pd );
    bool writeOptionElements();
    bool writeInputsUnit();

    void writeFormatAttributeForOutputElement( const QString &resultType );
    bool checkDocumentFormatAttribute( const ActorPrototype *element );
    void writeChangeFormatAttribute( const QString &aliasName, const ActorPrototype *element );
    void tryToWriteChangeFormatAttribute( const ActorPrototype *element, QList <int> &usedOptionElements );
    bool writeOutputsUnit();

    void writeHelpUnit();

    bool createConfigForGalaxy();

    void doCopyCommands( const QString &pathToCopy );
    void doDeleteCommands();
    bool prepareDirectoryForTool();
    
    void makeCopyOfGalaxyToolConfig();
    bool addToolToConfig();
    bool writeNewSection( const QString &config );
    bool addToolToGalaxyConfig();

    QString appDirPath;
    QString schemeName;
    QString schemePath;
    QString ugenePath;
    QString galaxyPath;
    QString schemeContent;
    QString galaxyToolName;
    QString galaxyHelpMessage;


    QList < QMap < QString, QStringList > > elemAliases;
    QList <int> inputElementsPositions;
    QList <int> outputElementsPositions;
    QList <int> optionElementsPositions;
    QXmlStreamWriter galaxyConfigOutput;

};// GalaxyConfigTask

} // U2

#endif // _U2_GALAXY_CONFIG_TASK_H_
