#include "MultipleDocumentsReadingModeSelectorController.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/LocalFileAdapter.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <QtGui/QFileDialog>

#include <memory>

namespace U2{

struct GUrlLess{     
        bool operator()(GUrl a, GUrl b) const     
        {
            return a.getURLString() < b.getURLString();
        } 
}; 

bool MultipleDocumentsReadingModeSelectorController::adjustReadingMode(QVariantMap& props, QList<GUrl>& urls, const QMap<QString, qint64>& headerSequenceLengths){	
    qSort(urls.begin(), urls.end(), GUrlLess());

    MultipleDocumentsReadingModeDialog d(urls, QApplication::activeWindow());
    return d.setupGUI(urls, props, headerSequenceLengths);    
}

void MultipleDocumentsReadingModeDialog::sl_onChooseDirPath(){
    QString fileFormats;
    fileFormats += extension4MergedDocument.toUpper() + " format (*." + extension4MergedDocument + ");;";
 
    fileFormats.append("All files (*)");

    LastUsedDirHelper helper("SaveMergeDocumentsAsOneDocument");
    helper.url = QFileDialog::getSaveFileName(this, tr("Select file to save new document"), helper.dir, fileFormats, 
        NULL, QFileDialog::DontConfirmOverwrite);
    if(!helper.url.isEmpty()){
        newDocUrl->setText(helper.url);
    }
}

bool MultipleDocumentsReadingModeSelectorController::mergeDocumentOption(const FormatDetectionResult& formatResult, QMap<QString, qint64>* headerSequenceLengths){
    QVariantMap docHints = formatResult.rawDataCheckResult.properties;
    if(formatResult.format == NULL){
        return false;
    }
    if(formatResult.format->getFormatId() == BaseDocumentFormats::PLAIN_GENBANK){
        if(docHints.value(RawDataCheckResult_Sequence) == false){
            static const int MAX_LINE = 8192;
            char buff[MAX_LINE + 1] = {0};

            std::auto_ptr<LocalFileAdapterFactory> factory( new LocalFileAdapterFactory());
            std::auto_ptr<IOAdapter> io(factory->createIOAdapter());
            if(!io->open(formatResult.url, IOAdapterMode_Read)){
                return false;
            }
            bool terminatorFound = false;
            io->readLine(buff, MAX_LINE, &terminatorFound);
            if(!terminatorFound){
                return false;
            }
            QString line = QString(QByteArray(buff));
            QStringList words = line.split(QRegExp("\\s"), QString::SkipEmptyParts);
            if(words.size() < 3){ // origin len not defined
                return false;
            }
            bool isLenDefined = false;
            qint64 seqLen = words[2].toLongLong(&isLenDefined); 
            
            if(!isLenDefined || seqLen <= 0){
                return false;
            }
            (*headerSequenceLengths)[formatResult.url.getURLString()] = seqLen;
            return true;
        }
    }
    return docHints.value(RawDataCheckResult_Sequence).toBool();
}


MultipleDocumentsReadingModeDialog::MultipleDocumentsReadingModeDialog(const QList<GUrl>& _urls,QWidget* parent /* = 0 */):QDialog(parent),  urls(_urls){}

void MultipleDocumentsReadingModeDialog::setupNewUrl(){
    GUrl url = urls.at(0);
    QString urlStr = url.getURLString(); 
    int startFileName = urlStr.lastIndexOf(url.fileName());
    urlStr.remove(startFileName, url.fileName().size());

    QString randomFileName = GUrlUtils::rollFileName(urlStr + QString("merged_document") + "." + extension4MergedDocument , DocumentUtils::getNewDocFileNameExcludesHint()) ;
    newDocUrl->setText(randomFileName);
}

bool MultipleDocumentsReadingModeDialog::setupGUI(QList<GUrl>& _urls, QVariantMap& props, const QMap<QString, qint64>& headerSequenceLengths){
    setModal(true);
    setupUi(this);
    // doesn't matter from what position, because excluded fileName all path of documents are the same
    CHECK(!urls.isEmpty(), false);

    connect(separateMode, SIGNAL(toggled(bool)), SLOT(sl_optionChanged(bool)));
    connect(mergeMode, SIGNAL(toggled(bool)), SLOT(sl_optionChanged(bool)));
    connect(join2alignmentMode, SIGNAL(toggled(bool)), SLOT(sl_optionChanged(bool)));
    connect(upperButton, SIGNAL(clicked()), SLOT(sl_onMoveUp()));
    connect(bottomButton, SIGNAL(clicked()), SLOT(sl_onMoveDown()));

    upperButton->setIcon(QIcon(":ugene/images/up.png"));
    bottomButton->setIcon(QIcon(":ugene/images/down.png"));

    for(int i = 0; i<urls.size(); ++i){
        listDocuments->addItem(new QListWidgetItem(QString("%1. ").arg(i + 1) + urls.at(i).fileName(), listDocuments));           
    }

    int rc = exec();
    
    if (rc == QDialog::Rejected) {
        return false;
    }

    if(separateMode->isChecked() || newDocUrl->text().isEmpty()){
        return true;
    }

    deleteAllNumPrefix();
    _urls.clear();

    for(int i = 0; i < listDocuments->count(); ++i){
        _urls << findUrlByFileName(listDocuments->item(i)->text());
    }

    listDocuments->clear();

    if(mergeMode->isChecked()){
        props[DocumentReadingMode_SequenceMergeGapSize] = fileGap->value();
        props[ProjectLoaderHint_MultipleFilesMode_RealDocumentFormat] = BaseDocumentFormats::PLAIN_GENBANK;

        foreach(const GUrl& url, _urls){
            props[RawDataCheckResult_HeaderSequenceLength + url.getURLString()] = headerSequenceLengths.value(url.getURLString(), -1);
        }
    }
    else if(join2alignmentMode->isChecked()){
        props[ProjectLoaderHint_MultipleFilesMode_RealDocumentFormat] = BaseDocumentFormats::CLUSTAL_ALN;
        props[DocumentReadingMode_SequenceAsAlignmentHint] = true;
    }
    props[ProjectLoaderHint_MultipleFilesMode_URLDocument] = newDocUrl->text();
    props[ProjectLoaderHint_MultipleFilesMode_SaveDocumentFlag] = saveBox->isChecked();
    props[ProjectLoaderHint_MultipleFilesMode_Flag] = true;
    

    QStringList urlsStr;
    foreach(GUrl url, _urls){
        urlsStr << url.getURLString();
    }

    props[ProjectLoaderHint_MultipleFilesMode_URLsDocumentConsistOf] =  urlsStr;

    

    _urls.clear();
    _urls << GUrl(newDocUrl->text());
    
    return true;
}

void MultipleDocumentsReadingModeDialog::setupOrderingMergeDocuments(){
    
}

QString MultipleDocumentsReadingModeDialog::findUrlByFileName(const QString& fileName){
    foreach(GUrl url, urls){
        QString _fileName = url.fileName();
        if(_fileName == fileName){
            return url.getURLString();
        }
    }
    assert(false && "Error finding url by file name");
    return "";
}

void MultipleDocumentsReadingModeDialog::sl_optionChanged(bool ){
    bool isNewDocInfoAvailable = !separateMode->isChecked();

    saveBox->setEnabled(isNewDocInfoAvailable);
    listDocuments->setEnabled(isNewDocInfoAvailable);
    upperButton->setEnabled(isNewDocInfoAvailable);
    bottomButton->setEnabled(isNewDocInfoAvailable);
    newDocLabel->setEnabled(isNewDocInfoAvailable);
    newDocUrl->setEnabled(isNewDocInfoAvailable);

    bool mergeInfoAvailable = mergeMode->isChecked();
    fileGap->setEnabled(mergeInfoAvailable );
    mergeModeLabel->setEnabled(mergeInfoAvailable );

    if(mergeMode->isChecked()){
        extension4MergedDocument = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK)->getSupportedDocumentFileExtensions().first();
    }
    else if(join2alignmentMode->isChecked()){
        extension4MergedDocument = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN)->getSupportedDocumentFileExtensions().first();
    }
    else{
        return;
    }
    setupNewUrl();
}

