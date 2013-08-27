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

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineUtils.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentImport.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Designer/DelegateEditors.h>

#include <U2Core/U2SafePoints.h>

#include "GalaxyConfigTask.h"

namespace U2 {

/*******************************************
* GalaxyConfigTask
*******************************************/

#define  GALAXY_PATH_OPTION      "galaxy-path"
#define  TOOL                    "tool"
#define  NAME                    "name"
#define  ID                      "id"
#define  COMMAND                 "command"
#define  INPUTS                  "inputs"
#define  OUTPUTS                 "outputs"
#define  PARAM                   "param"
#define  FORMAT                  "format"
#define  OPTION                  "option"
#define  DATA                    "data"
#define  CHANGE_FORMAT           "change-format"
#define  WHEN                    "when"
#define  HELP                    "help"
#define  WORKFLOW_RUN_LOG        "ugene_workflow_run_log"

static int SUBSTRING_NOT_FOUND = -1;

const QString GalaxyConfigTask::GALAXY_CONFIG_OPTION = "galaxy-config";

GalaxyConfigTask::GalaxyConfigTask() 
:Task( tr( "Create Galaxy config from existing workflow scheme" ), TaskFlag_None ),  appDirPath(QString()), schemeName(QString()),
                                                                                     schemePath(QString()), ugenePath(QString()),
                                                                                     galaxyPath(QString()), schemeContent(QString()),
                                                                                     galaxyToolName(QString()), galaxyHelpMessage(QString())
{
    elemAliases.clear();
    inputElementsPositions.clear();
    outputElementsPositions.clear();
    optionElementsPositions.clear();

    appDirPath = QApplication::applicationDirPath() + "/";

    CHECK_OPERATIONS( !getSchemeName(),           coreLog.error( stateInfo.getError() + "\nScheme name is incorrect" ),                                         return; );
    CHECK_OPERATIONS( !getGalaxyPath(),           coreLog.error( stateInfo.getError() + "\nGalaxy directory is not found" ),                                    return; );
    CHECK_OPERATIONS( !getSchemeContent(),        coreLog.error( stateInfo.getError() + "\nCan not get scheme content" ),                                       return; );
    CHECK_OPERATIONS( !getHelpMessage(),          coreLog.error( stateInfo.getError() + "\nCan not get scheme description" ),                                   return; );
    CHECK_OPERATIONS( !getWorkflowName(),         coreLog.error( stateInfo.getError() + "\nCan not get scheme name" ),                                          return; );
    CHECK_OPERATIONS( !defineAliases(),           coreLog.error( stateInfo.getError() + "\nCan not get scheme aliases" ),                                       return; );
    CHECK_OPERATIONS( !createConfigForGalaxy(),   coreLog.error( stateInfo.getError() + "\nScheme Config for Galaxy is not created" ),                          return; );
    CHECK_OPERATIONS( !prepareDirectoryForTool(), coreLog.error( stateInfo.getError() + "\nCan not create directory for Galaxy tool. Check user privileges" ),  return; );
    CHECK_OPERATIONS( !addToolToGalaxyConfig(),   coreLog.error( stateInfo.getError() + "\nCan not add new section to tool_conf.xml. Check user privileges" ),  return; );

}

GalaxyConfigTask::~GalaxyConfigTask() {

}

bool GalaxyConfigTask::tryToFindInPath( const QString &objectName, QString &objectPath ) {
    QString pathVariable = qgetenv("PATH").constData();
    const int objectNamePosition = pathVariable.indexOf( objectName );
    if( objectNamePosition == SUBSTRING_NOT_FOUND ) {
        coreLog.info( "Path to " + objectName + " directory is not found in PATH variable" );
        return false;
    }
    int currPos;
    for( currPos = objectNamePosition; currPos >= 0; currPos-- ) {
        if( QString(pathVariable[currPos]) == HRSchemaSerializer::COLON ) {
            break;
        }
    }
    const int pathStartPosition = currPos + 1,
              pathEndPosition = pathVariable.indexOf( HRSchemaSerializer::COLON, objectNamePosition ),
              pathLength = pathEndPosition - pathStartPosition;
    objectPath = pathVariable.mid( pathStartPosition, pathLength );
    if( !objectPath.endsWith( "/" ) ) {
        objectPath.append("/");
    }
    return true;
}

bool GalaxyConfigTask::tryToFindByLocate( const QString &objectName, QString &objectPath ) {
    system( QString("locate " + objectName + " -l 1 > " + objectName + "_path.txt").toLocal8Bit().constData() );
    QFile file( objectName + "_path.txt" );
    if( !file.open(QIODevice::ReadOnly ) ) {
        coreLog.info( "Can not read " + objectName + "_path.txt file to find " + objectName + " directory. Check user privileges" );
        return false;
    }
    QTextStream inFile(&file);
    inFile >> objectPath;
    file.close();
    system( QString("rm " + objectName + "_path.txt").toLocal8Bit().constData() );
    if( !objectPath.length() ) {
        coreLog.info( "Path to " + objectName + " directory is not found by \"locate\" command" );
        return false;
    }
    if( !objectPath.endsWith( "/" ) ) {
        objectPath.append("/");
    }
    return true;
}

bool GalaxyConfigTask::fileExists( const QString &objectPath, const QString &suffix ) {
    const QString fullPath = objectPath + suffix;
    QFile file( fullPath );
    if( file.exists() ) {
        return true;
    }
    stateInfo.setError( "Directory is incorrect. Check it contains " + suffix );
    return false;
}

bool GalaxyConfigTask::findPathToObject( const QString &objectName, QString &objectPath ) {
    const QString suffix = "tool_conf.xml";
    if( tryToFindInPath( objectName, objectPath ) && fileExists( objectPath, suffix ) ) {
        return true;
    }
    if( tryToFindByLocate( objectName, objectPath ) && fileExists( objectPath, suffix ) ) {
        return true;
    }
    return false;
}

bool GalaxyConfigTask::getPath( const QString &whatPath, QString &resultPath ) {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );

