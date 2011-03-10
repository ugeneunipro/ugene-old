
#include "RemoteMachineScanner.h"

namespace U2 {

RemoteMachineScanner::RemoteMachineScanner( bool filter ) : filterLocalHost( filter ) {
}

RemoteMachineScanner::~RemoteMachineScanner() {
}

void RemoteMachineScanner::setFilterLocalHost( bool filter ) {
    filterLocalHost = filter;
}

bool RemoteMachineScanner::getFilterLocalHost() const {
    return filterLocalHost;
}

} // U2
