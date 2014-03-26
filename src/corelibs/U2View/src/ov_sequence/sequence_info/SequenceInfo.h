/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SEQUENCE_INFO_H_
#define _U2_SEQUENCE_INFO_H_

#include "CharOccurTask.h"
#include "DinuclOccurTask.h"
#include "DNAStatisticsTask.h"

#include <U2Core/BackgroundTaskRunner.h>
#include <U2Core/U2Region.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QtGui>
#else
#include <QtWidgets/QtWidgets>
#endif


namespace U2 {

class ADVSequenceObjectContext;
class ADVSequenceWidget;
class AnnotatedDNAView;
class LRegionsSelection;
class U2Region;
class ShowHideSubgroupWidget;


class U2VIEW_EXPORT SequenceInfo : public QWidget
{
    Q_OBJECT
public:
    SequenceInfo(AnnotatedDNAView*);

private slots:
    void sl_onSelectionChanged(LRegionsSelection*, const QVector<U2Region>& , const QVector<U2Region>&);

    /**
    * Focus is changed e.g. when a user selects another sequence or deletes the sequence in focus
    * Verifies either a region is selected on the sequence in focus.
    */
    void sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*);

    /** A sequence part was added, removed or replaced */
    void sl_onSequenceModified(ADVSequenceObjectContext*);

    /** A sequence object has been added */
    void sl_onSequenceAdded(ADVSequenceObjectContext*);

    /** Update calculated info */
    void sl_updateCharOccurData();
    void sl_updateDinuclData();
    void sl_updateStatData();

    /** A subgroup (e.g. characters occurrence subgroup) has been opened/closed */
    void sl_subgroupStateChanged(QString subgroupId);

private:
    /** Initializes the whole layout of the widget */
    void initLayout();

    /** Show or hide widgets depending on the alphabet of the sequence in focus */
    void updateLayout(); // calls the following update functions
    void updateCharOccurLayout();
    void updateDinuclLayout();

    /**  Listen when something has been changed in the AnnotatedDNAView or in the Options Panel */
    void connectSlotsForSeqContext(ADVSequenceObjectContext*);
    void connectSlots();

    /**
     * A sequence in focus should be present to call this function.
     * If regions are selected, the currentRegion is set to the first one,
     * otherwise it is set to the whole sequence.
     */
    void updateCurrentRegion();

    /**
     * Calculates the sequence (or region) length and launches other tasks (like characters occurrence).
     * The tasks are launched if:
     * 1) The corresponding widget is shown (this depends on the sequence alphabet)
     * 2) The corresponding subgroup is opened
     * The subgroupId parameter is used to skip unnecessary calculation when a subgroup signal has come.
     * Empty subgroupId means that the signal has come from other place and all required calculation should be re-done.
     */
    void launchCalculations(QString subgroupId = QString(""));

    QString  formTableRow(const QString& caption, QString value) const;

    AnnotatedDNAView* annotatedDnaView;

    ShowHideSubgroupWidget* statsWidget;
    QLabel* statisticLabel;
    BackgroundTaskRunner< DNAStatistics > dnaStatisticsTaskRunner;

    ShowHideSubgroupWidget* charOccurWidget;
    QLabel* charOccurLabel;
    BackgroundTaskRunner< QList<CharOccurResult> > charOccurTaskRunner;

    ShowHideSubgroupWidget* dinuclWidget;
    QLabel* dinuclLabel;
    BackgroundTaskRunner< QMap<QByteArray, qint64> > dinuclTaskRunner;

    U2Region currentRegion;

    static const int COMMON_STATISTICS_VALUE_MAX_WIDTH;
    static const QString CAPTION_SEQ_REGION_LENGTH;

    //nucl
    static const QString CAPTION_SEQ_GC_CONTENT;
    static const QString CAPTION_SEQ_MOLAR_WEIGHT;
    static const QString CAPTION_SEQ_MOLAR_EXT_COEF;
    static const QString CAPTION_SEQ_MELTING_TM;

    static const QString CAPTION_SEQ_NMOLE_OD;
    static const QString CAPTION_SEQ_MG_OD;

    //amino
    static const QString CAPTION_SEQ_MOLECULAR_WEIGHT;
    static const QString CAPTION_SEQ_ISOELECTIC_POINT;

    static const QString CHAR_OCCUR_GROUP_ID;
    static const QString DINUCL_OCCUR_GROUP_ID;
    static const QString STAT_GROUP_ID;
};


} // namespace

#endif
