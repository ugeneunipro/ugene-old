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


#ifndef _U2_UINDEX_VIEWER_FACTORY_H_
#define  _U2_UINDEX_VIEWER_FACTORY_H_

#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class MultiGSelection;

class U2VIEW_EXPORT UIndexViewerFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    UIndexViewerFactory();
    
    virtual bool canCreateView(const MultiGSelection& multiSelection);
    virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);
    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);
    virtual Task* createViewTask(const QString& viewName, const QVariantMap& stateData);
    bool supportsSavedStates() const;
    
    static const GObjectViewFactoryId ID;
    
}; // UIndexViewerFactory

} // U2

#endif // _U2_UINDEX_VIEWER_FACTORY_H_