    const int result = CMDLineRegistryUtils::getParameterIndex(whatPath);
    const QStringList paramValues = CMDLineRegistryUtils::getParameterValues(whatPath);
    if( result == -1 || !paramValues.first().length() ) {
        return findPathToObject("galaxy", galaxyPath);
    }
    resultPath = paramValues.first();
    if( !resultPath.endsWith("/") ) {
        resultPath.append("/");
    }
    return true;
}

bool GalaxyConfigTask::getGalaxyPath() {
    return getPath( GALAXY_PATH_OPTION, galaxyPath );
}

bool GalaxyConfigTask::getSchemeName() {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );

    const QStringList paramValues = CMDLineRegistryUtils::getParameterValues( GALAXY_CONFIG_OPTION );
    if( !paramValues.first().length() || !QFile::exists( paramValues.first() ) ) {
        stateInfo.setError( "Scheme file is incorrect. Check it exists" );
        return false;
    }
    schemePath = paramValues.first();
    schemeName = schemePath.split("/").last();
    return true;
}

bool GalaxyConfigTask::getSchemeContent() {
    QFile schemeFile( schemePath );
    if( !schemeFile.open(QIODevice::ReadOnly ) ) {
        stateInfo.setError( "Can not read " + schemePath + " file. Check user privileges " );
        return false;
    }
    QTextStream input(&schemeFile);
    schemeContent = input.readAll();
    schemeFile.close();
    return true;
}

bool GalaxyConfigTask::getHelpMessage() {
    galaxyHelpMessage = "\r\n**Description**\r\n";
    int commentStartPosition = schemeContent.indexOf( HRSchemaSerializer::HEADER_LINE );
    if( commentStartPosition == SUBSTRING_NOT_FOUND ) {
        stateInfo.setError( "Scheme is corrupted. It does not contain " + HRSchemaSerializer::HEADER_LINE + " keyword" );
        return false;
    }
    commentStartPosition += HRSchemaSerializer::HEADER_LINE.length();
    const int commentEndPosition = schemeContent.lastIndexOf( HRSchemaSerializer::BODY_START );
    if( commentEndPosition == SUBSTRING_NOT_FOUND ) {
        stateInfo.setError( "Scheme is corrupted. It does not contain " + HRSchemaSerializer::BODY_START + " keyword" );
        return false;
    }
    const int commentLength = commentEndPosition - commentStartPosition;
    QString comment = QString();
    comment = schemeContent.mid( commentStartPosition, commentLength );
    comment.replace( HRSchemaSerializer::SERVICE_SYM, "\r\n" );
    galaxyHelpMessage += comment;
    return true;
}

bool GalaxyConfigTask::getWorkflowName() {
    int nameStartPosition = schemeContent.lastIndexOf( HRSchemaSerializer::BODY_START );
    nameStartPosition += HRSchemaSerializer::BODY_START.length() + 1;
    const int nameEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_START, nameStartPosition );
    CHECK_OPERATIONS( nameEndPosition != SUBSTRING_NOT_FOUND, 
                      stateInfo.setError( "Scheme is corrupted. It does not contain start of block" ),
                      return false;
    );
    const int nameLength = nameEndPosition - nameStartPosition;
    galaxyToolName = schemeContent.mid( nameStartPosition, nameLength );
    galaxyToolName.replace( QRegExp("^\""),"" );
    galaxyToolName.replace( QRegExp("\"$"),"" );
    return true;
}

