#include "contentwrapper.h"

#include <stdlib.h>

namespace buzz{

  ContentWrapper::ContentWrapper(const std::string& host, const std::string& lang): host(host), lang(lang){};

  ContentWrapper::~ContentWrapper(){};

  std::string ContentWrapper::GenerateLoginStart()
  {
    std::string start_string = "<stream:stream to=\'" + host + "\' ";
    start_string += "xml:lang=\'" + lang + "\' ";
    start_string += "version=\'1.0\' ";
    start_string += "xmlns:stream=\'http://etherx.jabber.org/streams\' ";
    start_string += "xmlns=\'jabber:client\'>\r\n";
    return start_string;
  }

  std::string ContentWrapper::GenerateLoginRestart()
  {
    return GenerateLoginStart();
  }

  std::string ContentWrapper::GenerateLogout()
  {
    return "</stream:stream>";
  }

  std::string ContentWrapper::GenerateRequest(const std::string& xml)
  {
    return xml;
  }

  bool ContentWrapper::HandleRecvData(std::string& xml, std::size_t len)
  {
    xml = xml.substr(0, len);
    return true;
  }

  BoshContentWrapper::BoshContentWrapper(const std::string& host, const std::string& lang, const std::string& server, int port) :
    ContentWrapper(host, lang),
    server(server),
    port(port),
    rid(0),
    sid(""),
    path("/http-bind/"),
    start_sent(false)
  {
    bosh_host = host + ":" + std::to_string((long)port);
  }

  BoshContentWrapper::~BoshContentWrapper()
  {
  }

  std::string BoshContentWrapper::GenerateLoginStart()
  {
    start_sent = true;
    std::string start_string = GetStartString();
    return WrapHtmlRequest(start_string);
  }

  std::string BoshContentWrapper::GenerateLoginRestart()
  {
    start_sent = true;
    std::string restart_string = GetRestartString();
    return WrapHtmlRequest(restart_string); 
  }

  std::string BoshContentWrapper::GenerateLogout()
  {
    std::string logout_string = GetLogoutString();
    return WrapHtmlRequest(logout_string);
  }

  std::string BoshContentWrapper::GenerateRequest(const std::string& xml)
  {
    std::string request = WrapBodyTag(xml);
    return WrapHtmlRequest(request);
  }

  std::string BoshContentWrapper::GetLogoutString()
  {
    ++rid;
    std::string requestbody = "<body";
    requestbody += "rid=\'" + std::to_string((long)rid) + "\' ";
    requestbody += "sid=\'" + sid + "\' ";
    requestbody += "type=\'terminate\'";
    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\'/>";
    rid = 0;
    sid = "";
    start_sent = false;
    return requestbody;
  }

  std::string BoshContentWrapper::GetStartString()
  {
    rid = rand() % 100000 + 1728679472;
    std::string requestbody = "<body ";
    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\' ";
    requestbody += "xmlns:xmpp=\'" + kXmlnsXmppBosh + "\' ";
    requestbody += "content=\'text/xml; charset=utf-8\' ";
    requestbody += "to=\'" + host + "\' ";
    requestbody += "rid=\'" + std::to_string((long)rid) + "\' ";
    requestbody += "route=\'xmpp:" + server + ":5222\' ";
    requestbody += "xmpp:version=\'1.0\' ";
    requestbody += "/>";
    return requestbody;

  }

  std::string BoshContentWrapper::GetRestartString()
  {
    ++rid;
    std::string requestbody = "<body ";
    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\' ";
    requestbody += "xmlns:xmpp=\'" + kXmlnsXmppBosh + "\' ";
    requestbody += "rid=\'" + std::to_string((long)rid) + "\' ";
    requestbody += "sid=\'" + sid + "\' ";
    requestbody += "xmpp:restart=\'true\' ";
    requestbody += "to=\'" + host + "\' ";
    requestbody += "/>";

    return requestbody;
  }

  std::string BoshContentWrapper::WrapBodyTag(const std::string& xml)
  {
    ++rid;
    std::string requestbody = "<body ";

    requestbody += "xmlns=\'" + kXmlnsHttpbind + "\' ";
    requestbody += "rid=\'" + std::to_string((long)rid) + "\' ";
    requestbody += "sid=\'" + sid + "\'>";
    requestbody += xml;
    requestbody += "</body>";

    return requestbody;
  }

  std::string BoshContentWrapper::WrapHtmlRequest(const std::string& str)
  {
    std::string request = "POST " + path + " HTTP/1.1\r\n";
    request += "Host: " + bosh_host + "\r\n";
    request += "Content-Type: text/xml; charset=utf-8\r\n";
    request += "Content-Length: " + std::to_string((long)str.length()) + "\r\n\r\n";
    request += str;

    return request;
  }

  bool BoshContentWrapper::CollectSid(const std::string& body)
  {
    std::size_t pos = body.find("sid");
    if (pos == std::string::npos)
      return false;
    std::string temp = body.substr(pos + 5, body.length() - pos - 5);
    pos = temp.find_first_of("\'");
    if (pos == std::string::npos)
      return false;
    sid = temp.substr(0, pos);
    return true;
  }

  bool BoshContentWrapper::HandleRecvData(std::string& str, std::size_t len)
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
    if (sid == "")
    {
      if (!CollectSid(content))
      {
        return false;
      }
    }

    // Process the return string of start/restart messages
    if (start_sent)
    {
      // Find the position of </body>
      std::size_t end_body = content.find_last_of("<");
      if (end_body == std::string::npos)
      {
        return false;
      }
      content = content.substr(0, end_body);
      start_sent = false;
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
