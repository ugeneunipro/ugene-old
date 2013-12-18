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

#include "FindTandemsDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <U2Formats/GenbankFeatures.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include <QtGui/QMessageBox>
#include <QtGui/QLineEdit>
#include <math.h>

namespace U2 {

#define SETTINGS_ROOT               QString("plugin_find_repeats/")
#define MIN_LEN_SETTINGS            QString("min_len")
#define IDENTITY_SETTINGS           QString("identity")

FindTandemsTaskSettings FindTandemsDialog::defaultSettings()
{
    FindTandemsTaskSettings res;
    Settings* s = AppContext::getSettings();
    res.minPeriod = (s->getValue(SETTINGS_ROOT + MIN_LEN_SETTINGS, 1).toInt());
    return res;
}

FindTandemsDialog::FindTandemsDialog(ADVSequenceObjectContext* _sc) 
: QDialog(_sc->getAnnotatedDNAView()->getWidget()) 
{
    sc = _sc;
    setupUi(this);
    tabWidget->setCurrentIndex(0);

    CreateAnnotationModel m;
    m.hideLocation = true;
    m.data.name = GBFeatureUtils::getKeyInfo(GBFeatureKey_repeat_unit).text;
    m.sequenceObjectRef = sc->getSequenceObject();
    m.useUnloadedObjects = true;
    m.sequenceLen = sc->getSequenceObject()->getSequenceLength();
    ac = new CreateAnnotationWidgetController(m, this);
    
    QWidget* caw = ac->getWidget();
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(caw);
    annotationsWidget->setLayout(l);
    annotationsWidget->setMinimumSize(caw->layout()->minimumSize());

    algoComboBox->addItem(tr("Suffix array"), TSConstants::AlgoSuffix);
    algoComboBox->addItem(tr("Suffix array (optimized)"), TSConstants::AlgoSuffixBinary);
    algoComboBox->setCurrentIndex(TSConstants::AlgoSuffixBinary);

    repeatLenComboBox->addItem(tr("All"), TSConstants::PresetAll);
    repeatLenComboBox->addItem(tr("Micro-satellites"), TSConstants::PresetMicro);
    repeatLenComboBox->addItem(tr("Mini-satellites"), TSConstants::PresetMini);
    repeatLenComboBox->addItem(tr("Big-period tandems"), TSConstants::PresetBigPeriod);
    repeatLenComboBox->addItem(tr("Custom"), TSConstants::PresetCustom);
    repeatLenComboBox->setCurrentIndex(TSConstants::PresetAll);

    int seqLen = sc->getSequenceLength();

    rs=new RegionSelector(this, seqLen, false, sc->getSequenceSelection());
    rangeSelectorLayout->addWidget(rs);

    setWindowIcon(QIcon(":/ugene/images/ugene_16.png"));
}

QStringList FindTandemsDialog::getAvailableAnnotationNames() const {
    QStringList res;
    const QSet<AnnotationTableObject *> objs = sc->getAnnotationObjects();
    QSet<QString> names;
    foreach ( const AnnotationTableObject *o, objs ) {
        foreach ( const Annotation &a, o->getAnnotations( ) ) {
            names.insert( a.getName( ) );
        }
    }
    res = names.toList();
    res.sort();
    return res;
}

void FindTandemsDialog::minPeriodChanged(int min){
    maxPeriodBox->setValue(qMax(min,maxPeriodBox->value()));
}
void FindTandemsDialog::maxPeriodChanged(int max){
    minPeriodBox->setValue(qMin(max,minPeriodBox->value()));
}
void FindTandemsDialog::customization(){
    repeatLenComboBox->setCurrentIndex(TSConstants::PresetCustom);
}
void FindTandemsDialog::presetSelected(int preset){
    int minPeriod = 1;
    int maxPeriod = INT_MAX;
    const unsigned maxMicro=6;
    const unsigned maxMini=30;
    switch (preset){
        case TSConstants::PresetAll:
            break;
        case TSConstants::PresetMicro:
            maxPeriod=maxMicro;
            break;
        case TSConstants::PresetMini:
            minPeriod=maxMicro+1;
            maxPeriod=maxMini;
            break;
        case TSConstants::PresetBigPeriod:
            minPeriod=maxMini+1;
            break;
        case TSConstants::PresetCustom:
            return;
        default:
            break;
    }
    minPeriodBox->setValue(minPeriod);
    maxPeriodBox->setValue(maxPeriod);
}

bool FindTandemsDialog::getRegions(QCheckBox* cb, QLineEdit* le, QVector<U2Region>& res) {
    bool enabled = cb->isChecked();
    QString names = le->text();
    if (!enabled || names.isEmpty()) {
        return true;
    }
    QSet<QString> aNames = names.split(',', QString::SkipEmptyParts).toSet();
    const QSet<AnnotationTableObject *> aObjs = sc->getAnnotationObjects();
    foreach ( AnnotationTableObject *obj, aObjs ) {
        foreach ( const Annotation &a, obj->getAnnotations( ) ) {
            if ( aNames.contains( a.getName( ) ) ) {
                res << a.getRegions( );
            }
        }
    }
    if (res.isEmpty()) {
        le->setFocus();
        QMessageBox::critical(this, tr("Error"), tr("No annotations found: %1").arg(names));
        return false;
    }
    return true;
}

U2Region FindTandemsDialog::getActiveRange(bool *ok) const {
    U2Region region=rs->getRegion(ok);//todo add check on wrong region
    return region;
}

void FindTandemsDialog::accept() {
    int minPeriod = minPeriodBox->value();
    int maxPeriod = maxPeriodBox->value();
    bool isRegionOk=false;
    U2Region range = getActiveRange(&isRegionOk);
    if(!isRegionOk){
        rs->showErrorMessage();
        return;
    }
    assert(range.length > 0);
    QString err = ac->validate();
    if (!err.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), err);
        return;
    }

    DNASequence seq = sc->getSequenceObject()->getSequence(range);
    if (seq.isNull() || !seq.alphabet) {
        QMessageBox::warning(this, tr("Error"), tr("Not enough memory error ocurred while preparing data. Try to set smaller region."));
        return;
    }
    bool objectPrepared = ac->prepareAnnotationObject();
    if (!objectPrepared){
        QMessageBox::warning(this, tr("Error"), tr("Cannot create an annotation object. Please check settings"));
        return;
    }

    FindTandemsTaskSettings settings;
    const CreateAnnotationModel& cam = ac->getModel();
    settings.minPeriod = minPeriod;
    settings.maxPeriod = maxPeriod;
    settings.algo = (TSConstants::TSAlgo)algoComboBox->currentIndex();
    settings.minRepeatCount = minRepeatsBox->value();
    settings.minTandemSize = qMax(FindTandemsTaskSettings::DEFAULT_MIN_TANDEM_SIZE, minTandemSizeBox->value());
    settings.showOverlappedTandems = overlappedTandemsCheckBox->isChecked();

    settings.seqRegion = U2Region(0, seq.length());
    settings.reportSeqShift = range.startPos;

    FindTandemsToAnnotationsTask* t = new FindTandemsToAnnotationsTask(settings, seq, cam.data.name, cam.groupName, cam.annotationObjectRef);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
    
    QDialog::accept();
}

quint64 FindTandemsDialog::areaSize() const {
    quint64 range = getActiveRange().length;
    if (range <=0) {
        return 0;
    }
    int minDist = 0;
    int maxDist = sc->getSequenceLength();

    quint64 dRange = qMax(0, maxDist - minDist);
    
    quint64 res = range * dRange;
    return res;
}

int FindTandemsDialog::estimateResultsCount() const {
    int len = 1;
    
    quint64 nVariations  = areaSize(); //max possible results
    double variationsPerLen = pow(double(4), double(len));
    quint64 res = quint64(nVariations / variationsPerLen);
    res = (res > 20) ? (res / 10) * 10 : res;
    res = (res > 200) ? (res / 100) * 100 : res;
    res = (res > 2000) ? (res / 1000) * 1000 : res;
    return res;
}

}//namespace