bool GalaxyConfigTask::getParameterValue( const QString &keyword, const int searchFrom, QString &parameterValue, int &nextSearchFrom ) {
    const int keywordPosition = schemeContent.indexOf( keyword, searchFrom );
    const int blockEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_END, searchFrom );
    if( keyword == HRSchemaSerializer::DESCRIPTION && ( keywordPosition == - 1 || blockEndPosition < keywordPosition ) ) {
        nextSearchFrom = searchFrom;
        return true;
    }
    const int parameterStartPosition = schemeContent.indexOf( HRSchemaSerializer::COLON, keywordPosition ) + 1,
              parameterEndPosition = schemeContent.indexOf( HRSchemaSerializer::SEMICOLON, parameterStartPosition );
    CHECK_OPERATIONS( parameterStartPosition != SUBSTRING_NOT_FOUND,
                      stateInfo.setError( "Scheme is corrupted. Begin of alias value is not found" ),
                      return false;
                    );
    CHECK_OPERATIONS( parameterEndPosition != SUBSTRING_NOT_FOUND,
                      stateInfo.setError( "Scheme is corrupted. End of alias value is not found" ),
                      return false;
                    );
    const int parameterLength = parameterEndPosition - parameterStartPosition;
    parameterValue = schemeContent.mid( parameterStartPosition, parameterLength );
    nextSearchFrom = parameterEndPosition;
    return true;
}

bool GalaxyConfigTask::defineAliases() {
    int aliasesStartPosition = schemeContent.indexOf( HRSchemaSerializer::PARAM_ALIASES_START );
    CHECK_OPERATIONS( aliasesStartPosition != SUBSTRING_NOT_FOUND, 
                      stateInfo.setError( "Scheme is corrupted. It does not contain " + HRSchemaSerializer::PARAM_ALIASES_START + " keyword" ),
                      return false;
                    );
    aliasesStartPosition += HRSchemaSerializer::PARAM_ALIASES_START.length();
    const int visualKeywordPosition = schemeContent.indexOf( HRSchemaSerializer::VISUAL_START, aliasesStartPosition );
    CHECK_OPERATIONS( visualKeywordPosition != SUBSTRING_NOT_FOUND, 
                      stateInfo.setError( "Scheme is corrupted. It does not contain " + HRSchemaSerializer::VISUAL_START + " keyword" ),
                      return false;
                    );
    int elementNameStartPosition = schemeContent.indexOf( QRegExp("[a-z]"), aliasesStartPosition );
    while ( elementNameStartPosition < visualKeywordPosition ) {
        const int elementNameEndPosition = schemeContent.indexOf( HRSchemaSerializer::DOT, elementNameStartPosition );
        CHECK_OPERATIONS( elementNameEndPosition != SUBSTRING_NOT_FOUND, 
                          stateInfo.setError( "Scheme contains wrong alias" ),
                          return false;
                        );
        const int elementNameLength = elementNameEndPosition - elementNameStartPosition;
        QString elementName = schemeContent.mid( elementNameStartPosition, elementNameLength );
        elementName.replace(QRegExp("[0-9]$"),"");

        const int elementAliasStartPosition = elementNameEndPosition + 1,
                  elementAliasEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_START, elementAliasStartPosition );
        const int elementAliasLength = elementAliasEndPosition - elementAliasStartPosition;
        QString elementAlias = schemeContent.mid( elementAliasStartPosition, elementAliasLength );
        elementAlias.replace(" ","");

        QString aliasName, aliasDescription;
        int aliasNameEndPosition, aliasDescriptionEndPosition;

        if( !getParameterValue( HRSchemaSerializer::ALIAS, elementAliasEndPosition, aliasName, aliasNameEndPosition ) ) {
            return false;
        }
        if( !getParameterValue( HRSchemaSerializer::DESCRIPTION, aliasNameEndPosition, aliasDescription, aliasDescriptionEndPosition ) ) {
            return false;
        }

        QMap< QString, QStringList > elementProperties;
        QStringList elementAliasParameters;
        elementAliasParameters << elementAlias << aliasName << aliasDescription;
        elementProperties[elementName] = elementAliasParameters;
        elemAliases.push_back(elementProperties);

        elementNameStartPosition = schemeContent.indexOf( QRegExp("[a-z]"), aliasDescriptionEndPosition );
    }
    return true;
}

void GalaxyConfigTask::writeToolUnit() {
    galaxyConfigOutput.writeStartElement( TOOL );  //tool unit begin
    galaxyConfigOutput.writeAttribute( ID, galaxyToolName );
    QString idStrCopy = galaxyToolName;
    idStrCopy.replace(" ", "_");
    galaxyConfigOutput.writeAttribute( NAME, idStrCopy );
}

