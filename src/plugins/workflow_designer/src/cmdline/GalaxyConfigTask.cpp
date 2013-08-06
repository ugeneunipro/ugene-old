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

#include "GalaxyConfigTask.h"

namespace U2 {

/*******************************************
* GalaxyConfigTask
*******************************************/

const QString GalaxyConfigTask::GALAXY_CONFIG_OPTION              = "galaxy-config";
const QString GalaxyConfigTask::GALAXY_PATH_OPTION                = "galaxy-path";

const QString GalaxyConfigTask::TOOL                              = "tool";
const QString GalaxyConfigTask::NAME                              = "name";
const QString GalaxyConfigTask::ID                                = "id";
const QString GalaxyConfigTask::COMMAND                           = "command";
const QString GalaxyConfigTask::INPUTS                            = "inputs";
const QString GalaxyConfigTask::OUTPUTS                           = "outputs";
const QString GalaxyConfigTask::PARAM                             = "param";
const QString GalaxyConfigTask::FORMAT                            = "format";
const QString GalaxyConfigTask::OPTION                            = "option";
const QString GalaxyConfigTask::DATA                              = "data";
const QString GalaxyConfigTask::CHANGE_FORMAT                     = "change-format";
const QString GalaxyConfigTask::WHEN                              = "when";
const QString GalaxyConfigTask::HELP                              = "help";

const QString GalaxyConfigTask::WORKFLOW_RUN_LOG                  = "ugene_workflow_run_log";

static int SUBSTRING_NOT_FOUND = -1;

GalaxyConfigTask::GalaxyConfigTask() 
:Task( tr( "Create Galaxy config from existing workflow scheme" ), TaskFlag_NoRun ) {
    appDirPath = QApplication::applicationDirPath() + "/";
    if( !getSchemeName() ) {
        coreLog.error("Scheme name is incorrect");
        return;
    }
    if( !getGalaxyPath() ) {
        coreLog.error("Galaxy directory is not found");
        return;
    }
    if( !getSchemeContent() ) {
        coreLog.error("Can't read scheme content");
        return;
    }
    if( !getHelpMessage() ) {
        coreLog.error("Can't read scheme description");
        return;
    }
    if( !getWorkflowName() ) {
        coreLog.error("Can't read scheme name");
        return;
    }
    if( !defineAliases() ){
        coreLog.error("Aliases are not found");
        return;
    }
    if( !createConfigForGalaxy() ) { 
        coreLog.error("Config for Galaxy is not created");
        return;
    }
    if( !prepareDirectoryForTool() ) {
        coreLog.error("Can't create directory for Galaxy tool. Check user privileges");
        return;
    }
    if( !addToolToGalaxyConfigFile() ) {
        coreLog.error("Can't add new tool to tool_conf.xml. Check user privileges");
        return;
    }
}

GalaxyConfigTask::~GalaxyConfigTask() {

}

bool GalaxyConfigTask::tryToFindInPath( const QString &objectName, QString &objectPath ) {
    QString pathVariable = qgetenv("PATH").constData();
    const int objectNamePosition = pathVariable.indexOf( objectName );
    if( objectNamePosition == SUBSTRING_NOT_FOUND ) {
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
    file.open(QIODevice::ReadOnly );
    QTextStream inFile(&file);
    inFile >> objectPath;
    file.close();
    system( QString("rm " + objectName + "_path.txt").toLocal8Bit().constData() );
    if( !objectPath.length() ) {
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
        return false;
    }
    schemePath = paramValues.first();
    schemeName = schemePath.split("/").last();
    return true;
}

bool GalaxyConfigTask::getSchemeContent() {
    QFile schemeFile( schemePath );
    if( !schemeFile.open(QIODevice::ReadOnly ) ) {
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
        return false;
    }
    commentStartPosition += HRSchemaSerializer::HEADER_LINE.length();
    const int commentEndPosition = schemeContent.lastIndexOf( HRSchemaSerializer::BODY_START ),
              commentLength = commentEndPosition - commentStartPosition;
    QString comment = QString();
    comment = schemeContent.mid( commentStartPosition, commentLength );
    comment.replace( HRSchemaSerializer::SERVICE_SYM, "\r\n" );
    galaxyHelpMessage += comment;
    return true;
}

bool GalaxyConfigTask::getWorkflowName() {
    int nameStartPosition = schemeContent.lastIndexOf( HRSchemaSerializer::BODY_START );
    if( nameStartPosition == SUBSTRING_NOT_FOUND ) {
        return false;
    }
    nameStartPosition += HRSchemaSerializer::BODY_START.length() + 1;
    const int nameEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_START, nameStartPosition ),
              nameLength = nameEndPosition - nameStartPosition;
    galaxyToolName = schemeContent.mid( nameStartPosition, nameLength );
    galaxyToolName.replace( QRegExp("^\""),"" );
    galaxyToolName.replace( QRegExp("\"$"),"" );
    return true;
}

void GalaxyConfigTask::getParameterValue( const QString &keyword, const int searchFrom, QString &parameterValue, int &nextSearchFrom ) {
    const int keywordPosition = schemeContent.indexOf( keyword, searchFrom ),
              blockEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_END, searchFrom );
    if( keyword == HRSchemaSerializer::DESCRIPTION && ( keywordPosition == - 1 || blockEndPosition < keywordPosition ) ) {
        nextSearchFrom = searchFrom;
        return;
    }
    const int parameterStartPosition = schemeContent.indexOf( HRSchemaSerializer::COLON, keywordPosition ) + 1,
              parameterEndPosition = schemeContent.indexOf( HRSchemaSerializer::SEMICOLON, parameterStartPosition ),
              parameterLength = parameterEndPosition - parameterStartPosition;
    parameterValue = schemeContent.mid( parameterStartPosition, parameterLength );
    nextSearchFrom = parameterEndPosition;
}

