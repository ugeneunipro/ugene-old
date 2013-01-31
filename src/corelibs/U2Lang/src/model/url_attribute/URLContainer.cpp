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

#include <U2Lang/HRSchemaSerializer.h>

#include "URLContainer.h"

namespace U2 {

URLContainer::URLContainer(const QString &_url)
: url(_url)
{

}

URLContainer::~URLContainer() {

}

const QString & URLContainer::getUrl() const {
    return url;
}

URLContainer * URLContainerFactory::createUrlContainer(const QString &url) {
    QFileInfo info(url);
    if (!info.exists()) {
        return NULL;
    }

    if (info.isDir()) {
        return new DirUrlContainer(url, "", "", false);
    } else {
        return new FileUrlContainer(url);
    }
}

FileUrlContainer::FileUrlContainer(const QString &url)
: URLContainer(url)
{

}

FileUrlContainer::~FileUrlContainer() {

}

FilesIterator * FileUrlContainer::getFileUrls() {
    return FilesIteratorFactory::createFileList(QStringList() << url);
}

URLContainer * FileUrlContainer::clone() {
    return new FileUrlContainer(url);
}

void FileUrlContainer::accept(URLContainerVisitor *visitor) {
    visitor->visit(this);
}

DirUrlContainer::DirUrlContainer(const QString &url)
: URLContainer(url), recursive(false)
{

}

DirUrlContainer::DirUrlContainer(const QString &url, const QString &_incFilter, const QString &_excFilter, bool _recursive)
: URLContainer(url), incFilter(_incFilter), excFilter(_excFilter), recursive(_recursive)
{

}

DirUrlContainer::~DirUrlContainer() {

}

FilesIterator * DirUrlContainer::getFileUrls() {
    return FilesIteratorFactory::createDirectoryScanner(
        QStringList() << url, incFilter, excFilter, recursive);
}

URLContainer * DirUrlContainer::clone() {
    DirUrlContainer *cloned = new DirUrlContainer(url);
    cloned->incFilter = incFilter;
    cloned->excFilter = excFilter;
    cloned->recursive = recursive;
    return cloned;
}

void DirUrlContainer::accept(URLContainerVisitor *visitor) {
    visitor->visit(this);
}

const QString & DirUrlContainer::getIncludeFilter() const {
    return incFilter;
}

const QString & DirUrlContainer::getExcludeFilter() const {
    return excFilter;
}

bool DirUrlContainer::isRecursive() const {
    return recursive;
}

void DirUrlContainer::setIncludeFilter(const QString &value) {
    incFilter = value;
}

void DirUrlContainer::setExcludeFilter(const QString &value) {
    excFilter = value;
}

void DirUrlContainer::setRecursive(bool value) {
    recursive = value;
}

} // U2
