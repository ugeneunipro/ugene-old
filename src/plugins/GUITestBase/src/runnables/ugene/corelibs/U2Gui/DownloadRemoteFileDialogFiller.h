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

#ifndef _U2_GT_RUNNABLES_DOWNLOAD_REMOTE_FILE_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_DOWNLOAD_REMOTE_FILE_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

    class RemoteDBDialogFiller : public Filler {
    public:
        RemoteDBDialogFiller(U2OpStatus &_os, const QString _resID, int _DBItemNum, bool _pressCancel = false, const QString _saveDirPath = QString(),
                             GTGlobals::UseMethod _useMethod = GTGlobals::UseMouse, int _outFormatVal = -1)
            :Filler(_os, "DownloadRemoteFileDialog"),
              resID(_resID),
              DBItemNum(_DBItemNum),
              pressCancel(_pressCancel),
              saveDirPath(_saveDirPath),
              useMethod(_useMethod),
              outFormatVal(_outFormatVal){}
        virtual void run();
    private:
        QString resID;
        int DBItemNum;
        bool pressCancel;
        QString saveDirPath;
        GTGlobals::UseMethod useMethod;
        int outFormatVal;
    };
}

#endif
