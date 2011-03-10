#ifndef _U2_NETWORK_CONFIGURATION_H_
#define _U2_NETWORK_CONFIGURATION_H_


#include <QtNetwork/QNetworkProxy>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <U2Core/global.h>

#ifndef QT_NO_OPENSSL
#include <QtNetwork/QSslConfiguration>
#endif


namespace U2
{
typedef QNetworkProxy::ProxyType Proxy_t;

class U2CORE_EXPORT ProxyConfig
{
public:
    ProxyConfig(): excepted_addr_enabled(false){}
    QMap< Proxy_t, QNetworkProxy > proxyz; //only one proxy for each type
    QMap< Proxy_t, bool > proxyz_usage; //true if proxy is used
    QStringList excepted_addr;
    bool excepted_addr_enabled;
};

class U2CORE_EXPORT SslConfig
{
public:
    static const QString SSLV2;
    static const QString SSLV3;
    static const QString TLSV1;

    SslConfig() { protocols << SSLV2 << SSLV3 << TLSV1; }
    QList<QString> protocols;
    QString currentProtocol;    
};

class U2CORE_EXPORT RemoteRequestConfig 
{
public:
    static const int DEFAULT_REMOTE_REQUEST_TIMEOUT_SECONDS;

    RemoteRequestConfig() {}
    int remoteRequestTimeout;
};

class U2CORE_EXPORT NetworkConfiguration
{
public:
    NetworkConfiguration();
    ~NetworkConfiguration();
    QNetworkProxy   getProxyByUrl( const QUrl & url ) const;
    QNetworkProxy   getProxy( Proxy_t prtype ) const;
    bool            isProxyUsed( Proxy_t prtype ) const;
    void            removeProxy( Proxy_t prtype);
    int             addProxy( const QNetworkProxy & p_ ); //returns non-zero if replacing
    void            setProxyUsed( Proxy_t prtype, bool val );
    void            setExceptionsList( const QStringList & exc_addr );
    QStringList     getExceptionsList() const {return pc.excepted_addr; }
    bool            exceptionsEnabled() const {return pc.excepted_addr_enabled;}
    void            setExceptionsEnabled( bool st ) {pc.excepted_addr_enabled = st; }
    QString         getSslProtocolName() const;
    QList<QString>  getSslProtocolNames() const { return sslConfig.protocols; }
    void            setSslProtocol(const QString& name);
    void            copyFrom(const NetworkConfiguration& image);
    int             remoteRequestTimeout() const { return rrConfig.remoteRequestTimeout; }
    void            setRequestTimeout(const int timeout);

#ifndef QT_NO_OPENSSL
    QSsl::SslProtocol getSslProtocol() const;
#endif

private:
    static Proxy_t url2type( const QUrl & url );
    ProxyConfig pc;
    SslConfig sslConfig;    
    RemoteRequestConfig rrConfig;
};

} //namespace

#endif
