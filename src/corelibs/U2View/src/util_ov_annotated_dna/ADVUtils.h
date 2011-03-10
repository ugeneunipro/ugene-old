#ifndef _U2_ADV_UTILS_H_
#define _U2_ADV_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QSet>


namespace U2 {

class ADVSequenceWidget;
class AnnotatedDNAView;

enum ADVGlobalActionFlag {
    ADVGlobalActionFlag_AddToToolbar        = 1 << 1,
    ADVGlobalActionFlag_AddToAnalyseMenu    = 1 << 2,
    ADVGlobalActionFlag_SingleSequenceOnly  = 1 << 3
};

typedef QFlags<ADVGlobalActionFlag> ADVGlobalActionFlags;

class U2VIEW_EXPORT ADVGlobalAction: public GObjectViewAction {
    Q_OBJECT
public:
    ADVGlobalAction(AnnotatedDNAView* v, const QIcon& icon, const QString& text, int pos = 1000*1000, 
        ADVGlobalActionFlags flags = ADVGlobalActionFlags(ADVGlobalActionFlag_AddToToolbar) 
                                    | ADVGlobalActionFlag_AddToAnalyseMenu
                                    | ADVGlobalActionFlag_SingleSequenceOnly);
    
    void addAlphabetFilter(DNAAlphabetType t) {alphabetFilter.insert(t); updateState();};
    
    ADVGlobalActionFlags getFlags() const {return flags;}
    
    int getPosition() const {return pos;}

private slots:
    void sl_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*);

private:
    void updateState();

    QSet<DNAAlphabetType>   alphabetFilter;
    int                     pos;
    ADVGlobalActionFlags    flags;
};

} //namespace
#endif
