/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PROJECT_DOCUMENT_COMBOBOX_CONTROLLER
#define _U2_PROJECT_DOCUMENT_COMBOBOX_CONTROLLER

#include <U2Core/DocumentModel.h>

#include <QtCore/QObject>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QComboBox>
#else
#include <QtWidgets/QComboBox>
#endif

namespace U2 {

class Project;

class U2GUI_EXPORT ProjectDocumentComboBoxController : public QObject {
	Q_OBJECT
public:

	ProjectDocumentComboBoxController(Project* p, QComboBox* cb, QObject* parent, const DocumentFilter* f);
    ~ProjectDocumentComboBoxController();

	Document* getDocument() const;

	void selectDocument(Document* d);
    
    void selectDocument(const QString& url);

private slots:
	void sl_onDocumentAdded(Document* d);
	void sl_onDocumentRemoved(Document* d);

private:
	bool checkConstraints(Document* d);
	void addDocument(Document* d);
	void removeDocument(Document* d);

	Project* p;
	QComboBox* cb;
	const DocumentFilter* filter;
};

} // namespace

#endif
