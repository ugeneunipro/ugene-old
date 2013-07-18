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
const QString GalaxyConfigTask::UGENE_PATH_OPTION                 = "ugene-path";
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

static int SUBSTRING_NOT_FOUND = -1;

GalaxyConfigTask::GalaxyConfigTask() 
:Task( tr( "Create Galaxy config from existing workflow scheme" ), TaskFlag_NoRun ) {
    if( !getSchemeName() ) {
        coreLog.error("Scheme name is incorrect");
        return;
    }
    /*if( !getUgenePath() ) {
        coreLog.error("UGENE is not found");
        return;
    }*/
    /*if( !getGalaxyPath() ) {
        coreLog.error("Galaxy is not found");
        return;
    }*/
    getSchemeContent();
    getHelpMessage();
    getWorkflowName();
    if( !defineAliases() ){
        coreLog.error("Aliases are not found");
        return;
    }
    if( !createConfigForGalaxy() ) { 
        coreLog.error("Config for Galaxy is not created");
        return;
    }
    if( !createConfigForUgene() ) {
        coreLog.error("Config for Ugene is not created");
        return;
    }
    prepareDirectoryForTool();
    addToolToGalaxyConfigFile();
}
GalaxyConfigTask::~GalaxyConfigTask() {

}

bool GalaxyConfigTask::readPathFromPATHVariable( const QString &objectName, QString &objectPath ) {
    QProcess process;
    process.start("locate" + objectName + " -l 1 > " + objectName + "_path.txt");
    //сделать, чтобы файлы для определения пути программы в PATH создавались во временной директории
    QFile file(objectName + "_path.txt");
    file.open(QIODevice::ReadOnly );
    QTextStream inFile(&file);
    inFile >> objectPath;
    file.close();
    process.start("rm " + objectName + "_path.txt");
    if( !objectPath.length() ) {
        return false;
    }
    objectPath += "/";
    return true;
}

bool GalaxyConfigTask::getPath( const QString &whatPath, QString &resultPath ) {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );

    const int result = CMDLineRegistryUtils::getParameterIndex(whatPath);
    const QStringList paramValues = CMDLineRegistryUtils::getParameterValues(whatPath);
    if( result == -1 || !paramValues.first().length() ) {
        if( whatPath == UGENE_PATH_OPTION ) {
            return readPathFromPATHVariable("ugene", ugenePath);
            
        } else {
            return readPathFromPATHVariable("galaxy", galaxyPath);
        }
    }
    resultPath = paramValues.first();
    return true;
}

bool GalaxyConfigTask::getUgenePath() {
    return getPath( UGENE_PATH_OPTION, ugenePath );
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

void GalaxyConfigTask::getSchemeContent() {
    const QString fileName = schemePath;
    QFile schemeFile( fileName );
    schemeFile.open(QIODevice::ReadOnly );
    QTextStream input(&schemeFile);
    schemeContent = input.readAll();
    schemeFile.close();
}

void GalaxyConfigTask::getHelpMessage() {
    galaxyHelpMessage = "\n**Description**\n\n";
    const int commentStartPosition = schemeContent.indexOf( HRSchemaSerializer::HEADER_LINE );
    const int commentEndPosition = schemeContent.lastIndexOf( HRSchemaSerializer::BODY_START );
    const int commentLength = commentEndPosition - commentStartPosition;
    const QString comment= schemeContent.mid( commentStartPosition, commentLength );
    galaxyHelpMessage += comment;
}

void GalaxyConfigTask::getWorkflowName() {
    const int nameStartPosition = schemeContent.lastIndexOf( HRSchemaSerializer::BODY_START ) + HRSchemaSerializer::BODY_START.length() + 1; // +1 because of space symbol
    const int nameEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_START, nameStartPosition );
    const int nameLength = nameEndPosition - nameStartPosition;
    galaxyToolName = schemeContent.mid( nameStartPosition, nameLength );
    galaxyToolName.replace( QRegExp("^\""),"" );
    galaxyToolName.replace( QRegExp("\"$"),"" );
}

