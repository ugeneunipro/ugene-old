#include "FindRepeatsDialog.h"
//#include "FindRepeatsTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <U2Formats/GenbankFeatures.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include <QtGui/QMessageBox>
#include <math.h>

namespace U2 {

#define SETTINGS_ROOT               QString("plugin_find_repeats/")
#define MIN_LEN_SETTINGS            QString("min_len")
#define IDENTITY_SETTINGS           QString("identity")
#define MIN_DIST_SETTINGS           QString("min_dist")
#define MAX_DIST_SETTINGS           QString("max_dist")
#define MIN_DIST_CHECK_SETTINGS     QString("min_dist_check")
#define MAX_DIST_CHECK_SETTINGS     QString("max_dist_check")
#define INVERT_CHECK_SETTINGS       QString("invert_check")

FindRepeatsTaskSettings FindRepeatsDialog::defaultSettings()
{
    FindRepeatsTaskSettings res;
    Settings* s = AppContext::getSettings();
    res.minLen = (s->getValue(SETTINGS_ROOT + MIN_LEN_SETTINGS, 5).toInt());
    res.setIdentity(s->getValue(SETTINGS_ROOT + IDENTITY_SETTINGS, 100).toInt());
    bool minDistCheck = (s->getValue(SETTINGS_ROOT + MIN_DIST_CHECK_SETTINGS, true).toBool());
    bool maxDistCheck = (s->getValue(SETTINGS_ROOT + MAX_DIST_CHECK_SETTINGS, true).toBool());
    res.minDist = !minDistCheck ? 0 : (s->getValue(SETTINGS_ROOT + MIN_DIST_SETTINGS, 0).toInt());
    res.maxDist = !maxDistCheck ? 0 : (s->getValue(SETTINGS_ROOT + MAX_DIST_SETTINGS, 5000).toInt());
    res.inverted = (s->getValue(SETTINGS_ROOT + INVERT_CHECK_SETTINGS, false).toBool());
    return res;
}

FindRepeatsDialog::FindRepeatsDialog(ADVSequenceObjectContext* _sc) 
: QDialog(_sc->getAnnotatedDNAView()->getWidget()) 
{
    sc = _sc;
    setupUi(this);
    
    CreateAnnotationModel m;
    m.hideLocation = true;
    m.data->name = GBFeatureUtils::getKeyInfo(GBFeatureKey_repeat_unit).text;
    m.sequenceObjectRef = sc->getSequenceObject();
    m.useUnloadedObjects = true;
    m.sequenceLen = sc->getSequenceObject()->getSequenceLen();
    ac = new CreateAnnotationWidgetController(m, this);
    
    QWidget* caw = ac->getWidget();
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(caw);
    annotationsWidget->setLayout(l);
    annotationsWidget->setMinimumSize(caw->layout()->minimumSize());

    algoCombo->addItem(tr("Auto"), RFAlgorithm_Auto);
    algoCombo->addItem(tr("Suffix index"), RFAlgorithm_Suffix);
    algoCombo->addItem(tr("Diagonals"), RFAlgorithm_Diagonal);

    bool hasSelection = !sc->getSequenceSelection()->isEmpty();
    selectionRangeButton->setEnabled(hasSelection);
    selectionRangeButton->setChecked(hasSelection);

    int seqLen = sc->getSequenceLen();

    Settings* s = AppContext::getSettings();
    minLenBox->setValue(s->getValue(SETTINGS_ROOT + MIN_LEN_SETTINGS, qBound(5, seqLen / 100, 100)).toInt());
    minLenBox->setMaximum(seqLen);
    identityBox->setValue(s->getValue(SETTINGS_ROOT + IDENTITY_SETTINGS, 100).toInt());
    minDistBox->setValue(s->getValue(SETTINGS_ROOT + MIN_DIST_SETTINGS, 0).toInt());
    maxDistBox->setValue(s->getValue(SETTINGS_ROOT + MAX_DIST_SETTINGS, 5000).toInt());
    minDistCheck->setChecked(s->getValue(SETTINGS_ROOT + MIN_DIST_CHECK_SETTINGS, true).toBool());
    maxDistCheck->setChecked(s->getValue(SETTINGS_ROOT + MAX_DIST_CHECK_SETTINGS, true).toBool());
    invertCheck->setChecked(s->getValue(SETTINGS_ROOT + INVERT_CHECK_SETTINGS, false).toBool());

    connect(minLenHeuristicsButton, SIGNAL(clicked()), SLOT(sl_minLenHeuristics()));
    connect(hundredPercentButton, SIGNAL(clicked()), SLOT(sl_hundredPercent()));


    connect(minLenBox, SIGNAL(valueChanged(int)), SLOT(sl_repeatParamsChanged(int)));
    connect(identityBox, SIGNAL(valueChanged(int)), SLOT(sl_repeatParamsChanged(int)));

    customRangeStartBox->setMaximum(seqLen);
    customRangeEndBox->setMaximum(seqLen);
    customRangeEndBox->setValue(seqLen);
    
    QStringList annotationNames = getAvailableAnnotationNames();
    bool haveAnnotations = !annotationNames.isEmpty();
    annotationFitCheck->setEnabled(haveAnnotations);
    annotationAroundKeepCheck->setEnabled(haveAnnotations);
    annotationAroundFilterCheck->setEnabled(haveAnnotations);
    if (haveAnnotations) {
        prepareAMenu(annotationFitButton, annotationFitEdit, annotationNames);
        prepareAMenu(annotationAroundKeepButton, annotationAroundKeepEdit, annotationNames);
        prepareAMenu(annotationAroundFilterButton, annotationAroundFilterEdit, annotationNames);
    }
    
    connect(customRangeStartBox, SIGNAL(valueChanged(int)), SLOT(sl_startRangeChanged(int)));
    connect(customRangeEndBox, SIGNAL(valueChanged(int)), SLOT(sl_endRangeChanged(int)));
    connect(minDistBox, SIGNAL(valueChanged(int)), SLOT(sl_minDistChanged(int)));
    connect(maxDistBox, SIGNAL(valueChanged(int)), SLOT(sl_maxDistChanged(int)));
    connect(minDistCheck, SIGNAL(toggled(bool)), SLOT(sl_minMaxToggle(bool)));
    connect(maxDistCheck, SIGNAL(toggled(bool)), SLOT(sl_minMaxToggle(bool)));
    
    updateStatus();

    setWindowIcon(QIcon(":/ugene/images/ugene_16.png"));
}

void FindRepeatsDialog::prepareAMenu(QToolButton* tb, QLineEdit* le, const QStringList& names) {
    assert(!names.isEmpty());
    QMenu* m = new QMenu(this);
    foreach(const QString& n, names) {
        QAction* a = new SetAnnotationNameAction(n, this, le);
        connect(a, SIGNAL(triggered()), SLOT(sl_setPredefinedAnnotationName()));
        m->addAction(a);
    }
    tb->setMenu(m);
    tb->setPopupMode(QToolButton::InstantPopup);
    tb->setIcon(QIcon(":/core/images/predefined_annotation_groups.png"));
}

QStringList FindRepeatsDialog::getAvailableAnnotationNames() const {
    QStringList res;
    const QSet<AnnotationTableObject*>& objs = sc->getAnnotationObjects();
    QSet<QString> names;
    foreach(AnnotationTableObject* o, objs) {
        foreach(const Annotation* a, o->getAnnotations()) {
            names.insert(a->getAnnotationName());
        }
    }
    res = names.toList();
    res.sort();
    return res;
}

void FindRepeatsDialog::sl_setPredefinedAnnotationName() {
    SetAnnotationNameAction* a = qobject_cast<SetAnnotationNameAction*>(sender());
    QString text = a->text();
    a->le->setText(text);
}

void FindRepeatsDialog::sl_minDistChanged(int i) {
    if (i > maxDistBox->value()) {
        maxDistBox->setValue(i);
    }
    updateStatus();
}

void FindRepeatsDialog::sl_maxDistChanged(int i) {
    if (i < minDistBox->value()) {
        minDistBox->setValue(i);
    }
    updateStatus();
}

void FindRepeatsDialog::sl_startRangeChanged(int i) {
    if (i > customRangeEndBox->value()) {
        customRangeEndBox->setValue(i);
    }
    updateStatus();
}

void FindRepeatsDialog::sl_endRangeChanged(int i) {
    if (i < customRangeStartBox->value()) {
        customRangeStartBox->setValue(i);
    }
    updateStatus();
}


bool FindRepeatsDialog::getRegions(QCheckBox* cb, QLineEdit* le, QVector<U2Region>& res) {
    bool enabled = cb->isChecked();
    QString names = le->text();
    if (!enabled || names.isEmpty()) {
        return true;
    }
    QSet<QString> aNames = names.split(',', QString::SkipEmptyParts).toSet();
    const QSet<AnnotationTableObject*> aObjs = sc->getAnnotationObjects();
    foreach(AnnotationTableObject* obj, aObjs) {
        foreach(Annotation* a, obj->getAnnotations()) {
            if (aNames.contains(a->getAnnotationName())) {
                res << a->getRegions();
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

U2Region FindRepeatsDialog::getActiveRange() const {
    U2Region range(0, sc->getSequenceLen());
    if (selectionRangeButton->isChecked() && !sc->getSequenceSelection()->isEmpty()) {
        range = sc->getSequenceSelection()->getSelectedRegions().at(0);
    } else if (customRangeButton->isChecked()) {
        range.startPos = customRangeStartBox->value();
        range.length = customRangeEndBox->value() - range.startPos;
    }
    return range;
}

void FindRepeatsDialog::accept() {
    int minLen = minLenBox->value();
    int identPerc = identityBox->value();
    int minDist = minDistCheck->isChecked() ? minDistBox->value() : 0;
    int maxDist = maxDistCheck->isChecked() ? maxDistBox->value(): sc->getSequenceLen();
    bool inverted = invertCheck->isChecked();
    U2Region range = getActiveRange();
    assert(range.length > 0);
    assert(minDist <= maxDist);
    QString err = ac->validate();
    if (!err.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), err);
        return;
    }
    QVector<U2Region> fitRegions, aroundRegions, filterRegions;
    if (!getRegions(annotationFitCheck, annotationFitEdit, fitRegions) 
        || !getRegions(annotationAroundKeepCheck, annotationAroundKeepEdit, aroundRegions) 
        || !getRegions(annotationAroundFilterCheck, annotationAroundFilterEdit, filterRegions)) 
    {
        return;
    }
    
    RFAlgorithm algo = algoCheck->isChecked() ? RFAlgorithm(algoCombo->itemData(algoCombo->currentIndex()).toInt()) : RFAlgorithm_Auto;

    ac->prepareAnnotationObject();
    const DNASequence& seq = sc->getSequenceObject()->getDNASequence();
    
    FindRepeatsTaskSettings settings;
    const CreateAnnotationModel& cam = ac->getModel();
    settings.minLen = minLen;
    settings.mismatches = (100-identPerc) * minLen / 100;
    settings.inverted = inverted;
    settings.maxDist = maxDist;
    settings.minDist = minDist;
    settings.seqRegion = range;
    settings.algo = algo;
    settings.allowedRegions = fitRegions;
    settings.midRegionsToInclude = aroundRegions;
    settings.midRegionsToExclude = filterRegions;
    settings.reportReflected = false;
    settings.filterNested = !allowNestedCheck->isChecked();
    
    FindRepeatsToAnnotationsTask* t = new FindRepeatsToAnnotationsTask(settings, seq, cam.data->name, cam.groupName, cam.annotationObjectRef);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
    
    saveState();
    QDialog::accept();
}

void FindRepeatsDialog::saveState() {
    Settings* s = AppContext::getSettings();

    int minLen = minLenBox->value();
    int identPerc = identityBox->value();
    int minDist = minDistBox->value();
    int maxDist = maxDistBox->value();
    bool minDistChecked = minDistCheck->isChecked();
    bool maxDistChecked = maxDistCheck->isChecked();
    bool invertChecked = invertCheck->isChecked();

    s->setValue(SETTINGS_ROOT + MIN_LEN_SETTINGS, minLen);
    s->setValue(SETTINGS_ROOT + IDENTITY_SETTINGS, identPerc);
    s->setValue(SETTINGS_ROOT + MIN_DIST_SETTINGS, minDist);
    s->setValue(SETTINGS_ROOT + MAX_DIST_SETTINGS, maxDist);
    s->setValue(SETTINGS_ROOT + MIN_DIST_CHECK_SETTINGS, minDistChecked);
    s->setValue(SETTINGS_ROOT + MAX_DIST_CHECK_SETTINGS, maxDistChecked);
    s->setValue(SETTINGS_ROOT + INVERT_CHECK_SETTINGS, invertChecked);
}

quint64 FindRepeatsDialog::areaSize() const {
    quint64 range = getActiveRange().length;
    if (range <=0) {
        return 0;
    }
    int minDist = minDistCheck->isChecked() ? minDistBox->value() : 0;
    int maxDist = maxDistCheck->isChecked() ? maxDistBox->value(): sc->getSequenceLen();

    quint64 dRange = qMax(0, maxDist - minDist);
    
    quint64 res = range * dRange;
    return res;
}

int FindRepeatsDialog::estimateResultsCount() const {
    assert(identityBox->value() == 100);
    int len = minLenBox->value();
    
    quint64 nVariations  = areaSize(); //max possible results
    double variationsPerLen = pow(double(4), double(len));
    quint64 res = quint64(nVariations / variationsPerLen);
    res = (res > 20) ? (res / 10) * 10 : res;
    res = (res > 200) ? (res / 100) * 100 : res;
    res = (res > 2000) ? (res / 1000) * 1000 : res;
    return res;
}

void FindRepeatsDialog::sl_minLenHeuristics() {
    identityBox->setValue(100);

    // formula used here: nVariations / lenVariations = wantedResCount (==1000)
    // where nVariations == area size
    // lenVariations = 4^len where len is result
    // so we have len = ln(nVariations/wantedResCount)/ln(4)

    double nVariations  = areaSize();
    double resCount = 1000;
    double len = log(nVariations / resCount) / log(double(4));
    minLenBox->setValue((int)len);
}

void FindRepeatsDialog::sl_repeatParamsChanged(int v) {
    Q_UNUSED(v);
    updateStatus();

    minLenBox->setSingleStep(minLenBox->value() >= 20 ? 10 : 1);
}

void FindRepeatsDialog::sl_minMaxToggle(bool v) {
    Q_UNUSED(v);
    updateStatus();
}

void FindRepeatsDialog::updateStatus() {
    if (identityBox->value() == 100) {
        int r = estimateResultsCount();
        statusLabel->setText(tr("Estimated repeats count: %1").arg(r));
        statusLabel->setToolTip(tr("Estimated repeats count hint is based on the active settings and random sequence model"));
    } else {
        statusLabel->setText("");
        statusLabel->setToolTip("");
    }
}

void FindRepeatsDialog::sl_hundredPercent() {
    identityBox->setValue(100);
}

}//namespace

