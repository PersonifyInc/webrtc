#ifndef _ANONSASLHANDLER_H_
#define _ANONSASLHANDLER_H_

#include "talk/xmpp/saslhandler.h"
#include <algorithm>

namespace buzz {

class AnonSaslHandler : public SaslHandler {
public:
  AnonSaslHandler() {}
    
  virtual ~AnonSaslHandler() {}

  // Should pick the best method according to this handler
  // returns the empty string if none are suitable
  virtual std::string ChooseBestSaslMechanism(const std::vector<std::string> & mechanisms, bool encrypted) {
    std::vector<std::string>::const_iterator it = std::find(mechanisms.begin(), mechanisms.end(), "ANONYMOUS");
    if (it == mechanisms.end()) {
      return "";
    }
    else {
      return "ANONYMOUS";
    }
  }

  // Creates a SaslMechanism for the given mechanism name (you own it
  // once you get it).  If not handled, return NULL.
  virtual SaslMechanism * CreateSaslMechanism(const std::string & mechanism) {
    if (mechanism == "ANONYMOUS") {
      return new SaslAnonMechanism();
    }
    return NULL;
  }
  
private:
};


}

#endif