void GalaxyConfigTask::getParameterValue( const QString &keyword, const int searchFrom, QString &parameterValue, int &nextSearchFrom ) {
    const int keywordPosition = schemeContent.indexOf( keyword, searchFrom );
    const int blockEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_END, searchFrom );
    if( keyword == HRSchemaSerializer::DESCRIPTION && ( keywordPosition == - 1 || blockEndPosition < keywordPosition ) ) {
        nextSearchFrom = searchFrom;
        return;
    }
    const int parameterStartPosition = schemeContent.indexOf( HRSchemaSerializer::COLON, keywordPosition ) + 1;
    const int parameterEndPosition = schemeContent.indexOf( HRSchemaSerializer::SEMICOLON, parameterStartPosition );
    const int parameterLength = parameterEndPosition - parameterStartPosition;
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
        const int elementNameEndPosition = schemeContent.indexOf( HRSchemaSerializer::DOT, elementNameStartPosition );
        const int elementNameLength = elementNameEndPosition - elementNameStartPosition;
        QString elementName = schemeContent.mid( elementNameStartPosition, elementNameLength );
        elementName.replace(QRegExp("[0-9]$"),"");

        const int elementAliasStartPosition = elementNameEndPosition + 1;
        const int elementAliasEndPosition = schemeContent.indexOf( HRSchemaSerializer::BLOCK_START, elementAliasStartPosition );
        const int elementAliasLength = elementAliasEndPosition - elementAliasStartPosition;
        QString elementAlias = schemeContent.mid( elementAliasStartPosition, elementAliasLength );
        elementAlias.replace(" ","");

        QString aliasName, 
                aliasDescription;
        int aliasNameEndPosition, 
            aliasDescriptionEndPosition;
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
    galaxyConfigOutput.writeAttribute( NAME, galaxyToolName );
}

ActorPrototype* GalaxyConfigTask::findElemInActorPrototypeRegistry( const QString &elementName ) {
    U2::Workflow::ActorPrototypeRegistry *prototypeRegistry
        = U2::Workflow::WorkflowEnv::getProtoRegistry();
    return prototypeRegistry->getProto( elementName );
}

void GalaxyConfigTask::makePathToGalaxyToolsDir() {
    QString galaxyToolsPath = galaxyPath + "tools/";
    QString fullPathToGalaxyDir = galaxyToolsPath + schemeName.mid(0,schemeName.length() - 4 );
    fullPathToGalaxyDir += HRSchemaSerializer::SEMICOLON + HRSchemaSerializer::NEW_LINE;
    galaxyConfigOutput.writeCharacters("cd " + fullPathToGalaxyDir );
}
void GalaxyConfigTask::getTypeOfAttribute( const QString &elementAttribute, const int elementPosition ) {
    if( BaseAttributes::URL_IN_ATTRIBUTE().getId() == elementAttribute ) {
        inputElementsPositions.push_back( elementPosition );
    }
    else if ( BaseAttributes::URL_OUT_ATTRIBUTE().getId() == elementAttribute ){
        outputElementsPositions.push_back( elementPosition );
    }
    else {
        optionElementsPositions.push_back( elementPosition );
    }
}

void GalaxyConfigTask::divideElementsByTypes() {
    QList < QMap < QString, QStringList > > ::iterator elemAliasesIterator;
    elemAliasesIterator = elemAliases.begin();
    while( elemAliasesIterator != elemAliases.end() ) {
        QMap < QString, QStringList > ::iterator elementProperties = (*elemAliasesIterator).begin();
        QString elementName = elementProperties.key();
        QString attributeName = elementProperties.value().at(0);
        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        if( currElement == NULL ) {
            elemAliasesIterator++;
            continue;
        }
        QList < Attribute* > elementAttibutes = currElement->getAttributes();
        foreach( Attribute *elementAttribute, elementAttibutes ) {
            if( elementAttribute->getId() == attributeName ) {
                getTypeOfAttribute( attributeName, std::distance( elemAliases.begin(), elemAliasesIterator ) );
            }
        }
        elemAliasesIterator++;
    }
}

void GalaxyConfigTask::writeConditionalStatementsAboutTmpFiles() {
    galaxyConfigOutput.writeCharacters((QString)"if [ -e no_input.txt ];\nthen\n" + 
                                       (QString)"rm no_input.txt;\n" + 
                                       (QString)"fi;\n" + 
                                       (QString)"if [ -e empty_aliases.txt ];\nthen\n" + 
                                       (QString)"rm empty_aliases.txt;\n" + 
                                       (QString)"fi;\n" + 
                                       (QString)"if [ -e ugene ];\nthen\n" + 
                                       (QString)"rm ugene;\n" + 
                                       (QString)"fi;\n" );
}
void GalaxyConfigTask::writeConditionalStatementsForInputElements() {
    QList <int> ::iterator inputElementsIterator;
    inputElementsIterator = inputElementsPositions.begin();
    while( inputElementsIterator != inputElementsPositions.end() ) {
        QMap < QString, QStringList > currAlias = elemAliases[ *inputElementsIterator ];
        QMap < QString, QStringList > ::iterator currAliasIterator = currAlias.begin();
        QStringList elementParameters = (currAliasIterator).value();
        galaxyConfigOutput.writeCharacters( QString("if [ -z $") + elementParameters.at(1) + QString(" ];\nthen\n") );
        galaxyConfigOutput.writeCharacters( QString("echo ") + elementParameters.at(1) + QString(" >> no_input.txt;\n") );
        galaxyConfigOutput.writeCharacters( "fi;\n" );
        inputElementsIterator++;
    }
}

