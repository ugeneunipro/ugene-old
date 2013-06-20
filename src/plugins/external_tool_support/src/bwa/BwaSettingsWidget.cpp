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
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/L10n.h>

#include "BwaSettingsWidget.h"
#include "BwaTask.h"

const qint64 MAX_REFERENCE_SIZE_FOR_IS_METHOD =     2.147e+9;
const qint64 MIN_REFERENCE_SIZE_FOR_BWTSW_METHOD =  1.049e+7;
const QString STYLE_SHEET_COLOR_ATTRIBUTE =         "color";
const QString STYLE_SHEET_FONT_WEIGHT_ATTRIBUTE =   "font-weight";
const QString INFO_MESSAGE_FONT =                   "bold";
const QString STYLE_SHEET_ATTRIBUTE_EQUALS_SIGN =   ": ";
const QString STYLE_SHEET_ATTRIBUTES_SEPARATOR =     ";";

void setStylesheetAttributeValue( const QString &attributeName, const QString &attributeValue,
    QString &stylesheet )
{
    int attributeDescriptionStart = stylesheet.indexOf( attributeName );
    if ( -1 != attributeDescriptionStart ) {
        attributeDescriptionStart += attributeName.length( )
            + STYLE_SHEET_ATTRIBUTE_EQUALS_SIGN.length( );
        int attributeDescriptionEnd = stylesheet.indexOf( STYLE_SHEET_ATTRIBUTES_SEPARATOR,
            attributeDescriptionStart );
        stylesheet.replace( attributeDescriptionStart,
            attributeDescriptionEnd - attributeDescriptionStart, attributeValue );
    } else {
        stylesheet.append( " " + attributeName + STYLE_SHEET_ATTRIBUTE_EQUALS_SIGN
            + attributeValue + STYLE_SHEET_ATTRIBUTES_SEPARATOR );
    }
}

namespace U2 {

// BwaSettingsWidget

BwaSettingsWidget::BwaSettingsWidget(QWidget *parent):
    DnaAssemblyAlgorithmMainWidget(parent), referenceSequencePath( )
{
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);

    threadsSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    threadsSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    connect( indexAlgorithmComboBox, SIGNAL( currentIndexChanged ( int ) ),
        SLOT( sl_IndexAlgorithmChanged( int ) ) );
    setupInfoLabel( );
}

QMap<QString,QVariant> BwaSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString, QVariant> settings;

    {
        QString algorithm;
        switch(indexAlgorithmComboBox->currentIndex()) {
        case 0:
            algorithm = "bwtsw";
            break;
        case 1:
            algorithm = "div";
            break;
        case 2:
            algorithm = "is";
            break;
        default:
            assert(false);
            break;
        }
        settings.insert(BwaTask::OPTION_INDEX_ALGORITHM, algorithm);
    }
    if(maxDiffRadioButton->isChecked()) {
        settings.insert(BwaTask::OPTION_N, maxDiffSpinBox->value());
    } else {
        settings.insert(BwaTask::OPTION_N, missingProbSpinBox->value());
    }
    settings.insert(BwaTask::OPTION_MAX_GAP_OPENS, maxGapOpensSpinBox->value());
    if(enableLongGapsCheckBox) {
        settings.insert(BwaTask::OPTION_MAX_GAP_EXTENSIONS, maxGapExtensionsSpinBox->value());
    }
    settings.insert(BwaTask::OPTION_INDEL_OFFSET, indelOffsetSpinBox->value());
    settings.insert(BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS, maxLongDeletionExtensionsSpinBox->value());
    settings.insert(BwaTask::OPTION_SEED_LENGTH, seedLengthSpinBox->value());
    settings.insert(BwaTask::OPTION_MAX_SEED_DIFFERENCES, maxSeedDifferencesSpinBox->value());
    settings.insert(BwaTask::OPTION_MAX_QUEUE_ENTRIES, maxQueueEntriesSpinBox->value());
    settings.insert(BwaTask::OPTION_THREADS, threadsSpinBox->value());
    settings.insert(BwaTask::OPTION_MISMATCH_PENALTY, mismatchPenaltySpinBox->value());
    settings.insert(BwaTask::OPTION_GAP_OPEN_PENALTY, gapOpenPenaltySpinBox->value());
    settings.insert(BwaTask::OPTION_GAP_EXTENSION_PENALTY, gapExtensionPenaltySpinBox->value());
    settings.insert(BwaTask::OPTION_BEST_HITS, bestHitsSpinBox->value());
    settings.insert(BwaTask::OPTION_QUALITY_THRESHOLD, qualityThresholdSpinBox->value());
    settings.insert(BwaTask::OPTION_BARCODE_LENGTH, barcodeLengthSpinBox->value());
    settings.insert(BwaTask::OPTION_COLORSPACE, colorspaceCheckBox->isChecked());
    settings.insert(BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS, longScaledGapPenaltyForLongDeletionsCheckBox->isChecked());
    settings.insert(BwaTask::OPTION_NON_ITERATIVE_MODE, nonIterativeModeCheckBox->isChecked());

    return settings;
}

