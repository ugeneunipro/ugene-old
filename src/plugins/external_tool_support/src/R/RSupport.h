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

#ifndef _U2_R_SUPPORT_H_
#define _U2_R_SUPPORT_H_

#include <U2Core/ExternalToolRegistry.h>
#include "utils/ExternalToolSupportAction.h"

#define ET_R "Rscript"
#define ET_R_GOSTATS "GOstats"
#define ET_R_GO_DB "GO.db"
#define ET_R_HGU133A_DB "hgu133a.db"
#define ET_R_HGU133B_DB "hgu133b.db"
#define ET_R_HGU1333PLUS2_DB "hgu133plus2.db"
#define ET_R_HGU95AV2_DB "hgu95av2.db"
#define ET_R_MOUSE430A2_DB "mouse430a2.db"
#define ET_R_CELEGANS_DB "celegans.db"
#define ET_R_DROSOPHILA2_DB "drosophila2.db"
#define ET_R_ORG_HS_EG_DB "org.Hs.eg.db"
#define ET_R_ORG_MM_EG_DB "org.Mm.eg.db"
#define ET_R_ORG_CE_EG_DB "org.Ce.eg.db"
#define ET_R_ORG_DM_EG_DB "org.Dm.eg.db"
#define ET_R_SEQLOGO "seqLogo"

namespace U2 {

class RSupport : public ExternalTool {
    Q_OBJECT
public:
    RSupport(const QString& name, const QString& path = "");
private slots:
    void sl_toolValidationStatusChanged(bool isValid);
};

class RModuleSupport : public ExternalToolModule {
    Q_OBJECT
public:
    RModuleSupport(const QString& name, const QString& path = "");
};

class RModuleGostatsSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleGostatsSupport(const QString& name, const QString& path = "");
};

class RModuleGodbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleGodbSupport(const QString& name, const QString& path = "");
};

class RModuleHgu133adbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleHgu133adbSupport(const QString& name, const QString& path = "");
};

class RModuleHgu133bdbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleHgu133bdbSupport(const QString& name, const QString& path = "");
};

class RModuleHgu133plus2dbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleHgu133plus2dbSupport(const QString& name, const QString& path = "");
};

class RModuleHgu95av2dbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleHgu95av2dbSupport(const QString& name, const QString& path = "");
};

class RModuleMouse430a2dbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleMouse430a2dbSupport(const QString& name, const QString& path = "");
};

class RModuleCelegansdbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleCelegansdbSupport(const QString& name, const QString& path = "");
};

class RModuleDrosophila2dbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleDrosophila2dbSupport(const QString& name, const QString& path = "");
};

class RModuleOrghsegdbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleOrghsegdbSupport(const QString& name, const QString& path = "");
};

class RModuleOrgmmegdbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleOrgmmegdbSupport(const QString& name, const QString& path = "");
};

class RModuleOrgceegdbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleOrgceegdbSupport(const QString& name, const QString& path = "");
};

class RModuleOrgdmegdbSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleOrgdmegdbSupport(const QString& name, const QString& path = "");
};

class RModuleSeqlogoSupport : public RModuleSupport {
    Q_OBJECT
public:
    RModuleSeqlogoSupport(const QString& name, const QString& path = "");
};

}   // namespace

#endif // _U2_R_SUPPORT_H_
