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


#include "SeqPosSettings.h"

#include <U2Core/GUrlUtils.h>

namespace U2 {

const QString SeqPosSettings::MOTIF_DB_CISTROME = "Cistrome curated motif database";
const QString SeqPosSettings::MOTIF_DB_PDM = "pbm";
const QString SeqPosSettings::MOTIF_DB_Y1H = "y1h";
const QString SeqPosSettings::MOTIF_DB_TRANSFAC = "transfac";
const QString SeqPosSettings::MOTIF_DB_HDPI = "hdpi";
const QString SeqPosSettings::MOTIF_DB_JASPAR = "jaspar";


SeqPosSettings::SeqPosSettings() {
    initDefault();
}

void SeqPosSettings::initDefault(){
    outDir = "";
    genomeAssembly = "";
    findDeNovo = false;
    motifDB = SeqPosSettings::MOTIF_DB_CISTROME;
    outName = "default.xml";
    regWidth = 600;
    pVal = 0.001f;
}

QStringList SeqPosSettings::getArguments( const QString& treatFilePath){
    QStringList result;

    result << GUrlUtils::getQuotedString(treatFilePath);

    result << genomeAssembly;

    if (findDeNovo){
        result << "-d";
    }

    if(motifDB != SeqPosSettings::MOTIF_DB_CISTROME){ //???
        result << "-m " + motifDB + ".xml";
    }

    result << "-n " + outName;

    result << "-p " + QByteArray::number(pVal);

    result << "-w " + QByteArray::number(regWidth);

    return result;
}

} // U2
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


#include "SeqPosSettings.h"

#include <U2Core/GUrlUtils.h>

namespace U2 {

const QString SeqPosSettings::MOTIF_DB_CISTROME = "Cistrome curated motif database";
const QString SeqPosSettings::MOTIF_DB_PDM = "pbm";
const QString SeqPosSettings::MOTIF_DB_Y1H = "y1h";
const QString SeqPosSettings::MOTIF_DB_TRANSFAC = "transfac";
const QString SeqPosSettings::MOTIF_DB_HDPI = "hdpi";
const QString SeqPosSettings::MOTIF_DB_JASPAR = "jaspar";


SeqPosSettings::SeqPosSettings() {
    initDefault();
}

void SeqPosSettings::initDefault(){
    outDir = "";
    genomeAssembly = "";
    findDeNovo = false;
    motifDB = SeqPosSettings::MOTIF_DB_CISTROME;
    outName = "default.xml";
    regWidth = 600;
    pVal = 0.001f;
}

QStringList SeqPosSettings::getArguments( const QString& treatFilePath){
    QStringList result;

    result << GUrlUtils::getQuotedString(treatFilePath);

    result << genomeAssembly;

    if (findDeNovo){
        result << "-d";
    }

    if(motifDB != SeqPosSettings::MOTIF_DB_CISTROME){ //???
        result << "-m " + motifDB + ".xml";
    }

    result << "-n " + outName;

    result << "-p " + QByteArray::number(pVal);

    result << "-w " + QByteArray::number(regWidth);

    return result;
}

} // U2
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


#include "SeqPosSettings.h"

#include <U2Core/GUrlUtils.h>

namespace U2 {

const QString SeqPosSettings::MOTIF_DB_CISTROME = "Cistrome curated motif database";
const QString SeqPosSettings::MOTIF_DB_PDM = "pbm";
const QString SeqPosSettings::MOTIF_DB_Y1H = "y1h";
const QString SeqPosSettings::MOTIF_DB_TRANSFAC = "transfac";
const QString SeqPosSettings::MOTIF_DB_HDPI = "hdpi";
const QString SeqPosSettings::MOTIF_DB_JASPAR = "jaspar";


SeqPosSettings::SeqPosSettings() {
    initDefault();
}

void SeqPosSettings::initDefault(){
    outDir = "";
    genomeAssembly = "";
    findDeNovo = false;
    motifDB = SeqPosSettings::MOTIF_DB_CISTROME;
    outName = "default.xml";
    regWidth = 600;
    pVal = 0.001f;
}

QStringList SeqPosSettings::getArguments( const QString& treatFilePath){
    QStringList result;

    result << GUrlUtils::getQuotedString(treatFilePath);

    result << genomeAssembly;

    if (findDeNovo){
        result << "-d";
    }

    if(motifDB != SeqPosSettings::MOTIF_DB_CISTROME){ //???
        result << "-m " + motifDB + ".xml";
    }

    result << "-n " + outName;

    result << "-p " + QByteArray::number(pVal);

    result << "-w " + QByteArray::number(regWidth);

    return result;
}

} // U2
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


#include "SeqPosSettings.h"

#include <U2Core/GUrlUtils.h>

namespace U2 {

const QString SeqPosSettings::MOTIF_DB_CISTROME = "Cistrome curated motif database";
const QString SeqPosSettings::MOTIF_DB_PDM = "pbm";
const QString SeqPosSettings::MOTIF_DB_Y1H = "y1h";
const QString SeqPosSettings::MOTIF_DB_TRANSFAC = "transfac";
const QString SeqPosSettings::MOTIF_DB_HDPI = "hdpi";
const QString SeqPosSettings::MOTIF_DB_JASPAR = "jaspar";


SeqPosSettings::SeqPosSettings() {
    initDefault();
}

void SeqPosSettings::initDefault(){
    outDir = "";
    genomeAssembly = "";
    findDeNovo = false;
    motifDB = SeqPosSettings::MOTIF_DB_CISTROME;
    outName = "default.xml";
    regWidth = 600;
    pVal = 0.001f;
}

QStringList SeqPosSettings::getArguments( const QString& treatFilePath){
    QStringList result;

    result << GUrlUtils::getQuotedString(treatFilePath);

    result << genomeAssembly;

    if (findDeNovo){
        result << "-d";
    }

    if(motifDB != SeqPosSettings::MOTIF_DB_CISTROME){ //???
        result << "-m " + motifDB + ".xml";
    }

    result << "-n " + outName;

    result << "-p " + QByteArray::number(pVal);

    result << "-w " + QByteArray::number(regWidth);

    return result;
}

} // U2