ActorPrototype* GalaxyConfigTask::getElementFromActorPrototypeRegistry( const QString &elementName ) {
    U2::Workflow::ActorPrototypeRegistry *prototypeRegistry = 
                                          U2::Workflow::WorkflowEnv::getProtoRegistry();
    assert( NULL != prototypeRegistry );
    return prototypeRegistry->getProto( elementName );
}

void GalaxyConfigTask::pushAttributeTypeToPositionsList( const QString &elementAttribute, const int elementPosition ) {
    if( BaseAttributes::URL_IN_ATTRIBUTE().getId() == elementAttribute ) {
        inputElementsPositions.push_back( elementPosition );
    } else if ( BaseAttributes::URL_OUT_ATTRIBUTE().getId() == elementAttribute ){
        outputElementsPositions.push_back( elementPosition );
    } else {
        optionElementsPositions.push_back( elementPosition );
    }
}

bool GalaxyConfigTask::divideElementsByType() {
    QList < QMap < QString, QStringList > > ::iterator elemAliasesIterator;
    elemAliasesIterator = elemAliases.begin();
    while( elemAliasesIterator != elemAliases.end() ) {
        const QMap < QString, QStringList > ::iterator elementProperties = (*elemAliasesIterator).begin();
        const QString elementName = elementProperties.key();
        const QString attributeName = elementProperties.value().at(0);

        ActorPrototype *currElement = getElementFromActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        const QList < Attribute* > elementAttributes = currElement->getAttributes();
        if( elementAttributes.isEmpty() ) {
            stateInfo.setError( "Can not get attributes from \"" + currElement->getId() + "\" element" );
            return false;
        }

        foreach( Attribute *elementAttribute, elementAttributes ) {
            if( elementAttribute->getId() == attributeName ) {
                pushAttributeTypeToPositionsList( attributeName, std::distance( elemAliases.begin(), elemAliasesIterator ) );
            }
        }
        elemAliasesIterator++;
    }
    return false;
}

bool GalaxyConfigTask::writeCommandUnit() {
    galaxyConfigOutput.writeStartElement ( COMMAND ); //command unit begin 
    CHECK( divideElementsByType(), false );

    QString ugeneExecutable;
#ifdef QT_DEBUG
    ugeneExecutable = "ugened";
#else
    ugeneExecutable = "ugene";
#endif

    QString runUgene = appDirPath + ugeneExecutable + " --task=" + schemePath + " ";
    QList < QMap < QString, QStringList > > ::iterator elemAliasesIterator;
    elemAliasesIterator = elemAliases.begin();
    while( elemAliasesIterator != elemAliases.end() ) {
        QMap <QString, QStringList> ::iterator elementParameters = (*elemAliasesIterator).begin();
        QString aliasName = elementParameters.value().at(1);
        runUgene += "--" + aliasName + "=$" + aliasName + " ";
        elemAliasesIterator++;
    }
    const QString redirectStderrToStdout = " 2>&1;";
    runUgene += QString("  >> $") + WORKFLOW_RUN_LOG + redirectStderrToStdout;
    galaxyConfigOutput.writeCharacters( runUgene );

    QList <int> ::iterator outputElementsPositionsIterator;
    outputElementsPositionsIterator = outputElementsPositions.begin();
    while( outputElementsPositionsIterator != outputElementsPositions.end() ) {
        QMap <QString, QStringList> ::iterator elementParameters = elemAliases[*outputElementsPositionsIterator].begin();
        const QString aliasName = elementParameters.value().at(1);
        galaxyConfigOutput.writeDTD( QString("\r\nif [ ! -s $" + aliasName + " ]; then") );
        galaxyConfigOutput.writeDTD( QString("\r\necho \"EMPTY RESULT FILE\" > $" + aliasName + ";") );
        galaxyConfigOutput.writeDTD( QString("\r\nfi;\r\n") );
        outputElementsPositionsIterator++;
    }
    galaxyConfigOutput.writeEndElement();            //command unit end
    return true;
}

