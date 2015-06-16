#ifndef WEBRTC_LIBJINGLE_XMPP_XMPPSTANZAGENERATOR_H_
#define WEBRTC_LIBJINGLE_XMPP_XMPPSTANZAGENERATOR_H_
#include "webrtc\libjingle\xmpp\constants.h"
#include <string>

namespace buzz{
  class XmppStanzaGenerator
  {
  public:
    XmppStanzaGenerator(const std::string& host, const std::string& lang);
    ~XmppStanzaGenerator();
    virtual std::string GenerateLoginStart();
    virtual std::string GenerateLoginRestart();
    virtual std::string GenerateLogout();
    virtual std::string GenerateRequest(const std::string& str);
    virtual bool HandleRecvData(std::string& str, std::size_t len, bool& is_empty_response);
  protected:
    std::string host_;
    std::string lang_;
  };

  class BoshXmppStanzaGenerator : public XmppStanzaGenerator
  {
  public:
    BoshXmppStanzaGenerator(const std::string& host, const std::string& lang, const std::string& server, int port);
    ~BoshXmppStanzaGenerator();
    std::string GenerateLoginStart();
    std::string GenerateLoginRestart();
    std::string GenerateEmptyRequest();
    std::string GenerateLogout();
    std::string GenerateRequest(const std::string& str);
    int GetInactivity() {return inactivity_;}
    int GetRid() { return rid_; }
    bool HandleRecvData(std::string& str, std::size_t len, bool& is_empty_response);

  private:
    bool CollectData(const std::string& body);
    std::string GetStartString();
    std::string GetRestartString();
    std::string GetEmptyBody();
    std::string GetLogoutString();
    std::string WrapBodyTag(const std::string& str);
    std::string WrapHtmlRequest(const std::string& str);

    const std::string kXmlnsHttpbind = "http://jabber.org/protocol/httpbind";
    const std::string kXmlnsXmppBosh = "urn:xmpp:xbosh";
    unsigned long rid_;
    std::string sid_;
    int wait_;
    int hold_;
    int inactivity_;
    std::string path_;
    std::string server_;
    int port_;
    std::string bosh_host_;
    bool start_sent_;
  };
}

#endif  // WEBRTC_LIBJINGLE_XMPP_XMPPSTANZAGENERATOR_H_
