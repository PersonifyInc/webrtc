#ifndef TALK_XMPP_SASLANONMECHANISM_H_
#define TALK_XMPP_SASLANONMECHANISM_H_

#include "talk/base/cryptstring.h"
#include "talk/xmpp/saslmechanism.h"

namespace buzz {

class SaslAnonMechanism : public SaslMechanism {

public:
  SaslAnonMechanism() {}

  virtual std::string GetMechanismName() { return "ANONYMOUS"; }

  virtual XmlElement * StartSaslAuth() {
    // send initial request
    XmlElement * el = new XmlElement(QN_SASL_AUTH, true);
    el->AddAttr(QN_MECHANISM, "ANONYMOUS");
    return el;
  }

private:
};

}

#endif  // TALK_XMPP_SASLANONMECHANISM_H_