void GalaxyConfigTask::getConstraint( const QString &typeName, QString &resultType ) {
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::TEXT ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::TEXT ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::TEXT ).type;
    } 
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::SEQUENCE ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::SEQUENCE ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::SEQUENCE ).type;
    } 
    else
    if( typeName.contains(GObjectTypes::getTypeInfo( GObjectTypes::ANNOTATION_TABLE ).name, Qt::CaseInsensitive ) ||
        typeName.contains(GObjectTypes::getTypeInfo( GObjectTypes::ANNOTATION_TABLE ).pluralName, Qt::CaseInsensitive ) )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::ANNOTATION_TABLE ).type;
    }
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::VARIANT_TRACK ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::VARIANT_TRACK ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::VARIANT_TRACK ).type;
    }
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::CHROMATOGRAM ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::CHROMATOGRAM ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::CHROMATOGRAM ).type;
    }
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::MULTIPLE_ALIGNMENT ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::MULTIPLE_ALIGNMENT ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::MULTIPLE_ALIGNMENT ).type;
    }
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::PHYLOGENETIC_TREE ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::PHYLOGENETIC_TREE ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::PHYLOGENETIC_TREE ).type;
    }
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::BIOSTRUCTURE_3D ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::BIOSTRUCTURE_3D ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::BIOSTRUCTURE_3D ).type;
    }
    else
    if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::ASSEMBLY ).name ||
        typeName == GObjectTypes::getTypeInfo( GObjectTypes::ASSEMBLY ).pluralName )
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::ASSEMBLY ).type;
    }
    else
    {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::UNKNOWN ).type;
    }
}

void GalaxyConfigTask::writeFormatAttribute( const QString &resultType ) {
    DocumentFormatRegistry *docFormatRegistry = AppContext::getDocumentFormatRegistry();
    assert( NULL != docFormatRegistry );

    DocumentFormatConstraints constraint;
    constraint.supportedObjectTypes.insert( resultType );
    QList <QString> selectedFormats = docFormatRegistry->selectFormats( constraint );

    QString resultFormatString = QString(); 
    QList <QString> ::iterator selectedFormatsIterator;
    selectedFormatsIterator = selectedFormats.begin();
    while( selectedFormatsIterator != selectedFormats.end() - 1 ) {
        resultFormatString += *selectedFormatsIterator;
        resultFormatString += HRSchemaSerializer::COMMA;
        selectedFormatsIterator++;
    }
    resultFormatString += (*selectedFormatsIterator);
    galaxyConfigOutput.writeAttribute( FORMAT, resultFormatString );
}

void GalaxyConfigTask::writeLabelAttribute( const QStringList &elementParameters, const ActorPrototype *element ) {
    const QString attributeName = elementParameters.at(0);
    QString aliasDescription = elementParameters.at(2);
    QString copyStr = aliasDescription;
    if( aliasDescription.length() == 0 || (!copyStr.contains("[a-zA-Z0-9]") ) ) {
        aliasDescription.clear();
        aliasDescription += element->getDisplayName();
        aliasDescription += ".";
        aliasDescription += element->getAttribute( attributeName )->getDocumentation();
    }
    aliasDescription = aliasDescription.trimmed();
    if( (QString)aliasDescription[0] == HRSchemaSerializer::QUOTE ) {
        aliasDescription.remove( 0, 1 );
    }
    if( (QString)aliasDescription[ aliasDescription.length() - 1 ] == HRSchemaSerializer::QUOTE ) {
        aliasDescription.remove( aliasDescription.length() - 1, 1 );
    }
    galaxyConfigOutput.writeAttribute( "label", aliasDescription );
}

bool GalaxyConfigTask::writeInputElements() {
    QList <int> ::iterator inputElementsIterator;
    inputElementsIterator = inputElementsPositions.begin();
    while( inputElementsIterator != inputElementsPositions.end() ) {
        galaxyConfigOutput.writeStartElement( PARAM );
        const QMap < QString, QStringList > currAlias = elemAliases[ *inputElementsIterator ];
        QMap < QString, QStringList > ::const_iterator currAliasIterator = currAlias.begin();
        const QString elementName = (currAliasIterator).key(),
                      aliasName = (currAliasIterator).value().at(1);

        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, aliasName );
        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::TYPE_ATTR, DATA );

        ActorPrototype *currElement = getElementFromActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        const QString formatType = currElement->getPortDesciptors().first()->getDisplayName();
        QString resultType = QString();
        getConstraint( formatType, resultType );
        if ( resultType == GObjectTypes::UNKNOWN ) {
            stateInfo.setError( "Element \"" + currElement->getId() + "\" has unknown type" );
            return false;
        }
        writeFormatAttribute( resultType );
        writeLabelAttribute( (currAliasIterator).value(), currElement );
        galaxyConfigOutput.writeEndElement();
        inputElementsIterator++;
    }
    return true;
}

bool GalaxyConfigTask::isDelegateComboBox( PropertyDelegate *pd ) {
    ComboBoxDelegate *comboBoxDelegate = dynamic_cast <ComboBoxDelegate *> (pd);
    if( comboBoxDelegate != NULL ) {
        return true;
    }
    return false;
}

bool GalaxyConfigTask::isDelegateSpinBox( PropertyDelegate *pd ) {
    SpinBoxDelegate *spinBoxDelegate = dynamic_cast <SpinBoxDelegate *> (pd);
    if( spinBoxDelegate != NULL ) {
        return true;
    }
    return false;
}

