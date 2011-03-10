#ifndef _U2_BOWTIE_ADAPTER_H_
#define _U2_BOWTIE_ADAPTER_H_

#include <QtCore/QObject>
#include <U2Core/GUrl.h>



namespace U2 {

class MAlignment;
class TaskStateInfo;
class BowtieReadsReader;
class BowtieReadsWriter;

class BowtieAdapter : public QObject {
Q_OBJECT
public:
	static void doBowtie(const QString& ebwtFileName, BowtieReadsReader* reader, BowtieReadsWriter* writer, const U2::GUrl& resultUrl, TaskStateInfo& ti);
	static void doBowtieWorker(int id, TaskStateInfo& ti);
	static void doBowtieBuild(const QString& infile, const QString& outfile, TaskStateInfo& ti );
};

}//namespace

#endif
