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

#ifndef _U2_BLASTPLUS_SUPPORT_H
#define _U2_BLASTPLUS_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2Gui/ObjectViewModel.h>

#define BLASTN_TOOL_NAME "BlastN"
#define BLASTP_TOOL_NAME "BlastP"
#define GPU_BLASTP_TOOL_NAME "GPU-BlastP"
#define BLASTX_TOOL_NAME "BlastX"
#define TBLASTN_TOOL_NAME "TBlastN"
#define TBLASTX_TOOL_NAME "TBlastX"
#define RPSBLAST_TOOL_NAME "RPSBlast"
#define BLASTPLUS_TMP_DIR    "blast_plus"

namespace U2 {

class BlastPlusSupport : public ExternalTool {
    Q_OBJECT
public:
    BlastPlusSupport(const QString& name, const QString& path = "");
public slots:
    void sl_runWithExtFileSpecify();
private:
    QString lastDBPath;
    QString lastDBName;
};

class BlastPlusSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    BlastPlusSupportContext(QObject* p);

protected slots:
    void sl_showDialog();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
private:
    QStringList toolList;
    QString lastDBPath;
    QString lastDBName;
};

}//namespace
#endif // _U2_BLASTPLUS_SUPPORT_H
