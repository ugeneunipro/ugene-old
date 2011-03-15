/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_BOWTIE_ADAPTER_H_
#define _U2_BOWTIE_ADAPTER_H_

#include <QtCore/QObject>
#include <U2Core/GUrl.h>



namespace U2 {

class MAlignment;
class TaskStateInfo;
class BowtieReadsReader;
class BowtieReadsWriter;

class BowtieAdapter : public QObject {
Q_OBJECT
public:
	static void doBowtie(const QString& ebwtFileName, BowtieReadsReader* reader, BowtieReadsWriter* writer, const U2::GUrl& resultUrl, TaskStateInfo& ti);
	static void doBowtieWorker(int id, TaskStateInfo& ti);
	static void doBowtieBuild(const QString& infile, const QString& outfile, TaskStateInfo& ti );
};

}//namespace

#endif
