#include "AssemblyBrowser.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QScrollBar>

#include <U2Core/AppContext.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2AssemblyUtils.h>

#include <U2Misc/DialogUtils.h>

#include "AssemblyOverview.h"
#include "AssemblyReferenceArea.h"
#include "AssemblyDensityGraph.h"
#include "AssemblyRuler.h"
#include "AssemblyReadsArea.h"

namespace U2 {

const double AssemblyBrowserWindow::ZOOM_MULT = 1.25;

//==============================================================================
// AssemblyModel
//==============================================================================

AssemblyModel::AssemblyModel() : cachedModelLength(NO_VAL), cachedModelHeight(NO_VAL), referenceDbi(0) {
}


bool AssemblyModel::isEmpty() const {
    assert(assemblies.isEmpty() == assemblyDbis.isEmpty());
    return assemblies.isEmpty() || assemblyDbis.isEmpty(); 
}

QList<U2AssemblyRead> AssemblyModel::getReadsFromAssembly(int assIdx, const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os) {
    return assemblyDbis.at(assIdx)->getReadsByRow(assemblies.at(assIdx).id, r, minRow, maxRow, os);
}

qint64 AssemblyModel::countReadsInAssembly(int assIdx, const U2Region & r, U2OpStatus & os) {
    return assemblyDbis.at(assIdx)->countReadsAt(assemblies.at(assIdx).id, r, os);
}

qint64 AssemblyModel::getModelLength(U2OpStatus & os) {
    if(NO_VAL == cachedModelLength) {
        //TODO: length must be calculated as length of the reference sequence or
        //if there is no reference, as maximum of all assembly lengths in the model
        qint64 refLen = hasReference() ? reference.length : 0;
        qint64 assLen = assemblyDbis.at(0)->getMaxEndPos(assemblies.at(0).id, os);
        cachedModelLength = qMax(refLen, assLen);
    }
    return cachedModelLength;
}

qint64 AssemblyModel::getModelHeight(U2OpStatus & os) {
    if(NO_VAL == cachedModelHeight) {
        // TODO: get rid of this? Use predefined max value?
        qint64 zeroAsmLen = assemblyDbis.at(0)->getMaxEndPos(assemblies.at(0).id, os); 
        //TODO: model height should be calculated as sum of all assemblies ? 
        //Or consider refactoring to getHeightOfAssembly(int assIdx, ...)
        cachedModelHeight = assemblyDbis.at(0)->getMaxPackedRow(assemblies.at(0).id, U2Region(0, zeroAsmLen), os);
        return cachedModelHeight;
    }
    return cachedModelHeight;
}

void AssemblyModel::addAssembly(U2AssemblyRDbi * dbi, const U2Assembly & assm) {
    //TODO: == operator for U2Assembly
    assert(!assemblyDbis.contains(dbi));
    //assert(!assemblies.contains(assm));
    assemblyDbis.push_back(dbi);
    assemblies.push_back(assm);
}

bool AssemblyModel::hasReference() const {
    return (bool)referenceDbi;
}

void AssemblyModel::setReference(U2SequenceRDbi * dbi, const U2Sequence & seq) {
    //TODO emit signal ??
    reference = seq;
    referenceDbi = dbi;
}

QByteArray AssemblyModel::getReferenceRegion(const U2Region& region, U2OpStatus& os) {
    return referenceDbi->getSequenceData(reference.id, region, os);
}

//==============================================================================
// AssemblyBrowserWindow
//==============================================================================

AssemblyBrowserWindow::AssemblyBrowserWindow() : MWMDIWindow("AssemblyBrowser"), ui(0),
model(new AssemblyModel), zoomFactor(1.), xOffsetInAssembly(0), yOffsetInAssembly(0) {
    initFont();
    setupActions();
    updateActions();

    sl_loadAssembly();
}

AssemblyBrowserWindow::~AssemblyBrowserWindow() {
    delete model;
}

void AssemblyBrowserWindow::setupMDIToolbar(QToolBar* tb) {
    tb->addAction(openAssemblyAction);
    tb->addAction(zoomInAction);
    tb->addAction(zoomOutAction);
}

void AssemblyBrowserWindow::setupViewMenu(QMenu* n) {
    n->addAction(openAssemblyAction);
    n->addAction(zoomInAction);
    n->addAction(zoomOutAction);
}

int AssemblyBrowserWindow::getCellWidth() const {
    return calcPixelCoord(1);
}

qint64 AssemblyBrowserWindow::calcAsmCoord(qint64 xPixCoord) const {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 width = ui->getReadsArea()->width();
    qint64 xAsmCoord = (double(modelLen) / width * getZoomFactor() * double(xPixCoord)) + 0.5;
    return xAsmCoord;
}

qint64 AssemblyBrowserWindow::calcPixelCoord(qint64 xAsmCoord) const {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 width = ui->getReadsArea()->width();
    qint64 xPixelCoord = (double(width) / modelLen * double(xAsmCoord)) / getZoomFactor() + 0.5;
    return xPixelCoord;
}

qint64 AssemblyBrowserWindow::basesCanBeVisible() const {
    int width = ui->getReadsArea()->width();
    qint64 letterWidth = getCellWidth();
    if(0 == letterWidth) {
        return calcAsmCoord(width);
    }
    qint64 result = width / letterWidth + !!(width % letterWidth);
    return result;
}

qint64 AssemblyBrowserWindow::basesVisible() const {
    U2OpStatusImpl status;
    qint64 modelLength = model->getModelLength(status);
    return qMin(modelLength, basesCanBeVisible());
}

qint64 AssemblyBrowserWindow::rowsCanBeVisible() const {
    int height = ui->getReadsArea()->height();
    qint64 letterWidth = getCellWidth();
    if(0 == letterWidth) {
        return calcAsmCoord(height);
    }
    qint64 result = height / letterWidth + !!(height % letterWidth);
    return result;
}

qint64 AssemblyBrowserWindow::rowsVisible() const {
    U2OpStatusImpl status;
    qint64 modelHeight = model->getModelHeight(status);
    return qMin(rowsCanBeVisible(), modelHeight);
}

bool AssemblyBrowserWindow::areReadsVisible() const {
    int readWidthPix = calcPixelCoord(1); // TODO: average read length ? 
    return readWidthPix >= 1;
}

bool AssemblyBrowserWindow::areCellsVisible() const {
    return getCellWidth() >= CELL_VISIBLE_WIDTH;
}

bool AssemblyBrowserWindow::areLettersVisible() const {
    return getCellWidth() >= LETTER_VISIBLE_WIDTH;
}

void AssemblyBrowserWindow::setXOffsetInAssembly(qint64 x) {
    xOffsetInAssembly = x;
    emit si_offsetsChanged();
}

void AssemblyBrowserWindow::setYOffsetInAssembly(qint64 y) {
    yOffsetInAssembly = y;
    emit si_offsetsChanged();
}

void AssemblyBrowserWindow::adjustOffsets(qint64 dx, qint64 dy) {
    U2OpStatusImpl status;
    qint64 modelLen = model->getModelLength(status);
    qint64 modelHeight = model->getModelHeight(status);

    xOffsetInAssembly = qMax(qMin(xOffsetInAssembly + dx, modelLen - basesVisible()), qint64(0));
    yOffsetInAssembly = qMax(qMin(yOffsetInAssembly + dy, modelHeight - rowsVisible()), qint64(0));

    emit si_offsetsChanged();
}

void AssemblyBrowserWindow::sl_loadAssembly() {
    dbi = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi")->createDbi(); 
    assert(U2DbiState_Void == dbi->getState());

    //TODO stub
    U2OpStatusImpl os;
    QHash<QString, QString> props;
    //props["url"] = "E:/BT474_dir75.sqlite";
    props["url"] = "E:/MCF7_dir100.sqlite";
    //props["url"] = "E:/example-alignment.sqlite";
    dbi->init(props, QVariantMap(), os);
    checkAndLogError(os);

    //dbi->getAssemblyRWDbi()->pack(1, os);

    if(!checkAndLogError(os)) {
        sl_assemblyLoaded();

        U2Dbi * sdbi = AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi")->createDbi();
        U2OpStatusImpl os;
        QHash<QString, QString> props;
        props["url"] = "E:/test_ref.sqlite";
//#define CREATE_REF
#ifdef CREATE_REF
        QFile::remove("E:/test_ref.sqlite");
        props["create"] = "1";
#endif
        sdbi->init(props, QVariantMap(), os);
        checkAndLogError(os);
#ifdef CREATE_REF
        U2SequenceRWDbi * seqDbi = sdbi->getSequenceRWDbi();
        sdbi->getFolderDbi()->createFolder("/", os);
        checkAndLogError(os);
        U2Sequence s;
        seqDbi->createSequenceObject(s, "/", os);
        checkAndLogError(os);
        extern QByteArray example_sequence;
        seqDbi->updateSequenceData(s.id, U2Region(), example_sequence, os);
        checkAndLogError(os);

        QByteArray seq = seqDbi->getSequenceData(s.id, U2Region(0, 9996), os);
        checkAndLogError(os);
#else
        U2DataId refId = sdbi->getObjectRDbi()->getObjects(U2Type::Sequence, 0, -1, os).at(0);
        checkAndLogError(os);
        U2Sequence s = sdbi->getSequenceRDbi()->getSequenceObject(refId, os);
        checkAndLogError(os);
#endif
        model->setReference(sdbi->getSequenceRDbi(), sdbi->getSequenceRDbi()->getSequenceObject(s.id, os));
        checkAndLogError(os);
    }
}

void AssemblyBrowserWindow::sl_assemblyLoaded() {
    U2OpStatusImpl status;
    checkAndLogError(status);
    assert(U2DbiState_Ready == dbi->getState());

    U2AssemblyRDbi * assmDbi = dbi->getAssemblyRDbi();

    U2DataId assmId = dbi->getObjectRDbi()->getObjects(U2Type::Assembly, 0, -1, status).at(0);
    checkAndLogError(status);
    U2Assembly assm = dbi->getAssemblyRDbi()->getAssemblyObject(assmId, status);
    checkAndLogError(status);

    model->addAssembly(assmDbi, assm);
    createWidgets();
    ui->update();
}

void AssemblyBrowserWindow::sl_zoomIn() {
    int oldCellSize = getCellWidth();
    if(!oldCellSize) {
        zoomFactor /= ZOOM_MULT;
    } else {
        double oldZoomFactor = zoomFactor;
        int cellWidth = 0;
        do {
            zoomFactor /= ZOOM_MULT;
            cellWidth = getCellWidth();
        } while(oldCellSize == cellWidth);
        if(cellWidth > MAX_CELL_WIDTH) {
            zoomFactor = oldZoomFactor;
        } 
    }

    updateActions();
    emit si_zoomOperationPerformed();
}

void AssemblyBrowserWindow::sl_zoomOut() {
    int oldCellSize = getCellWidth();
    if(zoomFactor * ZOOM_MULT > 1.) {
        zoomFactor = 1.;
    } else if(!oldCellSize) {
        zoomFactor *= ZOOM_MULT;
    } else {
        do {
            zoomFactor *= ZOOM_MULT;
        } while(oldCellSize && getCellWidth() == oldCellSize);
    }
    updateActions();
    emit si_zoomOperationPerformed();
}

void AssemblyBrowserWindow::createWidgets() {
    ui = new AssemblyBrowserUi(this);
    QLayout * l = new QVBoxLayout(this);
    l->setMargin(1);
    l->addWidget(ui);
    setLayout(l);
}

void AssemblyBrowserWindow::initFont() {
    font.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
}

void AssemblyBrowserWindow::setupActions() {
    openAssemblyAction = new QAction(tr("Load assembly"), this);
    connect(openAssemblyAction, SIGNAL(triggered()), SLOT(sl_loadAssembly()));

    zoomInAction = new QAction(tr("Zoom in"), this);
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomIn()));

    zoomOutAction = new QAction(tr("Zoom out"), this);
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOut()));
}