bool GalaxyConfigTask::defineAliases() {
    int aliasesStartPosition = schemeContent.indexOf( HRSchemaSerializer::PARAM_ALIASES_START );
    if( aliasesStartPosition == SUBSTRING_NOT_FOUND ) {
        return false;
    }
    aliasesStartPosition += HRSchemaSerializer::PARAM_ALIASES_START.length();
    const int visualKeywordPosition = schemeContent.indexOf( HRSchemaSerializer::VISUAL_START, aliasesStartPosition );
    int elementNameStartPosition = schemeContent.indexOf( QRegExp("[a-z]"), aliasesStartPosition );
    while ( elementNameStartPosition < visualKeywordPosition ) {
        const int elementNameEndPosition = schemeContent.indexOf( HRSchemaSerializer::DOT, elementNameStartPosition ),
                  elementNameLength = elementNameEndPosition - elementNameStartPosition;
        QString elementName = schemeContent.mid( elementNameStartPosition, elementNameLength );
        elementName.replace(QRegExp("[0-9]$"),"");

        const int elementAliasStartPosition = elementNameEndPosition + 1,
                  elementAliasEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_START, elementAliasStartPosition ),
                  elementAliasLength = elementAliasEndPosition - elementAliasStartPosition;
        QString elementAlias = schemeContent.mid( elementAliasStartPosition, elementAliasLength );
        elementAlias.replace(" ","");

        QString aliasName, aliasDescription;
        int aliasNameEndPosition, aliasDescriptionEndPosition;

        getParameterValue( HRSchemaSerializer::ALIAS, elementAliasEndPosition, aliasName, aliasNameEndPosition );
        getParameterValue( HRSchemaSerializer::DESCRIPTION, aliasNameEndPosition, aliasDescription, aliasDescriptionEndPosition );

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

ActorPrototype* GalaxyConfigTask::findElemInActorPrototypeRegistry( const QString &elementName ) {
    U2::Workflow::ActorPrototypeRegistry *prototypeRegistry
        = U2::Workflow::WorkflowEnv::getProtoRegistry();
    assert( NULL != prototypeRegistry );
    return prototypeRegistry->getProto( elementName );
}

void GalaxyConfigTask::getTypeOfAttribute( const QString &elementAttribute, const int elementPosition ) {
    if( BaseAttributes::URL_IN_ATTRIBUTE().getId() == elementAttribute ) {
        inputElementsPositions.push_back( elementPosition );
    } else if ( BaseAttributes::URL_OUT_ATTRIBUTE().getId() == elementAttribute ){
        outputElementsPositions.push_back( elementPosition );
    } else {
        optionElementsPositions.push_back( elementPosition );
    }
}

void GalaxyConfigTask::divideElementsByTypes() {
    QList < QMap < QString, QStringList > > ::iterator elemAliasesIterator;
    elemAliasesIterator = elemAliases.begin();
    while( elemAliasesIterator != elemAliases.end() ) {
        const QMap < QString, QStringList > ::iterator elementProperties = (*elemAliasesIterator).begin();
        QString elementName = elementProperties.key();
        QString attributeName = elementProperties.value().at(0);

        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        const QList < Attribute* > elementAttibutes = currElement->getAttributes();
        foreach( Attribute *elementAttribute, elementAttibutes ) {
            if( elementAttribute->getId() == attributeName ) {
                getTypeOfAttribute( attributeName, std::distance( elemAliases.begin(), elemAliasesIterator ) );
            }
        }
        elemAliasesIterator++;
    }
}

void GalaxyConfigTask::writeCommandUnit() {
    galaxyConfigOutput.writeStartElement ( COMMAND ); //command unit begin 
    divideElementsByTypes();

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
    runUgene += "  >> $" + WORKFLOW_RUN_LOG + redirectStderrToStdout;
    galaxyConfigOutput.writeCharacters( runUgene );

    QList <int> ::iterator outputElementsPositionsIterator;
    outputElementsPositionsIterator = outputElementsPositions.begin();
    while( outputElementsPositionsIterator != outputElementsPositions.end() ) {
        QMap <QString, QStringList> ::iterator elementParameters = elemAliases[*outputElementsPositionsIterator].begin();
        QString aliasName = elementParameters.value().at(1);
        galaxyConfigOutput.writeDTD( QString("\r\nif [ ! -s $" + aliasName + " ]; then") );
        galaxyConfigOutput.writeDTD( QString("\r\necho \"EMPTY RESULT FILE\" > $" + aliasName + ";") );
        galaxyConfigOutput.writeDTD( QString("\r\nfi;\r\n") );
        outputElementsPositionsIterator++;
    }
    galaxyConfigOutput.writeEndElement();            //command unit end
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
    resultFormatString += selectedFormats.last();
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

void GalaxyConfigTask::writeInputElements() {
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

        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        const QString formatType = currElement->getPortDesciptors().first()->getDisplayName();
        QString resultType = QString();
        getConstraint( formatType, resultType );
        if ( resultType == GObjectTypes::UNKNOWN ) {
            inputElementsIterator++;
            continue;
        }
        writeFormatAttribute( resultType );
        writeLabelAttribute( (currAliasIterator).value(), currElement );
        galaxyConfigOutput.writeEndElement();
        inputElementsIterator++;
    }
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

void GalaxyConfigTask::convertAttributeType( QString &attributeType, PropertyDelegate *pd ) {
    if( isDelegateComboBox(pd) && pd != NULL ) {
        attributeType = "select";
    }
    else if ( isDelegateSpinBox(pd) && pd != NULL && attributeType == BaseTypes::NUM_TYPE()->getId() ) {
        attributeType = "integer";
        QVariantMap items;
        pd->getItems( items );
        const QString typeName1 = items.value("minimum").typeName();
        const QString typeName2 = items.value("minimum").typeName();
        if( typeName1 == "double" || typeName2 == "double" ) {
            attributeType = "float";
        }
    }
    else if( attributeType == BaseTypes::BOOL_TYPE()->getId() ) {
        attributeType = "boolean";
    }
    else if( attributeType == BaseTypes::STRING_TYPE()->getId() ) {
        attributeType = "text";
    }
}

void GalaxyConfigTask::writeTypeForOptionElement( const QStringList &elementParameters, const ActorPrototype *element ) {
    const QString attributeName = elementParameters.at(0);
    QString attributeType = element->getAttribute(attributeName)->getAttributeType()->getId();

    PropertyDelegate *pd = element->getEditor()->getDelegate(attributeName);

    convertAttributeType( attributeType, pd );
    galaxyConfigOutput.writeAttribute( HRSchemaSerializer::TYPE_ATTR, attributeType );
    if( attributeType == "integer" ) {
       galaxyConfigOutput.writeAttribute( "value", "1" );
    }
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

void GalaxyConfigTask::writeOptionElements() {
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
       
        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        writeTypeForOptionElement( (currAliasIterator).value(), currElement );

        PropertyDelegate *pd = currElement->getEditor()->getDelegate(attributeName);

        writeLabelAttribute( (currAliasIterator).value(), currElement );
        if( pd != NULL ) {
            if( isDelegateComboBox(pd) ) {
                writeSelectAttribute( pd );
            }
            if( isDelegateSpinBox(pd) ) {
                writeMinAndMaxAttributes( pd );
            }
        }
        galaxyConfigOutput.writeEndElement();
        optionElementsIterator++;
    }
}

void GalaxyConfigTask::writeInputsUnit() {
    galaxyConfigOutput.writeStartElement( INPUTS );  //inputs unit begin
    writeInputElements();
    writeOptionElements();
    galaxyConfigOutput.writeEndElement();            //inputs unit end
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
            return;
        }
        optionElementsIterator++;
    }
}