void MultipleDocumentsReadingModeDialog::sl_onMoveUp(){
    QListWidgetItem * item = listDocuments->currentItem();
    if(item == NULL){return;}
    int row = listDocuments->row(item);
    if(row == 0){return;}

    listDocuments->insertItem(row - 1, new QListWidgetItem(item->text()));
    listDocuments->removeItemWidget(item);
    delete item;

    changeNumPrefix();
}

void MultipleDocumentsReadingModeDialog::sl_onMoveDown(){
    QListWidgetItem * item = listDocuments->currentItem();
    if(item == NULL){return;}
    int row = listDocuments->row(item);
    if(row == listDocuments->count() - 1){return;}

    listDocuments->insertItem(row + 2, new QListWidgetItem(item->text()));
    listDocuments->removeItemWidget(item);
    delete item;
    
    changeNumPrefix();
}

void MultipleDocumentsReadingModeDialog::changeNumPrefix(){
    for(int i = 0; i < listDocuments->count(); ++i){
        listDocuments->item(i)->setText(QString("%1. ").arg(i+1) + deleteNumPrefix(listDocuments->item(i)->text()));
    }
}

QString MultipleDocumentsReadingModeDialog::deleteNumPrefix(QString prefixString){
    QString toDelete = prefixString.split(" ")[0]; // number with space
    prefixString.remove(0, toDelete.size() + 1);
    return prefixString;
}

void MultipleDocumentsReadingModeDialog::deleteAllNumPrefix(){
    for(int i = 0; i < listDocuments->count(); ++i){
        listDocuments->item(i)->setText(deleteNumPrefix(listDocuments->item(i)->text()));
    }
}

MultipleDocumentsReadingModeDialog::~MultipleDocumentsReadingModeDialog(){
    
}

};