bool GalaxyConfigTask::convertAttributeType( QString &attributeType, PropertyDelegate *pd ) {
    if( isDelegateComboBox(pd) && pd != NULL ) {
        attributeType = "select";
    } else if ( isDelegateSpinBox(pd) && pd != NULL && attributeType == BaseTypes::NUM_TYPE()->getId() ) {
        attributeType = "integer";
        QVariantMap items;
        pd->getItems( items );
        const QString typeName1 = items.value("minimum").typeName();
        const QString typeName2 = items.value("minimum").typeName();
        if( typeName1 == "double" || typeName2 == "double" ) {
            attributeType = "float";
        }
    } else if( attributeType == BaseTypes::BOOL_TYPE()->getId() ) {
        attributeType = "boolean";
    } else if( attributeType == BaseTypes::STRING_TYPE()->getId() ) {
        attributeType = "text";
    } else {
        stateInfo.setError( "Unknown attribute type: " + attributeType );
        return false; 
    }
    return true;
}

bool GalaxyConfigTask::writeTypeForOptionElement( const QStringList &elementParameters, const ActorPrototype *element ) {
    const QString attributeName = elementParameters.at(0);
    Attribute *elementAttribute = element->getAttribute(attributeName);
    assert( elementAttribute != NULL );
    QString attributeType = elementAttribute->getAttributeType()->getId();

    ConfigurationEditor *editor = element->getEditor();
    assert( editor != NULL );

    PropertyDelegate *pd = editor->getDelegate(attributeName);
    CHECK( convertAttributeType( attributeType, pd ), false );

    galaxyConfigOutput.writeAttribute( HRSchemaSerializer::TYPE_ATTR, attributeType );
    if( attributeType == "integer" ) {
       galaxyConfigOutput.writeAttribute( "value", "1" );
    }
    return false;
}

void GalaxyConfigTask::writeSelectAttribute( const PropertyDelegate *pd ) {
    QVariantMap items;
    pd->getItems(items);
    QVariantMap ::iterator itemsIterator;
    itemsIterator = items.begin();
    while( itemsIterator != items.end() ) {
        galaxyConfigOutput.writeStartElement( OPTION );
        galaxyConfigOutput.writeAttribute( "value", itemsIterator.value().toString() );
        if( itemsIterator == items.begin() ) {
            galaxyConfigOutput.writeAttribute( "selected", "true" );
        }
        galaxyConfigOutput.writeDTD( itemsIterator.key() );
        galaxyConfigOutput.writeEndElement();
        itemsIterator++;
    }
}

void GalaxyConfigTask::writeMinAndMaxAttributes( const PropertyDelegate *pd ) {
    QVariantMap items;
    pd->getItems(items);
    QString minValue = items.value("minimum").toString();
    QString maxValue = items.value("maximum").toString();
    galaxyConfigOutput.writeAttribute( "min", minValue );
    galaxyConfigOutput.writeAttribute( "max", maxValue );
}

bool GalaxyConfigTask::writeOptionElements() {
    QList <int> ::iterator optionElementsIterator;
    optionElementsIterator = optionElementsPositions.begin();
    while( optionElementsIterator != optionElementsPositions.end() ) {
        galaxyConfigOutput.writeStartElement( PARAM );
        const QMap < QString, QStringList > currAlias = elemAliases[ *optionElementsIterator ];
        QMap < QString, QStringList > ::const_iterator currAliasIterator = currAlias.begin();

        const QString elementName = (currAliasIterator).key(),
                      attributeName = (currAliasIterator).value().at(0),
                      aliasName = (currAliasIterator).value().at(1);
        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, aliasName );
       
        ActorPrototype *currElement = getElementFromActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        CHECK( writeTypeForOptionElement( (currAliasIterator).value(), currElement ), false );
        writeLabelAttribute( (currAliasIterator).value(), currElement );

        PropertyDelegate *pd = currElement->getEditor()->getDelegate(attributeName);
        if( pd != NULL ) {
            if( isDelegateComboBox(pd) ) {
                writeSelectAttribute( pd );
            } else if( isDelegateSpinBox(pd) ) {
                writeMinAndMaxAttributes( pd );
            }
        }
        galaxyConfigOutput.writeEndElement();
        optionElementsIterator++;
    }
    return true;
}

bool GalaxyConfigTask::writeInputsUnit() {
    galaxyConfigOutput.writeStartElement( INPUTS );  //inputs unit begin
    CHECK( writeInputElements(), false );
    CHECK( writeOptionElements(), false );
    galaxyConfigOutput.writeEndElement();            //inputs unit end
    return true;
}

