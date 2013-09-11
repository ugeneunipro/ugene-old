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

#ifndef _U2_DAS_OPTIONS_PANEL_WIDGET_CONTROLLER_H_
#define _U2_DAS_OPTIONS_PANEL_WIDGET_CONTROLLER_H_

#include "ui/ui_DasOptionsPanelWidget.h"
#include "ui/ui_DasBlastSettingsWidget.h"

#include <U2Core/AnnotationData.h>

#include <U2Gui/RegionSelector.h>

#include <QtGui/QListWidget>

namespace U2 {

class AnnotatedDNAView;
class DNASequenceSelection;
class GetDasIdsBySequenceTask;
class ADVSequenceWidget;
class ADVSequenceObjectContext;
class DASSource;
class Task;
class CreateAnnotationWidgetController;
class LRegionsSelection;
class ShowHideSubgroupWidget;
class UniprotBlastSettings;

class DasBlastSettingsWidget : public QWidget, private Ui_DasBlastSettingsWidget {
    Q_OBJECT
public:
    DasBlastSettingsWidget(QWidget* parent = NULL);
    UniprotBlastSettings getSettings();
    double getMinIdentity() { return minimumIdentityDoubleSpinBox->value(); }
};

class DasOptionsPanelWidget : public QWidget, private Ui_DasOptionsPanelWidget {
    Q_OBJECT
public:
    DasOptionsPanelWidget(AnnotatedDNAView* adv);

private slots:
    void sl_searchTypeChanged(int type);
    void sl_searchIdsClicked();
    void sl_annotateClicked();
    void sl_exactSearchFinish();
    void sl_blastSearchFinish();
    void sl_onLoadAnnotationsFinish();
    void sl_onSequenceFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*);
    void sl_onSelectionChanged(LRegionsSelection* _selection, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    void sl_openInNewView();

private:
    void initialize();
    void connectSignals();
    void checkState();
    void updateRegionSelectorWidget();

    QList<DASSource> getFeatureSources();
    DASSource getSequenceSource();
    void mergeFeatures(const QMap<QString, QList<SharedAnnotationData> >& newAnnotations);
    void addAnnotations();
    U2Region getRegion();
    bool regionIsOk();

private:
    AnnotatedDNAView* annotatedDnaView;
    ADVSequenceObjectContext* ctx;
    DNASequenceSelection* selection;
    ShowHideSubgroupWidget* settingsShowHideWidget;
    DasBlastSettingsWidget* blastSettingsWidget;
    QListWidget* dasFeaturesListWidget;
    CreateAnnotationWidgetController* annotationsWidgetController;
    RegionSelector* regionSelector;

    QList<Task*> loadDasObjectTasks;
    QAction* fetchIdsAction;
    QAction* fetchAnnotationsAction;
    QAction* openInNewViewAction;

    QMap<QString, QList<SharedAnnotationData> > annotationData;

    const static QString EXACT_SEARCH;
    const static QString BLAST_SEARCH;
    const static QString ALGORITHM_SETTINGS;
    const static QString ANNOTATIONS_SETTINGS;
    const static QString SOURCES;
    const static QString WHOLE_SEQUENCE;
    const static QString SELECTED_REGION;
    const static QString CUSTOM_REGION;
};

}   // namespace

#endif // _U2_DAS_OPTIONS_PANEL_WIDGET_CONTROLLER_H_