void AssemblyBrowserWindow::updateActions() {
    zoomOutAction->setEnabled(1. != zoomFactor);
}

//==============================================================================
// AssemblyBrowserUi
//==============================================================================

AssemblyBrowserUi::AssemblyBrowserUi(AssemblyBrowserWindow * window_) : window(window_) {
    setMinimumSize(300, 200);

    QScrollBar * readsHBar = new QScrollBar(Qt::Horizontal);
    QScrollBar * readsVBar = new QScrollBar(Qt::Vertical);

    overview = new AssemblyOverview(this);        
    referenceArea = new AssemblyReferenceArea(this);
    densityGraph = new AssemblyDensityGraph(this);
    ruler = new AssemblyRuler(this);
    readsArea  = new AssemblyReadsArea(this, readsHBar, readsVBar);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(2);
    mainLayout->addWidget(overview);

    QGridLayout * readsLayout = new QGridLayout();
    readsLayout->setMargin(0);
    readsLayout->setSpacing(0);

    //    readsLayout->addWidget(densityGraph);
    readsLayout->addWidget(referenceArea, 0, 0);
    readsLayout->addWidget(ruler, 1, 0);

    readsLayout->addWidget(readsArea, 2, 0);
    readsLayout->addWidget(readsVBar, 2, 1, 2, 1);
    readsLayout->addWidget(readsHBar, 3, 0);

    QWidget * readsLayoutWidget = new QWidget;
    readsLayoutWidget->setLayout(readsLayout);
    mainLayout->addWidget(readsLayoutWidget);

    setLayout(mainLayout);  

    connect(readsArea, SIGNAL(si_heightChanged()), overview, SLOT(sl_visibleAreaChanged()));
}

