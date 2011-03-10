#include "KalignException.h"
#include <assert.h>
#include <memory>
#include <string.h>

extern "C" void throwKalignException(char *message) {
	throw U2::KalignException(message);
}

namespace U2 {

KalignException::KalignException(const char* _str) {
	int len = strlen(_str);
	assert(len < 4096);
	memcpy(str, _str, len);
	str[len] = '\0';
}

KalignException::KalignException() {
	memset(str, 4096, '\0');
}

} // namespace U2
