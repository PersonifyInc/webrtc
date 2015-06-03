#include "webrtc\libjingle\xmpp\xmppstanzagenerator.h"
#include <stdlib.h>
#include <time.h>

namespace buzz{

  XmppStanzaGenerator::XmppStanzaGenerator(const std::string& host, const std::string& lang) : host_(host), lang_(lang){};

  XmppStanzaGenerator::~XmppStanzaGenerator(){};

  std::string XmppStanzaGenerator::GenerateLoginStart()
  {
    std::string start_string = "<stream:stream to=\'" + host_ + "\' ";
    start_string += "xml:lang=\'" + lang_ + "\' ";
    start_string += "version=\'1.0\' ";
    start_string += "xmlns:stream=\'http://etherx.jabber.org/streams\' ";
    start_string += "xmlns=\'jabber:client\'>\r\n";
    return start_string;
  }

  std::string XmppStanzaGenerator::GenerateLoginRestart()
  {
    return GenerateLoginStart();
  }

  std::string XmppStanzaGenerator::GenerateLogout()
  {
    return "</stream:stream>";
  }

  std::string XmppStanzaGenerator::GenerateRequest(const std::string& xml)
  {
    return xml;
  }

  bool XmppStanzaGenerator::HandleRecvData(std::string& xml, std::size_t len)
  {
    xml = xml.substr(0, len);
    return true;
  }

  BoshXmppStanzaGenerator::BoshXmppStanzaGenerator(const std::string& host, const std::string& lang, const std::string& server, int port) :
    XmppStanzaGenerator(host, lang),
    server_(server),
    port_(port),
    rid_(0),
    sid_(""),
    start_sent_(false)
  {
    path_ = (port == buzz::HTTP_PORT) ? "http://" : "https://";
    path_ += server + "/http-bind/";
    bosh_host_ = host + ":" + std::to_string((long)port);
  }

  BoshXmppStanzaGenerator::~BoshXmppStanzaGenerator()
  {
  }

  std::string BoshXmppStanzaGenerator::GenerateLoginStart()
  {
    start_sent_ = true;
    std::string start_string = GetStartString();
    return WrapHtmlRequest(start_string);
  }

  std::string BoshXmppStanzaGenerator::GenerateLoginRestart()
  {
    start_sent_ = true;
    std::string restart_string = GetRestartString();
    return WrapHtmlRequest(restart_string);
  }

  std::string BoshXmppStanzaGenerator::GenerateLogout()
  {
    std::string logout_string = GetLogoutString();
    return WrapHtmlRequest(logout_string);
  }

  std::string BoshXmppStanzaGenerator::GenerateRequest(const std::string& xml)
  {
    std::string request = WrapBodyTag(xml);
    return WrapHtmlRequest(request);
  }

  std::string BoshXmppStanzaGenerator::GetLogoutString()
  {
    ++rid_;
    std::string requestbody = "<body";
    requestbody += "rid=\'" + std::to_string((long)rid_) + "\' ";
    requestbody += "sid=\'" + sid_ + "\' ";
    requestbody += "type=\'terminate\'";
    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\'/>";
    rid_ = 0;
    sid_ = "";
    start_sent_ = false;
    return requestbody;
  }

  std::string BoshXmppStanzaGenerator::GetStartString()
  {
    srand(time(NULL));
    rid_ = (long)rand();

    std::string requestbody = "<body ";
    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\' ";
    requestbody += "xmlns:xmpp=\'" + kXmlnsXmppBosh + "\' ";
    requestbody += "content=\'text/xml; charset=utf-8\' ";
    requestbody += "to=\'" + host_ + "\' ";
    requestbody += "rid=\'" + std::to_string((long)rid_) + "\' ";
    requestbody += "route=\'xmpp:" + server_ + ":" + std::to_string(XMPP_PORT) + "\' ";
    requestbody += "xmpp:version=\'1.0\' ";
    requestbody += "/>";
    return requestbody;

  }

  std::string BoshXmppStanzaGenerator::GetRestartString()
  {
    ++rid_;
    std::string requestbody = "<body ";
    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\' ";
    requestbody += "xmlns:xmpp=\'" + kXmlnsXmppBosh + "\' ";
    requestbody += "rid=\'" + std::to_string((long)rid_) + "\' ";
    requestbody += "sid=\'" + sid_ + "\' ";
    requestbody += "xmpp:restart=\'true\' ";
    requestbody += "to=\'" + host_ + "\' ";
    requestbody += "/>";

    return requestbody;
  }

  std::string BoshXmppStanzaGenerator::WrapBodyTag(const std::string& xml)
  {
    ++rid_;
    std::string requestbody = "<body ";

    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\' ";
    requestbody += "rid=\'" + std::to_string((long)rid_) + "\' ";
    requestbody += "sid=\'" + sid_ + "\'>";
    requestbody += xml;
    requestbody += "</body>";

    return requestbody;
  }

  std::string BoshXmppStanzaGenerator::WrapHtmlRequest(const std::string& str)
  {
    std::string request = "POST " + path_ + " HTTP/1.1\r\n";
    request += "Host: " + bosh_host_ + "\r\n";
    request += "Content-Type: text/xml; charset=utf-8\r\n";
    request += "Content-Length: " + std::to_string((long)str.length()) + "\r\n\r\n";
    request += str;

    return request;
  }

  bool BoshXmppStanzaGenerator::CollectSid(const std::string& body)
  {
    std::size_t pos = body.find("sid");
    if (pos == std::string::npos)
      return false;
    std::string temp = body.substr(pos + 5, body.length() - pos - 5);
    pos = temp.find_first_of("\'");
    if (pos == std::string::npos)
      return false;
    sid_ = temp.substr(0, pos);
    return true;
  }

  bool BoshXmppStanzaGenerator::HandleRecvData(std::string& str, std::size_t len)
  {
    std::string content = str.substr(0, len);
    // Find the position of <body...
    std::size_t start_body = content.find_first_of("<");
    if (start_body == std::string::npos)
    {
      return false;
    }
    content = content.substr(start_body, len - start_body);

    // Collect the sid of message
    if (sid_ == "")
    {
      if (!CollectSid(content))
      {
        return false;
      }
    }

    // Process the return string of start/restart messages
    if (start_sent_)
    {
      // Find the position of </body>
      std::size_t end_body = content.find_last_of("<");
      if (end_body == std::string::npos)
      {
        return false;
      }
      content = content.substr(0, end_body);
      start_sent_ = false;
    }
    // Process the return string of other messages
    else
    {
      //Remove the <body...> and </body> tags
      std::size_t start = content.find_first_of(">");
      std::size_t stop = content.find_last_of("<");
      if (start == std::string::npos || stop == std::string::npos)
      {
        return false;
      }
      content = content.substr(start + 1, stop - start - 1);
    }
    str = content;
    return true;
  }
}