void GalaxyConfigTask::writeConditionalStatementsForOptionElements() {
    QList <int> ::iterator optionElementsIterator;
    optionElementsIterator = optionElementsPositions.begin();
    while( optionElementsIterator != optionElementsPositions.end() ) {
        QMap < QString, QStringList > currAlias = elemAliases[ *optionElementsIterator ];
        QMap < QString, QStringList > ::iterator currAliasIterator = currAlias.begin();
        QStringList elementParameters = (currAliasIterator).value();
        galaxyConfigOutput.writeCharacters( QString("if [ -z $") + elementParameters.at(1) + QString(" ];\nthen\n") );
        galaxyConfigOutput.writeCharacters( QString("echo ") + elementParameters.at(1) + QString(" >> empty_aliases.txt;\n") );
        galaxyConfigOutput.writeCharacters( "fi;\n" );
        optionElementsIterator++;
    }
}

void GalaxyConfigTask::writeSelectedElements( const QList <int> &elementsPositions ) {
    QList <int> ::const_iterator elementsIterator;
    elementsIterator = elementsPositions.begin();
    while( elementsIterator != elementsPositions.end() ) {
        QMap < QString, QStringList > currAlias = elemAliases[ *elementsIterator ];
        QMap < QString, QStringList > ::iterator currAliasIterator = currAlias.begin();
        QStringList elementParameters = (currAliasIterator).value();
        galaxyConfigOutput.writeCharacters( QString("$") + elementParameters.at(1) + " " );
        elementsIterator++;
    }
}
void GalaxyConfigTask::writeAllElements() {
    writeSelectedElements( inputElementsPositions );
    writeSelectedElements( optionElementsPositions );
    writeSelectedElements( outputElementsPositions );
    galaxyConfigOutput.writeCharacters( HRSchemaSerializer::SEMICOLON );
}

void GalaxyConfigTask::writeCommandUnit() {
    galaxyConfigOutput.writeStartElement ( COMMAND ); //command unit begin 
    makePathToGalaxyToolsDir();
    divideElementsByTypes();
    writeConditionalStatementsAboutTmpFiles();
    writeConditionalStatementsForInputElements();
    writeConditionalStatementsForOptionElements();
    //нужно изменить текущий вызов, т.к. не будет утилиты ugene_run. Может быть сделать тут вызов Ugene с указанием папки tool'a ?
    const QString runUgene = "./ugene_run ";
    galaxyConfigOutput.writeCharacters(runUgene);
    //-------------------------------------------------------------
    writeAllElements();
    galaxyConfigOutput.writeEndElement(); //command unit end
}

void GalaxyConfigTask::getConstraint( const QString &typeName, QString &resultType ) {
    if ( typeName == GObjectTypes::getTypeInfo( GObjectTypes::TEXT ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::TEXT ).type;
    } 
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::SEQUENCE ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::SEQUENCE ).type;
    } 
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::ANNOTATION_TABLE ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::ANNOTATION_TABLE ).type;
    }
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::VARIANT_TRACK ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::VARIANT_TRACK ).type;
    }
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::CHROMATOGRAM ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::CHROMATOGRAM ).type;
    }
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::MULTIPLE_ALIGNMENT ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::MULTIPLE_ALIGNMENT ).type;
    }
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::PHYLOGENETIC_TREE ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::PHYLOGENETIC_TREE ).type;
    }
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::BIOSTRUCTURE_3D ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::BIOSTRUCTURE_3D ).type;
    }
    //else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::UINDEX ).name ) {
    //    resultType = GObjectTypes::getTypeInfo( GObjectTypes::UINDEX ).type;
    //}
    else if( typeName == GObjectTypes::getTypeInfo( GObjectTypes::ASSEMBLY ).name ) {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::ASSEMBLY ).type;
    }
    else {
        resultType = GObjectTypes::getTypeInfo( GObjectTypes::UNKNOWN ).type;
    }
}

