#ifndef _U2_KALIGN_CONTEXT_H_
#define _U2_KALIGN_CONTEXT_H_

namespace U2 {

struct KalignException {
	KalignException();
	KalignException(const char* str);
	char str[4096];
};

} //namespace U2

#endif //_U2_KALIGN_CONTEXT_H_

