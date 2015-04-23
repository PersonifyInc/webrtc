#ifndef TALK_XMPP_CONTENTWRAPPER_H_
#define TALK_XMPP_CONTENTWRAPPER_H_

#include <string>

namespace buzz{
  class ContentWrapper
  {
  public:
    ContentWrapper(const std::string& host, const std::string& lang);
    ~ContentWrapper();
    virtual std::string GenerateLoginStart();
    virtual std::string GenerateLoginRestart();
    virtual std::string GenerateLogout();
    virtual std::string GenerateRequest(const std::string& str);
    virtual bool HandleRecvData(std::string& str, std::size_t len);
  protected:
    std::string host;
    std::string lang;
  };

  class BoshContentWrapper : public ContentWrapper
  {
  public:
    BoshContentWrapper(const std::string& host, const std::string& lang, const std::string& server, int port);
    ~BoshContentWrapper();
    std::string GenerateLoginStart();
    std::string GenerateLoginRestart();
    std::string GenerateLogout();
    std::string GenerateRequest(const std::string& str);
    bool HandleRecvData(std::string& str, std::size_t len);

  private:
    bool CollectSid(const std::string& body);
    std::string GetStartString();
    std::string GetRestartString();
    std::string GetLogoutString();
    std::string WrapBodyTag(const std::string& str);
    std::string WrapHtmlRequest(const std::string& str);

    const std::string kXmlnsHttpbind = "http://jabber.org/protocol/httpbind";
    const std::string kXmlnsXmppBosh = "urn:xmpp:xbosh";
    unsigned long rid;
    std::string sid;
    std::string path;
    std::string server;
    int port;
    std::string bosh_host;
    bool start_sent;
  };
}

#endif  // TALK_XMPP_CONTENTWRAPPER_H_