void BwaSettingsWidget::buildIndexUrl(const GUrl &) {
    // do nothing
}

bool BwaSettingsWidget::isParametersOk(QString &) {
    return true;
}

void BwaSettingsWidget::validateReferenceSequence( const GUrl &url ) {
    referenceSequencePath = url;
    sl_IndexAlgorithmChanged( indexAlgorithmComboBox->currentIndex( ) );
}

void BwaSettingsWidget::sl_IndexAlgorithmChanged( int index ) {
    QFile referenceSequenceFile( referenceSequencePath.getURLString( ) );
    QString infoText = QString( );
    if ( 2 == index ) {
        if ( MAX_REFERENCE_SIZE_FOR_IS_METHOD < referenceSequenceFile.size( ) ) {
            infoText = tr( "NOTE: \"is\" index algorithm is not supposed to work with reference sequences\n"
                "having size larger than 2 GB. In order to achieve stable BWA performance\n"
                "it is strongly recommend to set the index algorithm to \"bwtsw\"" );
        }
    } else if ( 0 == index ) {
        if ( MIN_REFERENCE_SIZE_FOR_BWTSW_METHOD > referenceSequenceFile.size( ) ) {
            infoText = tr( "NOTE: \"bwtsw\" index algorithm is not supposed to work with reference sequences\n"
                "having size smaller than 10 MB. In order to achieve stable BWA performance\n"
                "it is strongly recommend to set the index algorithm to \"is\"" );
        }
    }
    infoLabel->setText( infoText );
}

void BwaSettingsWidget::setupInfoLabel( ) {
    QString infoLabelStyleSheet = infoLabel->styleSheet( );
    setStylesheetAttributeValue( STYLE_SHEET_COLOR_ATTRIBUTE, U2::L10N::errorColorLabelStr( ),
        infoLabelStyleSheet );
    setStylesheetAttributeValue( STYLE_SHEET_FONT_WEIGHT_ATTRIBUTE, INFO_MESSAGE_FONT,
        infoLabelStyleSheet );
    infoLabel->setStyleSheet( infoLabelStyleSheet );
}

// BwaBuildSettingsWidget

BwaBuildSettingsWidget::BwaBuildSettingsWidget(QWidget *parent):
    DnaAssemblyAlgorithmBuildIndexWidget(parent)
{
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);
}

QMap<QString,QVariant> BwaBuildSettingsWidget::getBuildIndexCustomSettings() {
    QMap<QString, QVariant> settings;
    settings.insert(BwaTask::OPTION_COLORSPACE, colorspaceCheckBox->isChecked());
    {
        QString algorithm;
        switch(indexAlgorithmComboBox->currentIndex()) {
        case 0:
            algorithm = "bwtsw";
            break;
        case 1:
            algorithm = "div";
            break;
        case 2:
            algorithm = "is";
            break;
        default:
            assert(false);
            break;
        }
        settings.insert(BwaTask::OPTION_INDEX_ALGORITHM, algorithm);
    }
    return settings;
}

