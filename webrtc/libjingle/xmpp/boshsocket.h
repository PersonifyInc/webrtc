#ifndef WEBRTC_LIBJINGLE_XMPP_BOSHSOCKET_H_
#define WEBRTC_LIBJINGLE_XMPP_BOSHSOCKET_H_

#include "webrtc/libjingle/xmpp/asyncsocket.h"
#include "webrtc/libjingle/xmpp/xmppengine.h"
#include "webrtc/base/asyncsocket.h"
#include "webrtc/base/bytebuffer.h"
#include "webrtc/base/sigslot.h"
#include <queue>
#include <string>

namespace rtc {
  class StreamInterface;
  class SocketAddress;
}

namespace buzz {
  struct ManagedSocket{
    rtc::AsyncSocket *socket_;
#ifdef USE_SSLSTREAM
    rtc::StreamInterface *stream_;
#endif // USE_SSLSTREAM
    bool pending_;
    bool connected_;
    bool connect_sent_;  // CONNECT sent, required when use ssl under proxy server.
    buzz::AsyncSocket::State state_;

    ManagedSocket() {
      socket_ = NULL;
#ifdef USE_SSLSTREAM
      stream_ = NULL;
#endif // USE_SSLSTREAM
      pending_ = true;
      connected_ = false;
      connect_sent_ = false;
      state_ = buzz::AsyncSocket::State::STATE_CLOSED;
    }
  };

  struct InputMsg {
    char *data_;
    size_t len_;
    bool recv_result_;
    InputMsg() : data_(NULL), len_(0), recv_result_(false) {
    }

    InputMsg(const InputMsg& msg) {
      data_ = new char[msg.len_];
      memcpy(data_, msg.data_, msg.len_);
      len_ = msg.len_;
      recv_result_ = msg.recv_result_;
    }

    ~InputMsg() {
      delete[] data_;
    }
  };

  struct OutputMsg{
    char *data_;
    size_t len_;
    OutputMsg(const char *str, size_t len) {
      data_ = new char[len];
      std::memcpy(data_, str, len);
      len_ = len;
    }

    OutputMsg(const OutputMsg& msg) {
      data_ = new char[msg.len_];
      memcpy(data_, msg.data_, msg.len_);
      len_ = msg.len_;
    }

    ~OutputMsg() {
      delete[] data_;
    }
  };

  class BoshSocket : public AsyncSocket, public sigslot::has_slots<> {
  public:
    BoshSocket(buzz::TlsOptions tls);
    ~BoshSocket();
    virtual buzz::AsyncSocket::State state();
    virtual buzz::AsyncSocket::Error error();
    virtual int GetError();

    virtual bool Connect(const rtc::SocketAddress& addr);
    virtual bool Read(char * data, size_t len, size_t* len_read);
    virtual bool Write(const char * data, size_t len);
    virtual bool Close();
    virtual bool StartTls(const std::string & domainname);
    virtual void SetProxy(const std::string & host, int port);

    sigslot::signal1<int> SignalCloseEvent;

  private:
    bool BothPending();
    bool BothConnected();
    void CreateCricketSocket(int family);
    buzz::ManagedSocket* GetSocket();
    void SendConnect(buzz::ManagedSocket* socket); // Send CONNECT to proxy server, in case HTTPS under proxy
    bool StartTls(buzz::ManagedSocket* socket, const std::string & domainname);
#ifndef USE_SSLSTREAM
    void OnReadEvent(rtc::AsyncSocket * socket);
    void OnWriteEvent(rtc::AsyncSocket * socket);
    void OnConnectEvent(rtc::AsyncSocket * socket);
    void OnCloseEvent(rtc::AsyncSocket * socket, int error);
#else  // USE_SSLSTREAM
    void OnEvent(rtc::StreamInterface* stream, int events, int err);
#endif  // USE_SSLSTREAM
    
    buzz::ManagedSocket *primary_socket_, *secondary_socket_, *last_socket_;
    rtc::ByteBuffer buffer_;
    buzz::TlsOptions tls_;

    std::queue<buzz::OutputMsg> output_queue_;
    std::queue<buzz::InputMsg> input_queue_;

    bool ssl_;
    bool use_proxy_;
    std::string proxy_host_;
    int proxy_port_;
    std::string server_host_;
    int server_port_;
  };
} // namespace buzz

#endif // WEBRTC_LIBJINGLE_XMPP_BOSHSOCKET_H_
