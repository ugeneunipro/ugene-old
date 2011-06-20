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

#include "DNAFlexDialog.h"
#include "DNAFlexPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>
#include <U2View/AnnotatedDNAView.h>

#include <QMessageBox>


namespace U2 {


extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC()
{
    DNAFlexPlugin* plugin = new DNAFlexPlugin();
    return plugin;
}


DNAFlexPlugin::DNAFlexPlugin()
: Plugin(
         tr("DNA Flexibility"),
         tr("Searches a DNA sequence for regions of high DNA helix flexibility.")),
         viewCtx(0)
{
    if (AppContext::getMainWindow())
    {
        viewCtx = new DNAFlexViewContext(this);
        viewCtx->init();
    }
}


DNAFlexViewContext::DNAFlexViewContext(QObject* parent)
    : GObjectViewWindowContext(parent, ANNOTATED_DNA_VIEW_FACTORY_ID)
{
}


void DNAFlexViewContext::sl_showDNAFlexDialog()
{
    QAction* action = (QAction*) sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(action);
    AnnotatedDNAView* annotView = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(annotView);

    ADVSequenceObjectContext* seqCtx = annotView->getSequenceInFocus();
    assert(seqCtx != 0 && seqCtx->getAlphabet()->isNucleic());

    if (inputSequenceIsValid(seqCtx->getSequenceData()))
    {
        DNAFlexDialog dialog(seqCtx);
        dialog.exec();
    }
    else
    {
        QMessageBox::critical(0, L10N::errorTitle(),
            tr("The input sequence must contain only A, C, G and T characters!"));
        return;
    }
}


bool DNAFlexViewContext::inputSequenceIsValid(const QByteArray& sequence)
{
    for (int i = 0; i < sequence.size(); ++i)
    {

        if (!((sequence.at(i) == 'A') || (sequence.at(i) == 'C')
            || (sequence.at(i) == 'G') || (sequence.at(i) == 'T')))
        {
            return false;
        }
    }
    return true;
}


void DNAFlexViewContext::initViewContext(GObjectView* view)
{
    AnnotatedDNAView* annotView = qobject_cast<AnnotatedDNAView*>(view);
    ADVGlobalAction* action = new ADVGlobalAction(annotView,
        QIcon(":dna_flexibility/images/flexibility.png"),
        tr("Find high DNA flexibility regions..."),
        20);
    action->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(action, SIGNAL(triggered()), SLOT(sl_showDNAFlexDialog()));
}


} // namespace