void GalaxyConfigTask::writeFormatAttribute( const QString &resultType ) {
    DocumentFormatRegistry *docFormatRegistry = AppContext::getDocumentFormatRegistry();
    DocumentFormatConstraints constraint;
    constraint.supportedObjectTypes.insert( resultType );
    QList <QString> selectedFormats = docFormatRegistry->selectFormats( constraint );

    QString resultFormatString = QString(); 
    QList <QString> ::iterator selectedFormatsIterator;
    selectedFormatsIterator = selectedFormats.begin();
    while( selectedFormatsIterator != selectedFormats.end()-1 ) {
        resultFormatString += *selectedFormatsIterator;
        resultFormatString += HRSchemaSerializer::COMMA;
        selectedFormatsIterator++;
    }
    resultFormatString += selectedFormats.last();
    galaxyConfigOutput.writeAttribute( FORMAT, resultFormatString );
}

void GalaxyConfigTask::writeLabelAttribute( const QStringList &elementParameters, const ActorPrototype *element ) {
    QString attributeName = elementParameters.at(0);
    QString aliasDescription = elementParameters.at(2);
    QString copyStr = aliasDescription;
    if( aliasDescription.length() == 0 || ( copyStr.replace(" ","") ).length() == 0 ) {
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
        aliasDescription.remove( aliasDescription.length()-1, 1 );
    }
    galaxyConfigOutput.writeAttribute( "label", aliasDescription );
}