void GalaxyConfigTask::writeOutputsUnit() {
    galaxyConfigOutput.writeStartElement( OUTPUTS );  //outputs unit begin
    QList <int>  usedOptionElements;
    QList <int> ::iterator outputElementsIterator;
    outputElementsIterator = outputElementsPositions.begin();
    while( outputElementsIterator != outputElementsPositions.end() ) {
        const QMap < QString, QStringList > currAlias = elemAliases[ *outputElementsIterator ];
        QMap < QString, QStringList > ::const_iterator currAliasIterator = currAlias.begin();

        const QString elementName = (currAliasIterator).key(),
                      aliasName = (currAliasIterator).value().at(1);

        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        assert( NULL != currElement );

        const QString formatType = currElement->getPortDesciptors().first()->getDisplayName();
        QString resultType = QString();
        getConstraint( formatType, resultType );
        if ( resultType == GObjectTypes::UNKNOWN ) {
            outputElementsIterator++;
            continue;
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
        coreLog.error( "Can't open " + schemePathWithoutExtension + ".xml" );
        return false;
    }

    galaxyConfigOutput.setDevice(&galaxyConfigFile);
    writeToolUnit();
    writeCommandUnit();
    writeInputsUnit();
    writeOutputsUnit();
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

bool GalaxyConfigTask::addToolToGalaxyConfigFile() {
    makeCopyOfGalaxyToolConfig();
    return addToolToConfig();
}

} // U2