void GalaxyConfigTask::writeFormatAttributeForOutputElement( const QString &resultType ) {
    DocumentFormatRegistry *docFormatRegistry = AppContext::getDocumentFormatRegistry();
    assert( NULL != docFormatRegistry );

    DocumentFormatConstraints constraint;
    constraint.supportedObjectTypes.insert( resultType );
    const QList <QString> selectedFormats = docFormatRegistry->selectFormats( constraint );
    galaxyConfigOutput.writeAttribute( FORMAT, selectedFormats.first() );
}

bool GalaxyConfigTask::checkDocumentFormatAttribute( const ActorPrototype *element ) {
    const QList < Attribute * > elementAttibutes = element->getAttributes();
    foreach( Attribute *elementAttribute, elementAttibutes ) {
        if( elementAttribute->getId() == BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId() ) {
            return true;
        }
    }
    return false;
}

void GalaxyConfigTask::writeChangeFormatAttribute( const QString &aliasName, const ActorPrototype *element ) {
    galaxyConfigOutput.writeStartElement( CHANGE_FORMAT );

    PropertyDelegate *pd = element->getEditor()->getDelegate( BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId() );
    assert( NULL != pd );

    QVariantMap items;
    pd->getItems(items);
    QVariantMap ::iterator itemsIterator;
    itemsIterator = items.begin();
    while( itemsIterator != items.end() ) {
        galaxyConfigOutput.writeStartElement( WHEN );
        galaxyConfigOutput.writeAttribute( "input", aliasName );
        galaxyConfigOutput.writeAttribute( "value", itemsIterator.value().toString() );
        galaxyConfigOutput.writeAttribute( FORMAT, itemsIterator.value().toString() );
        galaxyConfigOutput.writeEndElement();
        itemsIterator++;
    }
    galaxyConfigOutput.writeEndElement();
}

void GalaxyConfigTask::tryToWriteChangeFormatAttribute( const ActorPrototype *element, QList <int> &usedOptionElements ) {
    if( !checkDocumentFormatAttribute( element ) ) {
        return;
    }
    QList <int> ::iterator optionElementsIterator;
    optionElementsIterator = optionElementsPositions.begin();
    while( optionElementsIterator != optionElementsPositions.end() ) {
        QMap < QString, QStringList > ::iterator elementProperties = elemAliases[ *optionElementsIterator ].begin();

        const QString elementName = elementProperties.key(),
                      attributeName = elementProperties.value().at(0),
                      aliasName = elementProperties.value().at(1);

        if( elementName == element->getId() &&
            attributeName == BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId() &&
            !usedOptionElements.count( *optionElementsIterator ) )
        {
            usedOptionElements.push_back( *optionElementsIterator );
            writeChangeFormatAttribute( aliasName, element );
            break;
        }
        optionElementsIterator++;
    }
}

bool GalaxyConfigTask::writeOutputsUnit() {
    galaxyConfigOutput.writeStartElement( OUTPUTS );  //outputs unit begin
    QList <int>  usedOptionElements;

    QList <int> ::iterator outputElementsIterator;
    outputElementsIterator = outputElementsPositions.begin();
    while( outputElementsIterator != outputElementsPositions.end() ) {
        const QMap < QString, QStringList > currAlias = elemAliases[ *outputElementsIterator ];
        QMap < QString, QStringList > ::const_iterator currAliasIterator = currAlias.begin();

        const QString elementName = (currAliasIterator).key(),
                      aliasName = (currAliasIterator).value().at(1);

        ActorPrototype *currElement = getElementFromActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        const QString formatType = currElement->getPortDesciptors().first()->getDisplayName();
        QString resultType = QString();
        getConstraint( formatType, resultType );
        if ( resultType == GObjectTypes::UNKNOWN ) {
            stateInfo.setError( "Element \"" + currElement->getId() + "\" has unknown type" );
            return false;
        }
        galaxyConfigOutput.writeStartElement( DATA );
        writeFormatAttributeForOutputElement( resultType ); 
        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, aliasName );
        tryToWriteChangeFormatAttribute( currElement, usedOptionElements );
        galaxyConfigOutput.writeEndElement(); 
        outputElementsIterator++;
    }

    galaxyConfigOutput.writeStartElement( DATA );
    galaxyConfigOutput.writeAttribute( FORMAT, "txt" );
    galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, WORKFLOW_RUN_LOG );
    galaxyConfigOutput.writeAttribute( "label", WORKFLOW_RUN_LOG );
    galaxyConfigOutput.writeEndElement();

    galaxyConfigOutput.writeEndElement();            //outputs unit end
    return true;
}

void GalaxyConfigTask::writeHelpUnit() {
    galaxyConfigOutput.writeStartElement( HELP );  
    galaxyConfigOutput.writeDTD( galaxyHelpMessage );
    galaxyConfigOutput.writeEndElement();      
}

