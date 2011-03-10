#include "Log.h"
#include "Timer.h"

#include <QtCore/QSet>

namespace U2 {


LogServer::LogServer() {
    qRegisterMetaType<LogMessage>("LogMessage");
}

LogServer* LogServer::getInstance() {
    static LogServer instance;
    return &instance;
}

QStringList LogServer::getCategories() const {
    QSet<QString> uniqueNames;
    QStringList result;
    foreach(const Logger* l, loggers) {
        foreach(const QString& category, l->getCategories()) {
            if (uniqueNames.contains(category)) {
                continue;
            }
            result.append(category);
            uniqueNames.insert(category);
        }
        
    }
    return result;
}

Logger::Logger(const QString& category1) {
    categoryNames << category1;
    init();
}

Logger::Logger(const QString& category1, const QString& category2) {
    categoryNames << category1 << category2;
    init();
}

Logger::Logger(const QString& category1, const QString& category2, const QString& category3) {
    categoryNames << category1 << category2 << category3;
    init();
}

Logger::Logger(const QString& category1, const QString& category2, const QString& category3, const QString& category4) {
    categoryNames << category1 << category2 << category3 << category4;
    init();
}

Logger::Logger(const QStringList& _categoryNames) {
    categoryNames = _categoryNames;
    init();
}

void Logger::init() {
    LogServer* s = LogServer::getInstance();
    s->loggers.append(this);
}

Logger::~Logger() {
// TODO possible race condition at shutdown -> log service could already be destroyed
//    LogServer* s = LogServer::getInstance();
//    s->categories.removeOne(this);
    categoryNames.clear();
}


LogMessage::LogMessage(const QStringList& cat, LogLevel l, const QString& m) 
: categories(cat), level(l), text(m), time(GTimer::currentTimeMicros()) 
{
}

void Logger::message(LogLevel level, const QString& msg)  {
    LogMessage m(categoryNames, level, msg);
    LogServer::getInstance()->message(m);
}

void Logger::message(LogLevel level, const QString& msg, const QString& extraCategory)  {
    message(level, msg, QStringList(extraCategory));
}

void Logger::message(LogLevel level, const QString& msg, const QStringList& extraCategories)  {
    QStringList resultCategories = categoryNames;
    resultCategories << extraCategories;
    LogMessage m(resultCategories, level, msg);
    LogServer::getInstance()->message(m);
}


void Logger::log(LogLevel level, const QString& message, const QString& category) {
    log(level, message, QStringList(category));
}

void Logger::log(LogLevel level, const QString& message, const QStringList& categoryies) {
    LogMessage m(categoryies, level, message);
    LogServer::getInstance()->message(m);
}


}//namespace
