#ifndef _U2_NEXUS_PARSER_H_
#define _U2_NEXUS_PARSER_H_

#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>

#include <cassert>

namespace U2
{

class GObject;

class Tokenizer
// Breaks input stream on tokens (QString)
{
public:
    Tokenizer(IOAdapter *io_) :
        io(io_)
    {
        assert(io_ && "IO must exist");
    }

    QString look();
    QString get();
    void skip() { (void)get(); }

    QStringList getUntil(QString what, Qt::CaseSensitivity cs = Qt::CaseInsensitive);
    void skipUntil(QString what, Qt::CaseSensitivity cs = Qt::CaseInsensitive);

    QString readUntil(QRegExp rwhat);

    bool isEof() { return io->isEof(); }
    IOAdapter* getIO() { return io; }

private:
    IOAdapter *io;
    QString next;

    static const int BUFF_SIZE;
    QString buff;
    QTextStream buffStream;
};

class NEXUSParser
// NEXUS File format parser
{
public:
    NEXUSParser (IOAdapter *io_, TaskStateInfo &ti_)
        : io(io_), ti(ti_), tz(io)
    {
        assert(io_ && "IO must exist");
    }

    QList<GObject*> loadObjects();

    bool hasErrors() { return !errors.isEmpty(); }
    bool hasWarnings() { return !warnings.isEmpty(); }

    const QStringList& getErrors() const { return errors; }
    const QStringList& getWarnings() const { return warnings; }

private:
    typedef QMap<QString, QString> Context;

    bool skipCommand();
    bool readSimpleCommand(Context &ctx);

    bool readBlock(Context &ctx);
    bool skipBlockContents();
    bool readTaxaContents(Context &ctx);
    bool readDataContents(Context &ctx);
    bool readTreesContents(Context &ctx);

    void reportProgress() { ti.progress = io->getProgress(); }

    // append object to objects, and resolve name conflicts
    void addObject(GObject *obj);

private:
    static const QString BEGIN;
    static const QString END;
    static const QString BLK_TAXA;
    static const QString BLK_DATA;
    static const QString BLK_CHARACTERS;
    static const QString CMD_DIMENSIONS;
    static const QString CMD_FORMAT;
    static const QString CMD_MATRIX;
    static const QString BLK_TREES;
    static const QString CMD_TREE;
    static const QString CMD_UTREE;

private:
    IOAdapter *io;
    TaskStateInfo &ti;
    Tokenizer tz;

    QList<GObject*> objects;
    QSet<QString> objectNames;

    Context global;

    QStringList errors, warnings;
};

} // namespace U2

#endif	// #ifndef _U2_NEXUS_PARSER_H_