QString BwaBuildSettingsWidget::getIndexFileExtension() {
    return QString();
}

void BwaBuildSettingsWidget::buildIndexUrl(const GUrl& ) {
    // do nothing
}

// BwaGUIExtensionsFactory

DnaAssemblyAlgorithmMainWidget *BwaGUIExtensionsFactory::createMainWidget(QWidget *parent) {
    return new BwaSettingsWidget(parent);
}

DnaAssemblyAlgorithmBuildIndexWidget *BwaGUIExtensionsFactory::createBuildIndexWidget(QWidget *parent) {
    return new BwaBuildSettingsWidget(parent);
}

bool BwaGUIExtensionsFactory::hasMainWidget() {
    return true;
}

bool BwaGUIExtensionsFactory::hasBuildIndexWidget() {
    return true;
}


// BwaSettingsWidget

BwaSwSettingsWidget::BwaSwSettingsWidget(QWidget *parent):
    DnaAssemblyAlgorithmMainWidget(parent)
{
    setupUi(this);
    layout()->setContentsMargins(0,0,0,0);

    numThreadsSpinbox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    numThreadsSpinbox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
}

QMap<QString,QVariant> BwaSwSettingsWidget::getDnaAssemblyCustomSettings() {
    QMap<QString, QVariant> settings;

    settings.insert(BwaTask::OPTION_SW_ALIGNMENT,true);
    
    settings.insert(BwaTask::OPTION_THREADS, numThreadsSpinbox->value());
    settings.insert(BwaTask::OPTION_MATCH_SCORE, matchScoreSpinbox->value());
    settings.insert(BwaTask::OPTION_MISMATCH_PENALTY, mismatchScoreSpinbox->value());
    settings.insert(BwaTask::OPTION_GAP_OPEN_PENALTY, gapOpenSpinbox->value());
    settings.insert(BwaTask::OPTION_GAP_EXTENSION_PENALTY, gapExtSpinbox->value());
    settings.insert(BwaTask::OPTION_CHUNK_SIZE, chunkSizeSpinbox->value());
    settings.insert(BwaTask::OPTION_BAND_WIDTH, bandWidthSpinbox->value());
    settings.insert(BwaTask::OPTION_MASK_LEVEL, maskLevelSpinbox->value());
    settings.insert(BwaTask::OPTION_SCORE_THRESHOLD, scoreThresholdSpinbox->value());
    settings.insert(BwaTask::OPTION_Z_BEST, zBestSpinbox->value());
    settings.insert(BwaTask::OPTION_REV_ALGN_THRESHOLD, revAlnThreshold->value());
    if (hardClippingCheckBox->isChecked()) {
        settings.insert(BwaTask::OPTION_PREFER_HARD_CLIPPING, true);
    }
   
    return settings;
}

void BwaSwSettingsWidget::buildIndexUrl(const GUrl &) {
    // do nothing
}

bool BwaSwSettingsWidget::isParametersOk(QString &) {
    return true;
}

// BwaGUIExtensionsFactory

DnaAssemblyAlgorithmMainWidget *BwaSwGUIExtensionsFactory::createMainWidget(QWidget *parent) {
    return new BwaSwSettingsWidget(parent);
}

DnaAssemblyAlgorithmBuildIndexWidget *BwaSwGUIExtensionsFactory::createBuildIndexWidget(QWidget *parent) {
    return new BwaBuildSettingsWidget(parent);
}

bool BwaSwGUIExtensionsFactory::hasMainWidget() {
    return true;
}

bool BwaSwGUIExtensionsFactory::hasBuildIndexWidget() {
    return true;
}

} //namespace
