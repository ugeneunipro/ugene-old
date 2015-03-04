/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "DnaAssemblyGUIExtension.h"

namespace U2 {

/************************************************************************/
/* DnaAssemblyAlgorithmBaseWidget */
/************************************************************************/
DnaAssemblyAlgorithmBaseWidget::DnaAssemblyAlgorithmBaseWidget(QWidget *parent)
: QWidget(parent)
{

}

void DnaAssemblyAlgorithmBaseWidget::validateReferenceSequence(const GUrl & /*url*/) {

}

/************************************************************************/
/* DnaAssemblyAlgorithmMainWidget */
/************************************************************************/
DnaAssemblyAlgorithmMainWidget::DnaAssemblyAlgorithmMainWidget(QWidget *parent)
: DnaAssemblyAlgorithmBaseWidget(parent)
{

}

bool DnaAssemblyAlgorithmMainWidget::isParametersOk(QString & /*error*/) const {
    return true;
}

bool DnaAssemblyAlgorithmMainWidget::buildIndexUrl(const GUrl & /*url*/, bool /*prebuiltIndex*/, QString & /*error*/) const {
    return true;
}

bool DnaAssemblyAlgorithmMainWidget::isIndexOk(const GUrl & /*url*/, QString & /*error*/) const {
    return true;
}

bool DnaAssemblyAlgorithmMainWidget::isIndex(const QString &url) const {
    return DnaAssemblyToReferenceTask::isIndexUrl(url, indexSuffixes);
}

bool DnaAssemblyAlgorithmMainWidget::isValidIndex(const QString &oneIndexFileUrl) const {
    QString baseUrl = DnaAssemblyToReferenceTask::getBaseUrl(oneIndexFileUrl, indexSuffixes);
    return DnaAssemblyToReferenceTask::isPrebuiltIndex(baseUrl, indexSuffixes);
}

/************************************************************************/
/* DnaAssemblyAlgorithmBuildIndexWidget */
/************************************************************************/
DnaAssemblyAlgorithmBuildIndexWidget::DnaAssemblyAlgorithmBuildIndexWidget(QWidget *parent)
: DnaAssemblyAlgorithmBaseWidget(parent)
{

}

/************************************************************************/
/* DnaAssemblyGUIExtensionsFactory */
/************************************************************************/
DnaAssemblyGUIExtensionsFactory::~DnaAssemblyGUIExtensionsFactory() {

}

/************************************************************************/
/* GenomeAssemblyAlgorithmMainWidget */
/************************************************************************/
GenomeAssemblyAlgorithmMainWidget::GenomeAssemblyAlgorithmMainWidget(QWidget *parent)
: QWidget(parent)
{

}

} // U2
