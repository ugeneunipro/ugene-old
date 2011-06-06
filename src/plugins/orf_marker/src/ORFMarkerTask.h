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

#ifndef _U2_ORF_MARKER_TASK_H_
#define _U2_ORF_MARKER_TASK_H_


#include <QtCore/QPointer>

#include <U2Algorithm/ORFAlgorithmTask.h>
#include <U2Core/AutoAnnotationsSupport.h>

namespace U2 {

class AnnotationTableObject;
class Settings;
class DNASequence;

struct ORFSettingsKeys {
public:
    static const QString STRAND;
    static const QString AMINO_TRANSL;
    static const QString SEARCH_REGION;
    static const QString MIN_LEN;
    static const QString MUST_FIT;
    static const QString MUST_INIT;
    static const QString ALLOW_ALT_START;
    static const QString ALLOW_OVERLAP;

    static void save(const ORFAlgorithmSettings& cfg, Settings* st);
    static void read(ORFAlgorithmSettings& cfg, const Settings* st);
};

 class FindORFsToAnnotationsTask: public Task {
     Q_OBJECT
 public:
     FindORFsToAnnotationsTask(AnnotationTableObject* aobj, const DNASequence& seq, 
         const ORFAlgorithmSettings& settings);
     
     void run();
     ReportResult report();
     
 private:
     QList<SharedAnnotationData>        aData;
     QPointer<AnnotationTableObject>    aObj;
     ORFFindTask*                       fTask;
     ORFAlgorithmSettings               cfg;
     const DNASequence&                 dna;
 };
 
 
class ORFAutoAnnotationsUpdater : public AutoAnnotationsUpdater {
public:
     ORFAutoAnnotationsUpdater();
     Task* createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa);
     bool checkConstraints(const AutoAnnotationConstraints& constraints);
};



} //namespace

#endif // _U2_ORF_MARKER_TASK_

