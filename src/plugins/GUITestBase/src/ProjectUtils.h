#ifndef _U2_PROJECT_UTILS_H_
#define _U2_PROJECT_UTILS_H_

#include <U2Core/GUrl.h>
#include <U2Core/U2OpStatus.h>

namespace U2 {

class GUITest;
class Document;

class ProjectUtils {
public:
	class OpenFileSettings {
	public:
		enum OpenMethod {DRAGDROP} openMethod;
	};

	/*
		opens a file using settings, checks if the document is loaded
	*/
	static void openFile(U2OpStatus &os, const GUrl &path, const OpenFileSettings& = OpenFileSettings());

protected:
	static void openFileDrop(U2OpStatus &os, const GUrl &path);

	static void checkProjectExists(U2OpStatus &os);

	static Document* checkDocumentExists(U2OpStatus &os, const GUrl &url);
	static void checkDocumentActive(U2OpStatus &os, Document *doc);
};

} // U2

#endif