bool checkAndLogError(const U2OpStatusImpl & status) {
    if(status.hasError()) {
        uiLog.error(AssemblyBrowserWindow::tr(QString("Assembly Browser -> Database Error: " + status.getError()).toAscii().data()));
    }
    return status.hasError();
}

qint64 countReadLength(qint64 realLen, const QList<U2CigarToken> & cigar) {
    return realLen + U2AssemblyUtils::getCigarExtraLength(cigar);
}

QByteArray getReadSequence(U2Dbi * dbi, const U2AssemblyRead & read, U2OpStatus & os) {
    if(read.sequenceId) {
        assert(dbi);
        U2Sequence seq = dbi->getSequenceRDbi()->getSequenceObject(read.sequenceId, os);
        return dbi->getSequenceRDbi()->getSequenceData(read.sequenceId, U2Region(0, seq.length), os);
    } else {
        return read.readSequence;
    }
}



} //ns

QByteArray example_sequence("GGCACGAAAATTTTTTAAATAAAATGCATTTTCTAGATACTACCGCGCAATCACTCCCAG\
GGAGTACGTTCGGAAAAGAAAGAGCGTTTGGTGACAAGTTATACGTACATCTCTTGCGCA\
TTTTTTTAACCCGTAGAGAACGGCAGTACGCGGCCCTGCCAAGCCGGTTCAATTGACCTC\
TCAGGATAACACTTGACTCCCAATGGTGACCAGGCCCACCACCGCTACGACGTCACATCA\
TACTTCGCCGCGTACCATCCTCGCAACCTGGTCCACTGTTCCTGCACTAAAGCTGGAACT\
TATTGTCGTGCAGCCCTACGTAGGATGGAATCAATACAAAACTCCCAACCCGGTCCTAGC\
TTCATATGGAAACCGAAAGCATGCGCTACGGACTTCAACTAACGAACAATGTGTCAACGT\
GCGCTATTGGGTAGTTCATCGATACCCAGAGGTTCCGTACTCAGCGGGATCCGGCGCGGC\
CCTCAGGTATCATCCTAAATGACAAATTAAAGCGAGAAGAGATTCGCGGCGAACACGGAA\
GAGCGCTTCGGAAGGAATACGTATTAAGCACTGATCGGCCCAGTGCAGGCTATCTTCCGA\
TACCTGTCACTTACGGGCGGTTTATCCTCTCGTATTCGCCAATCTGTGGTAGTGAGACAT\
GCAACTACGATTGACCATCCCAGAAGCCGCTGTGCCTAGGCCACGGTACCCGAGAAATTC\
ATCCTCTGACTCTGTTACGTATACGCCAGTATTTAATTCTCTTTGTCCCCCGGCCTGTCC\
GCATACGGTTAAAGCGACGCGTTACATCTGTGACTAAATTCCACTGGTATAAGGGCCCCT\
GTTTAGCCTTGCAGCATCCGCACGTCCGCGCTACAACCCGACAGGCAGCGCGTCCAAGCG\
TCCTTAGAGTTACAGCGGCCACAGCACATGATGGGCGAGCCATTGGATCGGGGGAGGAAT\
CCGGTACTGGACACTATGTGGCCTGACCATAACGGGTCAATCCTCCCATGGGCGGGTGGA\
CGACGCCAATATCGTCGCTTTCCTGTCCCTTCGCCCCCTGACCTATATGCCAAGCCACAA\
GCAGGTCCCGGTGCTGCAATGATGAACCACTGCAACATAGATGCGAGCATTCGAAGAACT\
TCGATCCCAATTGTTAATCTTGGCGGGATGTATAGACTACCTGGTTGGCTGACGACGGGA\
CAAACGCGAGTCGGATCGTACCTGTGTTACTTGGGATGCGGCGTGTGCATAGTAACGGGG\
ACGTGGCGTCACTCAGTGTTGTCTTGCGGTCTCAAACTTTTGGGTGACATGGGATAAGCT\
CGAGTCACTAAGTCCAACATGATCCCAGTAACGGTGCAACTTACCACGTACCGCCAACAT\
ATGCTGTGATCGATTCGAAGCTCCAACGGGCGCGTTAGGGGGTTAGCTTGCTCGACGTCC\
ATTCTGAGTACGGGGCGCTTTCTTCTATATCGAGGTCACATGCTCTAACGCGCGCTCAGG\
TCTATTTCCGAATAGCTGGACGCATTTGATCTACCGTATGGCTACTCGGTTACGGACCCA\
GCTGCGCATGGGTATAGACATAGCACTCGGCGCTTCGGAGGTCGATCGGAGGAATCTGGG\
GCAGTTTCGTGGACAGGTACGCTGCCCCACGCTCGGCTGGCGTCCGTACTAGAGAACTAG\
CTTAAATTAATTTGCGGCATAGAGAAAGCCTGCGTACCAGAGATATGATAGCAAAACGAC\
GTCATGCGTGCTGGTTCGTTCACCGCTAACACACCCGGTCCGCAGGTTGGTAAGGCAGTA\
ACCGGATCGCCGATTAACTCCGGTCCGCCTGCTGCCCTTTTATTGCCCTCGGCGTAGCGA\
GCTTGCGAGTATCGCGCCATCATGCTTCTTCACTGTCGAAGCTGCCGAAATAAGAGTCCC\
GAAATTGTTCTTGGTTCGGATGTACGATAGTGTCGTCCCACATATGTTTGAAGGAAGGGA\
AGGTGCACAGCCTCTTGCTGAATGCTTTTCTTCTCTACTCAGTCGGATTTATGTCTCGTC\
TCTTACCACCCCGGCTGTGAGAGATCTGAATGGGCCCCCCATTTGTCTCGGAGTATGTAC\
GACGATTATGCCGGAATTAGTCAATCAAGCTGACCTTCTCCCTTTGCGGGAGTGGCAGGG\
GTAACAGTGCTGAACACTGCGATCAGCGCAAGATCGCTCACACAAAAGGCCCAATTCCGT\
GAATGCAGTCCGTTCGGCCCACCAACACAGATCTTTGTGAGTTCAGGCAGAATGGGGGTG\
AAATGTATCGATATAGTGAGCCGCCACCATCTCTTTGGGATCAACATCGCGTAGCCGCAT\
CCGAGCTCTATGGGAAGACCGCCTGCAACCTATATCCCAGCTCAATATCGTTTCTATACC\
TCGGATCACCCATAGGGAATCAGCTTACCGCAATCCGGTTATCGTTGTTGGATAGACATT\
AAAACGGTCGGTAAATGTTAACAGTATTCACAAACGTTAATCGCAAGAGCGTGAAGATCA\
CATCGGGGTGGATCTCTCCGTGAAAGTTTGTCGAAGAACGGATTTTTTATGCGTGTATCG\
GCCGCTACTAATGGTAGCCGCAGTCCACTTGCCCAGCTCCAATACCTTTGTCCACGGTAC\
CAGAGTTCGCGTAAAGGACGGCCTTATCTGTGAGTCTTCTAGTAAGAAGGTTTACCAAGC\
TAACAACCCGGCATAGTAAAAGAGAACTCAGGAGACTCTTCAATAACCCTCTGATTAGTC\
TCGGTAAACGAGAGATTCCGCGTGATCTTGAGTTTGTATCTCTACAGCTAATCTTTTGTC\
GGACTTTCTATTCTATGTCGAGCGTACGTTCGAGTAACAGGACGACCTGAACATCGCAAA\
CCCGATGAAGAGATCTGGTAGAAACACTGGTACACGTTTGTGCACAGTACCTGTCCAGGA\
AACTCCCTGTACAGGTGAACCTGGCCAGACTTTCGGATAGACCTTGCCATCTTATATGGG\
TATAGTCTTGGCCACCCTAGAACCTCGGACCTCGCGCGATACGAACTCGATCATCAGCGT\
GTTAAACTTGAGCCTGCGCTGGGCAAATCCATGGTTCTAGGCCTATCGATAAGATATCAT\
CGCCCACTGACCCCTACGCTTCAAGTATTTACACATATACGAGCAAGGGGAAAACCTTGC\
GGAAGTCACGTATTCGATGCGTATAGCGACTAAGGTATGAATCATACGTTGCGCGTCGCG\
ACTCAGAGAAAGCGAGTAGGCGTAGGATTAGGTGGGCTCGTTAGCATTCTTCAAAACCTT\
CCTAATCACATGATGCGATTGTACAGTTGACAGCACCCCATGTCATATGTCAAGGTCGCA\
GGTCATGTCCCACTTGAGCTGTTACAGCAGGAGATCAGCGTACCCAAAACGGCGTCTGAA\
ATTTGTAGAAGCAGACCAGAAAGCAAATGAAGACGACTGCGGTAACGCTGCGCCCTATGT\
GCCCTCGGTCACGGTCGACAATATGAAGCATTGAATCTTTTCATCACTCCTGTTCAGCGA\
GCTTCCCGCTGGGCACTTAATCAGCCTGCGGACTATCCTTATCTAGCTACCTAAAATCGG\
TTCACCAATGCAATGTCTTGCCGCTATCCCTTATTGAATGACGGCTTGTGGTGCCTAGTA\
GGCTTCTTGTGGGCCGAATGTAGAGAGTCATCGCTTCATGCTGTGTAGTTGGCACCTTAC\
CCAGCGGGTGGTCTATCTAAGGGGGGCGCAGCCTCAGCCGAAACGACGTAGCCGTCGAAC\
CATATAGTGATATCGAAGATCGCTTAGGCTCCGCCACGACTAGTACCTCGAACTTTTGAA\
ACAAGGTGTGTCATTGGCTTCTAATAGGCGGTAAGTGTCCCGGATATAGAACCAGACACC\
CGTCGTTTTTCCGCGCGGTAAAGCTCTAGCTTAGAGTCCTGCCAGGGGATTTACTGATGA\
AACCCCATTAGCAACATGACTCTCGCTCTACTTCGGACCAGTCTAATTCGCGTTAACCAA\
CCTGACGACCACAATGGCGGATAGACCCGGGGTTTAGCCACTGCATAGATCGCATCACCT\
AGTTGGAAAGGTGTGAGTCGGGGAGATCGCTCGACCAGTTCCGTCTACTGTTACGGGGCT\
AACCGTTTTCTAAGGAATCTAGTAATAGGACGCGGCCGCAAGTAGGTCTGATGTTAGCGT\
ATACGTATTGTCGGGCGACCACCCAGCGATATATGTAAGGGGTGAAAAGCAAAATGAGAG\
CACAGATGTGCGACTCCGACGTCGCTTAGCAGAGAATGGCAAGAAAATAGTGGTTAAGAC\
GGCACGAATTATCAACGTCCTGTCGGATTATTCCGACTGTTTCCATGTAGACCATGACTT\
ATCAATACACACCTACTTGTCGAAAGTCTACGCGACGTACTGATTCGGTCATTGAGCCCC\
GTGGATACCACACCACCCTCAGCTATGGGCTTTGTCGGTCGGGTCTGGAGACTATTCCGC\
AAGAAATAGGGAAGTATATGACCTCTAGCATCTCGAGTACGCTGTTCAAGGTTAAAAGGT\
GAAAGCTGGGACGGGTCCAATCGATTGCGGCGATAAAGACTAGGACATCCACGATCTCAA\
GCGACTGACGCTGCACGTACCCGTCCGCGAACTCAAATGACTCACGCATGATAATCCGCT\
TAAAGTGCAACAGGCTTGACCATCTACATTTGCAAGCAACACTGACACGTCTTCCGATGA\
AGATCCTTTGTAGCTGTTCTTGCGGTGCGCGAATTTGAGGACTGCCGGTCCGAACTTCTG\
GTGATCCTACGGATAGACATGGCAATAGACGCCCCATCCAAATTGTTCCTGCAAATAGCG\
ATAGGCGATACGAGCTTTTTGATGCTTGGCTATCACAATCGAGACCTACCCAGAGGTCCG\
CTCGGTGACGATTGTGAATACAACTTTTGTGTGTTTCCATCACTGGGGGGGCTGCAGCAG\
CCGCTTAAGGGGCATAAGCTTGTATTTTCTCTAAGGCGGGTATCATGCCATAAGGTGATA\
AGCGCACCCCCGCCCTGCCGATACATGGGCAGTTGTCTAATGTGCACTGAAGCGTTTCTA\
TTTTAACGCTGGCTAGTGTAAATGATCGGAGAAGGAGGATGTGCAAACCACCTTGGCCTT\
CAAGCCCGGGATAAGTATCAGAGGCTAACTGAGGTCTTTGGCCAGAACTGGATATACCGT\
ACCGGCACTGCCAAAAGGCTCTCATCAATGAGAGTTGTGAGTCAATGCTCAATGCTGCGG\
CGTAGTTCTTGATGTATTCAACAATCGTAATAATGTATCGTACCATGTTGAATCTATATC\
TTCATCAACGCCAATGCGCCATACACTGACGCAGCGAGCCGCCTTCATACTTTGTTAGCA\
CGGCTCCACACATATGGTCCGACTCTAATGAAGATCCTCATGTTATATCGGCAGTGGGTT\
GATCAATCCACGTGGATAGATGCCCCCCGAGCCACGAAAACCGGGCACTTGTAACTCCAG\
GCTCAACTATGCGACGGACGGGGTCTTGTTACCGCCACTTGAGACTGTAAAGAGATATCG\
TCCCTCTGTGAATCCTCAAGAATTCCAAGCCTAGTCTTGGCGGTGGTTACCGCCTCCTTA\
TACTCATTATGAAGACGGTGCGGGATCGCCCCTGGTATTTAGGTCTACGCTAACAACCAC\
TTCTTTGGGGACGCCTCGGGGACTGACAGATAATGATCATGGCCCAAGCTATATACGTGT\
GTCTCGGGGCCTCTCCGTATACGATTTAGGCCGAGGACTTGGAATACGGCTTGGCAACCC\
CACGGCATGGTATCCAAGGGAAGAGACCGATAAATTACCCGCGCTGTCGAACTGCGATAA\
GATTTTTTATTCAGCGTCAATACGAATAGCTTATCGGAGGAAGAGTTAAAGTTTGCTAGC\
GAGTTTTCTTGACTCAGTGGGCGGGGGTACAATATGAGCTCTCCTGTTAATCCAATCTTT\
AGAAGGAATTTGTGCCCTCTTGTCAAATACCTGTGAGGTCTAACGAATATTGGCACATTG\
TGGCTGCCTCGCCGGCCGCTTCCTCAGATTAGGTACGCCGGAATCGGTCAAAGTCCTTCG\
CCTTAGTTCCGCATAACTACTTCAGGGAAGCTACAGTCCCACTTCGGATGGTACAACCCC\
TTACCTACTCGGATATAAGGAGGGAAAACATCCGAATTTCGACCAGGTCGTCGGGCGGCT\
TACGGTCCGATGGAGAATTGGCGAGCTGATTTGGGAAAGGAAGTTATCTGACAATAAAGG\
CCTAGCCAATGTTTCAATTTATTGATCCCGAGGTAAACATCAGCGGCTACATGTACCCTT\
TGTGCGTAAGATGCATTTAGAAGCCTGAGCTATGCATGAATGAGAGCTTATGAGCGCGGT\
TTCTTCCACGTGATCTTGAGGCTCATCGGACGCGGGCTCTGCAGCCTTGGGGCAAAGGAT\
GATCCAGACTACGATACTTGTGCTGTTTCTCCAGCGGACACAACTATAACGATCAAACTA\
CGTAAAGATTGAACCCCTCGTTGATAGTGTAGCTCCTCTGAGACCCTCTGTAACATAACC\
CTTGACGCGAAGTTCGGGTCGGCCATCACAAGTTGCGTATGGCAGGCTTCCCTACGTACT\
CGGGGTAGGGAGGTCTCATGGAAAGACAATCGAGCGCCACTTACCGGCTGACCGTACTTA\
TTAAAAGATTGATGACAAAGTGATGCGGGAAACTTTGGAGGGATCACTCATGTCACGAAG\
AAAGTTGTTCATAAACTGACATGCATAACTCGCCACGTTGGGGCTATCCAAGCATGAGCT\
CGGATGTTTCGGCAACGTTGAAGGGGAATACCGGGCCGCCACTGATCGCCAGGGCTGTAG\
AATCCTGCAATATGCTTAAACACCTCCATTAGCTGTGCCGAGGCGGGCTCCAGTAAGGCA\
ACAAAAAGTCGATGTCATCTAAGCGTCGGCCGGACCTGGGCGCACCTTGCGTTTTTCAAC\
GAAACTGCCTATGGGTCGTTAGTAGCGACACACTCTAGCGCTATGATGTTTCGCTAGGAA\
TGTCCCAAATTACTAGCGGGTAATCTGAGAGGGCATCACGGTGGGATGTCCGTCGGAGCG\
TGATGAATGCAACCCTAAAGCCCTGTGCTACTCGCACGAACAGGAGCACCTAACCTGCGA\
GACTTGCTGATTTCGAGCCCCGGTCGCCCCCGGGGGTGATACTCGTCCATAGTATTAGGC\
ATCCATTAGGGCGATGTTTAGAGCATCCATTAGCGGATAGTTTCATATGTGGGTATTACA\
GTCTGAATCGAGCACCGCACGATAGACGCTATAGTACTTAGATATTACGACGTACAGTGT\
AGTCTCTAGGACTCCTCTGATGGCTCCCACTACCCCGCGTCTACACGCACTCTTCTCGCT\
CCAGGGTACAAATAAATATGTATCTTCGCCCCGGTCGGCGCGGGACTGGACACCCAGGGA\
TAGCTCCGGATCAGCGAGCGCAGGATGCAATATTGGAGTTCTGATGAACAGCTCGGTACG\
CCGGGTCTAGTGGTTTCACGCTGAGATAGACTACTCCACAAGCTAATCCTTCACGGTCGC\
CTCCTTACCAACTATCGTTTGCCTATATCAGTGGTCGATGTTACCATAAGTGAGGTCACG\
GGACACGGACTAAGGTTAAATGAATAGGAAATACGGGAGTCTCTCCAGCCCTTATGGGCC\
GCCATCGCCTCTGCGTCAGTCCGTCCAACAGCTTTGGAGGCCTTCTGACAGCCGCTGGAT\
GTACTGCTACGCCTACCGGAGTGGTCTCGCGCATACGCACCAACTTTTTCCTTTTGACCA\
GTCATACATGAACGCAAGACGAATTCGATTGGCCTGGAATCCATTAGTGCTGCTCAGTGT\
TCTGCCGCGCGCACTGCGAGTGCATGCGAAAGTGCGGCCGTTCGATAGGGCTCTGTGTAA\
GTTCTCACTGATCTTGTCCGTCCTACACCACATTCTTATAAGTTATGTGTCATCCGAGAC\
GCAGCCCTGGACGGAATACATGTTGTATTTGTCGACGACCGTAATCCGAACGGACTTGAC\
CGAGGTCCTGTGCCCTGCATTTTTCTGCCCTACTTCCATGACTCTTCTATAGCGTAGGCC\
GCTTGTGGCGCGGCCACTAAAAACGCGTGCCAGCCGTAATTCCACCGGGATGACGCGACT\
TTACATGCCCCGAAGCGTTATATCCGCGTAGCTTCACTGTGGATAGGGTTTTGATCGCGA\
AGGAATCTCAATTCTAATCACAATATTATTCCTAAGGCAATACTCGGGCCACGCAATGAA\
ACTTGGCGGGATTATCGTAAATAGGATGAATCGATTTGTGCGAGATAACGCGCTGAAAAC\
TGAAACCATACCCTCTAATACGGTCCATTGATGGGTAGTATTTGTTGACCTGCGATTTGT\
CACGTTCGGGCCGTGGAATTTTTTTTCGGCGCATTTTATCCCGCGTACCTGGGCGCCGAT\
TTCCTAGCGACTATTTCGCAAGATCCACGTGTTCGTCACAAGGAGATAGCTCCAGCCGTG\
ATGCCACACCGGCACATTTTCTGCACGCGTGAGCCGTATATATCCAGTTAATTTTGGAAT\
CCGTAAGGTCGTCGCTGTCATGACTAGTCGTCAGGGACTCAATCAATCCCCGGTATCTCA\
AAACTCTGCAGGAGTTCCCAGTTATGGCACCGTTCTCAATTTGACGTAGATGATGCGCGA\
TTACGTACCGTACCCTTGGTGAAAACCAACCTGCTTCGCGCCGTAATGTCAAGCAACGGA\
CAAGCTGCGCTGGGTCGGCAAGACAGTCCGCGCTATTCACCGAGTGAAGCCTTAAGCTCA\
TGCAGTGAAAAGATGCGTCTAGGACGTGTCCATCTCAGAACTAGCTGTGCAGGTGAAGCC\
CAGCTTTAGGTGTCGTGACGGTCAAGATATCCAGTAAATGCGCCGACTAGTCCGGGCAAT\
ACGGTTTGTGATGCTACCTGATCTTGAGGACATTTAGTACACGATTCGGATTGGCCGGGT\
CGTGGATTATCCTTGTTAAAAACAGGACTCTATCAGATACCAGACAGCGAAAATTAACGT\
CGGCGCTGAGAACGAGCGTCATCGGGAGCACGCGGCGAAACGCTCCAATTACTAATTTCC\
TAATATGATATCGCCCGCCACCAGCGGGTAAGAGGCCAGGTATGACTTCGTTATTCGCCA\
TTGACTCAGCGCTTCAAACCGGTTGCTTGGTGAGAGTCAGCATTGGCGTCGTTCTGGGAG\
CTCCTCCGGTCATATCCTCATGCATCGCTGTACATCTCTATTATCCCTCGGCTTGGGATC\
CTACTTCGCCTTCGGGAACTTGAGCTGCGGGCCCAGGCAATGCTGGGGCACATTTACTAG\
GAGATCGATATTCATGCTGGCCGCGCCATAATTATGTCACTAGACACCATTCAGTTGCAC\
TTTGCTAAAAAGATTTGAGTTACTACTAATCTGTAAGATCATGCCTCAGCCCCGATCACC\
ACGGCACCCTATTGCGCGGATGACCAATCCCATGGTTTGTTTCGAACCAACTATCAAACC\
TAAGCCAAAATCGAGTTAGCTCCGCCCGGTACGACAATAACCCGAACCAACTTCTAGACC\
AACCTGACGCGATTCCGTGGTACTGTACGCTCAGAAACACTCCCTCCGGAGAAGCAATCC\
CCTATTCCACTGCCCATTGTCTACGGGTCCGAATTCATGCGCGGAGAAACCCGGTAGCGA\
CTGTTTGCAGTTAAAGGTGGCGCAAACGACATTAATTATATTAATTCCGAGGCGACGATG\
TTCTTAGATTCGGTCTATTGCAATACTAAGAGTTCGAAGCAGGCCGTCGCAGGGAACGAA\
CCACCTACTTGATCTACACTTGATTGAGCCAGCATGTAATTGCTCTGAACAGTATCATTA\
CAAACGGCTCTAAACTTCATAATGAGGTAACATCCGAAGG\
");