void GalaxyConfigTask::writeInputElements() {
    QList <int> ::iterator inputElementsIterator;
    inputElementsIterator = inputElementsPositions.begin();
    while( inputElementsIterator != inputElementsPositions.end() ) {
        galaxyConfigOutput.writeStartElement( PARAM );
        QMap < QString, QStringList > currAlias = elemAliases[ *inputElementsIterator ];
        QMap < QString, QStringList > ::iterator currAliasIterator = currAlias.begin();
        QString elementName = (currAliasIterator).key(),
                aliasName = (currAliasIterator).value().at(1);

        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, aliasName );
        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::TYPE_ATTR, DATA );

        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        QString formatType = currElement->getPortDesciptors().first()->getDisplayName(),
                resultType = QString();
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
    if( isDelegateComboBox(pd) ) {
        attributeType = "select";
    }
    else if ( isDelegateSpinBox(pd) && attributeType == BaseTypes::NUM_TYPE()->getId() ) {
        attributeType = "integer";
        QVariantMap items;
        pd->getItems( items );
        if( items.value("minimum").typeName() == "double" || items.value("maximum").typeName() == "double" ) {
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
    QString attributeName = elementParameters.at(0),
            attributeType = element->getAttribute(attributeName)->getAttributeType()->getId();
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
        galaxyConfigOutput.writeCharacters( itemsIterator.key() );
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
        QMap < QString, QStringList > currAlias = elemAliases[ *optionElementsIterator ];
        QMap < QString, QStringList > ::iterator currAliasIterator = currAlias.begin();
        QString elementName = (currAliasIterator).key(), 
                attributeName = (currAliasIterator).value().at(0), 
                aliasName = (currAliasIterator).value().at(1);
        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, aliasName );
       
        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        writeTypeForOptionElement( (currAliasIterator).value(), currElement );

        PropertyDelegate *pd = currElement->getEditor()->getDelegate(attributeName);
        writeLabelAttribute( (currAliasIterator).value(), currElement );
        if( !isDelegateComboBox(pd) ) {
            galaxyConfigOutput.writeAttribute( "optional", "true" );
        }
        else {
            writeSelectAttribute( pd );
        }
        if( isDelegateSpinBox( pd ) ) {
            writeMinAndMaxAttributes( pd );
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
    DocumentFormatConstraints constraint;
    constraint.supportedObjectTypes.insert( resultType );
    QList <QString> selectedFormats = docFormatRegistry->selectFormats( constraint );
    galaxyConfigOutput.writeAttribute( FORMAT, selectedFormats.first() );
}

bool GalaxyConfigTask::checkDocumentFormatAttribute( const ActorPrototype *element ) {
    QList < Attribute * > elementAttibutes = element->getAttributes();
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
        QString elementName = elementProperties.key(),
                attributeName = elementProperties.value().at(0),
                aliasName = elementProperties.value().at(1);
        if( elementName == element->getId() && attributeName == BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId() && 
            !usedOptionElements.count( *optionElementsIterator ) ) {
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
        galaxyConfigOutput.writeStartElement( DATA );
        QMap < QString, QStringList > currAlias = elemAliases[ *outputElementsIterator ];
        QMap < QString, QStringList > ::iterator currAliasIterator = currAlias.begin();
        QString elementName = (currAliasIterator).key(),
                aliasName = (currAliasIterator).value().at(1);

        ActorPrototype *currElement = findElemInActorPrototypeRegistry( elementName );
        QString formatType = currElement->getPortDesciptors().first()->getDisplayName(),
                resultType = QString();
        getConstraint( formatType, resultType );
        if ( resultType == GObjectTypes::UNKNOWN ) {
            outputElementsIterator++;
            continue;
        }
        writeFormatAttributeForOutputElement( resultType ); 
        galaxyConfigOutput.writeAttribute( HRSchemaSerializer::NAME_ATTR, aliasName );
        tryToWriteChangeFormatAttribute( currElement, usedOptionElements );  //подправить, чтобы для 2 выходных элементов всё было ок
        galaxyConfigOutput.writeEndElement(); 
        outputElementsIterator++;
    }
    galaxyConfigOutput.writeEndElement();            //outputs unit end
}

void GalaxyConfigTask::writeHelpUnit() {
    galaxyConfigOutput.writeStartElement( HELP );  //help unit begin
    galaxyConfigOutput.writeCharacters( galaxyHelpMessage );
    galaxyConfigOutput.writeEndElement();          //help unit end
}

bool GalaxyConfigTask::createConfigForGalaxy() {
    QString schemePathWithoutExtension = schemePath.mid(0, schemePath.length()-4 );
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

void GalaxyConfigTask::getAliases( QList <int> positions, QString &aliases ) {
    foreach( int position, positions ) {
        QStringList elementProperties = elemAliases[position].begin().value();
        QString aliasName = elementProperties.at(1);
        aliases.append( aliasName );
        aliases.append( "\r\n" );
    }
}

bool GalaxyConfigTask::createConfigForUgene() {
    QString appDirPath = QApplication::applicationDirPath();
    QFile configFile( appDirPath + "/config.txt" );
    configFile.open( QIODevice::WriteOnly );
    if ( !configFile.isOpen() ) {
        return false;
    }
    QTextStream file(&configFile);
    file << ugenePath << "\r\n";
    file << schemePath << "\r\n";
    file << outputElementsPositions.length() << "\r\n";
    QString aliases;
    getAliases( inputElementsPositions, aliases );
    getAliases( optionElementsPositions, aliases );
    getAliases( outputElementsPositions, aliases );
    file << aliases;
    configFile.close();
    return true;
}

void GalaxyConfigTask::doCopyCommands( const QString &pathToCopy ) {
    QProcess process;
    process.start( "cp " + schemePath.mid(0, schemePath.length()-4 ) + ".xml " + pathToCopy );
    process.start( "cp " + schemePath + " " + pathToCopy );
    process.start( "cp " + QApplication::applicationDirPath() +"/config.txt " + pathToCopy );
}
void GalaxyConfigTask::doDeleteCommands() {
    QProcess process;
    process.start( "rm " + QApplication::applicationDirPath() +"/config.txt" );
    process.start( "rm " + schemePath.mid(0, schemePath.length()-4 ) + ".xml" );
}

void GalaxyConfigTask::prepareDirectoryForTool() {
    QString pathToCopy = galaxyPath + "tools/" + schemeName.mid( 0, schemeName.length() - 4 );
    QString createDirCommand = "mkdir " + pathToCopy;
    doCopyCommands( pathToCopy );
    doDeleteCommands();
}
void GalaxyConfigTask::makeCopyOfGalaxyToolConfig() {
    QProcess process;
    QString toolConfiguration = galaxyPath.append( "tool_conf.xml" );
    process.start( "cp " + toolConfiguration + " " + toolConfiguration + "_old" );
}

void GalaxyConfigTask::readGalaxyToolConfig() {
    
}

void GalaxyConfigTask::addToolToGalaxyConfigFile() {
    makeCopyOfGalaxyToolConfig();
    readGalaxyToolConfig();
    /*ifstream xmlIn( galaxyConfig.c_str(), ios::in );
    bool skip = false;
    readGalaxyConfig( xmlIn, idTemp, result, resultXml, toolName, dirName, id, skip );
    xmlIn.close();
    checkSkipVariable( galaxyConfig, result, skip );*/
}

} // U2