bool GalaxyConfigTask::createConfigForGalaxy() {
    const QString schemePathWithoutExtension = schemePath.mid(0, schemePath.length()-4 );

    QFile galaxyConfigFile( schemePathWithoutExtension + ".xml");
    galaxyConfigFile.open( QIODevice::WriteOnly );
    if( !galaxyConfigFile.isOpen() ) {
        this->stateInfo.setError( "Can not open " + galaxyConfigFile.fileName() );
        return false;
    }
    galaxyConfigOutput.setDevice(&galaxyConfigFile);

    writeToolUnit();
    CHECK( writeCommandUnit(), false );
    CHECK( writeInputsUnit(), false );
    CHECK( writeOutputsUnit(), false );
    writeHelpUnit();
    galaxyConfigOutput.writeEndElement(); //tool unit end

    galaxyConfigFile.close();
    return true;
}

void GalaxyConfigTask::doCopyCommands( const QString &pathToCopy ) {
    system( QString("cp " + schemePath.left( schemePath.length() - 4 ) + ".xml" + pathToCopy).toLocal8Bit().constData() );
    system( QString("cp " + schemePath + " " + pathToCopy).toLocal8Bit().constData() );
}
void GalaxyConfigTask::doDeleteCommands() {
    system( QString("rm " + schemePath.left( schemePath.length() - 4 ) + ".xml").toLocal8Bit().constData() );
}

bool GalaxyConfigTask::prepareDirectoryForTool() {
    QString schemeNameWithoutExtension = schemeName.left( schemeName.length() - 4 );
    const QString pathToCopy = galaxyPath + "tools/" + schemeNameWithoutExtension;
    QDir directory;
    directory.setPath( pathToCopy );
    if( !directory.exists() ) {
        const QString createDirCommand = "mkdir " + pathToCopy;
        system( createDirCommand.toLocal8Bit().constData() );
    }
    if( !directory.exists() ) {
        return false;
    }
    doCopyCommands( pathToCopy );
    doDeleteCommands();
    return true;
}

void GalaxyConfigTask::makeCopyOfGalaxyToolConfig() {
    const QString toolConfigurationPath = galaxyPath + "tool_conf.xml";
    system( QString("cp " + toolConfigurationPath + " " + toolConfigurationPath + "_old").toLocal8Bit().constData() );
}

bool GalaxyConfigTask::writeNewSection( const QString &config ) {
    const int toolboxPosition = config.indexOf( "<toolbox>" );
    const int beginLength = toolboxPosition + QString("<toolbox>").length();
    QString begin = config.mid(0, beginLength );
    QString end = config.mid( beginLength, config.length() - begin.length() );

    const QString toolsConfigurationPath = galaxyPath + "tool_conf.xml";
    QFile configFile( toolsConfigurationPath );
    if( !configFile.open( QIODevice::WriteOnly ) ) {
        return false;
    }

    QXmlStreamWriter galaxyToolsConfigOutput;
    galaxyToolsConfigOutput.setDevice( &configFile );
    galaxyToolsConfigOutput.writeDTD( begin );
    galaxyToolsConfigOutput.writeStartElement( "section" );
    galaxyToolsConfigOutput.writeAttribute( "name", galaxyToolName + "-tool" );
    QString idStrCopy = galaxyToolName;
    idStrCopy.replace(" ","_");
    galaxyToolsConfigOutput.writeAttribute( "id", idStrCopy );
    galaxyToolsConfigOutput.writeStartElement( "tool" );
    const QString schemeNameWithoutExtension = schemeName.left( schemeName.length() - 4 );
    galaxyToolsConfigOutput.writeAttribute( "file", schemeNameWithoutExtension + "/" + schemeNameWithoutExtension + ".xml" );
    galaxyToolsConfigOutput.writeEndElement();
    galaxyToolsConfigOutput.writeEndElement();
    galaxyToolsConfigOutput.writeDTD( end );
    return true;
}

bool GalaxyConfigTask::addToolToConfig() {
    const QString toolsConfigurationPath = galaxyPath + "tool_conf.xml";
    QFile configFile( toolsConfigurationPath );
    if( !configFile.open( QIODevice::ReadOnly ) ) {
        return false;
    }
    QTextStream input( &configFile );
    QString config = input.readAll();
    configFile.close();
    if( config.indexOf( galaxyToolName ) != SUBSTRING_NOT_FOUND ) {
        return true;
    }
    return writeNewSection( config );
}

bool GalaxyConfigTask::addToolToGalaxyConfig() {
    makeCopyOfGalaxyToolConfig();
    return addToolToConfig();
}

} // U2
