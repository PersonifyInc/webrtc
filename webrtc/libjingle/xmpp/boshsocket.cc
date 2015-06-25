#include "boshsocket.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include "webrtc/base/basicdefs.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/thread.h"
#include "constants.h"
#ifdef WEBRTC_POSIX
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef FEATURE_ENABLE_SSL
#include "webrtc/base/ssladapter.h"
#endif

#ifdef USE_SSLSTREAM
#include "webrtc/base/socketstream.h"
#ifdef FEATURE_ENABLE_SSL
#include "webrtc/base/sslstreamadapter.h"
#endif  // FEATURE_ENABLE_SSL
#endif  // USE_SSLSTREAM

#include "webrtc/base/socketadapters.h"

#include <iostream>

namespace buzz {

  const unsigned int kMilliToNano = 1000000;
  bool running_ = true;
  int inactivity_ = 0;
  time_t current_;

#if defined (WEBRTC_WIN)
  DWORD WINAPI DoTask(LPVOID param) {
    BoshSocket *socket = (BoshSocket *)param;

    while (running_) {
      if (!socket->OutputEmpty())
      {
        socket->TrySending();
      }
      else {
        time(&current_);
        auto current_socket = socket->GetCurrentSocket();
        auto other_socket = socket->GetOtherSocket();
        // If there was things is sent before, and we have had inactivity time.
        if (inactivity_ != 0) {
          if (current_socket != NULL) {
            if (current_socket->pending_ == false)
            {
              // Send empty request
              std::string temp = "empty";
              socket->Write(temp.c_str(), temp.length());
              socket->TrySending(current_socket);
            }
          }
          if (other_socket != NULL) {
            if (difftime(current_, other_socket->receive_time_) >= inactivity_ / 2) {
              if (other_socket->pending_ == false) {
                // Send empty request
                std::string temp = "empty";
                socket->Write(temp.c_str(), temp.length());
                socket->TrySending(other_socket);
              }
            }
          }
          Sleep(50);
        }
      }
    }
    return 0;
  }
#elif defined (WEBRTC_POSIX)
  void* DoTask(void* param) {
    BoshSocket *socket = (BoshSocket *)param;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 50 * kMilliToNano;
    while (running_) {
      if (!socket->OutputEmpty())
      {
        socket->TrySending();
      }
      else {
        time(&current_);
        auto current_socket = socket->GetCurrentSocket();
        auto other_socket = socket->GetOtherSocket();
        // If there was things is sent before, and we have had inactivity time.
        if (inactivity_ != 0) {
          if (current_socket != NULL) {
            if (current_socket->pending_ == false) {
              // Send empty request
              std::string temp = "empty";
              socket->Write(temp.c_str(), temp.length());
              socket->TrySending(current_socket);
            }
          }
          if (other_socket != NULL) {
            if (difftime(current_, other_socket->receive_time_) >= inactivity_ / 2) {
              if (other_socket->pending_ == false) {
                // Send empty request
                std::string temp = "empty";
                socket->Write(temp.c_str(), temp.length());
                socket->TrySending(other_socket);
              }
            }
          }
          nanosleep(&ts, NULL);
        }
      }
    }
    pthread_exit(NULL);
  }
#endif

  BoshSocket::BoshSocket(buzz::TlsOptions tls) : primary_socket_(NULL),
                                                 secondary_socket_(NULL),
                                                 last_socket_(NULL),
                                                 //generator_(NULL),
                                                 tls_(tls),
                                                 ssl_(false),
                                                 use_proxy_(false),
                                                 start_sent_(false)
  {
    running_ = true;
    inactivity_ = 0;
#if defined WEBRTC_WIN
    DWORD thread_id;
    HANDLE thread = CreateThread(0, 0, DoTask, this, 0, &thread_id);
    CloseHandle(thread);
#elif defined WEBRTC_POSIX
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, DoTask, (void*)this);
    pthread_detach(thread);
#endif
  }

  void BoshSocket::CreateCricketSocket(int family) {
    rtc::Thread* pth = rtc::Thread::Current();
    primary_socket_ = new ManagedSocket();
    secondary_socket_ = new ManagedSocket();
    if (family == AF_UNSPEC) {
      family = AF_INET;
    }
    rtc::AsyncSocket* first_socket =
      pth->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);
    rtc::AsyncSocket* second_socket = 
      pth->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);
#ifndef USE_SSLSTREAM
#ifdef FEATURE_ENABLE_SSL
    if (tls_ != buzz::TLS_DISABLED) {
      first_socket = rtc::SSLAdapter::Create(first_socket);
      second_socket = rtc::SSLAdapter::Create(second_socket);
    }
#endif  // FEATURE_ENABLE_SSL

    // DEBUG: wrap sockets with logging goodness
    first_socket = new rtc::LoggingSocketAdapter(first_socket, rtc::LS_SENSITIVE, "first_bosh_socket", true);
    second_socket = new rtc::LoggingSocketAdapter(second_socket, rtc::LS_SENSITIVE, "first_bosh_socket", true);

    primary_socket_->socket_ = first_socket;
    secondary_socket_->socket_ = second_socket;

    primary_socket_->socket_->SignalReadEvent.connect(this, &BoshSocket::OnReadEvent);
    primary_socket_->socket_->SignalWriteEvent.connect(this, &BoshSocket::OnWriteEvent);
    primary_socket_->socket_->SignalConnectEvent.connect(this,
      &BoshSocket::OnConnectEvent);
    primary_socket_->socket_->SignalCloseEvent.connect(this, &BoshSocket::OnCloseEvent);

    secondary_socket_->socket_->SignalReadEvent.connect(this, &BoshSocket::OnReadEvent);
    secondary_socket_->socket_->SignalWriteEvent.connect(this, &BoshSocket::OnWriteEvent);
    secondary_socket_->socket_->SignalConnectEvent.connect(this,
      &BoshSocket::OnConnectEvent);
    secondary_socket_->socket_->SignalCloseEvent.connect(this, &BoshSocket::OnCloseEvent);

#else  // USE_SSLSTREAM
    primary_socket_->socket_ = first_socket;
    secondary_socket_->socket_ = second_socket;
    primary_socket_->stream_ = new rtc::SocketStream(primary_socket_->socket_);
    secondary_socket_->stream_ = new rtc::SocketStream(secondary_socket_->socket_);
#ifdef FEATURE_ENABLE_SSL
    if (tls_ != buzz::TLS_DISABLED) {
      primary_socket_->stream_ = rtc::SSLStreamAdapter::Create(primary_socket_->stream_);
      secondary_socket_->stream_ = rtc::SSLStreamAdapter::Create(secondary_socket_->stream_);
    }
#endif  // FEATURE_ENABLE_SSL
    primary_socket_->stream_->SignalEvent.connect(this, &BoshSocket::OnEvent);
    secondary_socket_->stream_->SignalEvent.connect(this, &BoshSocket::OnEvent);
#endif  // USE_SSLSTREAM
    last_socket_ = primary_socket_;
  }


  BoshSocket::~BoshSocket()
  {
    Close();
    delete generator_;
#ifdef USE_SSLSTREAM
    delete primary_socket_->stream_;
    delete secondary_socket_->stream_;
#else  // USE_SSLSTREAM
    delete primary_socket_->socket_;
    delete secondary_socket_->socket_;
#endif
    delete primary_socket_;
    delete secondary_socket_;
    primary_socket_ = NULL;
    secondary_socket_ = NULL;
    last_socket_ = NULL;
  }

#ifndef USE_SSLSTREAM
  void BoshSocket::OnReadEvent(rtc::AsyncSocket *socket) {
    auto temp_socket = primary_socket_;
    if (temp_socket->socket_ != socket) {
      temp_socket = secondary_socket_;
    }
    temp_socket->receive_time_ = time(NULL);
    buzz::InputMsg input_msg;
    char bytes[4096];
    int len = socket->Recv(bytes, sizeof(bytes));
    if (len > 0) {
      // After receiving the return of CONNECT, signal Connected
      // sendconnect always eq true in case of ssl and proxy
      if (temp_socket->connect_sent_) {
        temp_socket->connect_sent_ = false;
        temp_socket->connected_ = true;
        temp_socket->pending_ = false;
        if (BothConnected()) {
          SignalConnected();
        }
      }
      // Else, process like normal message
      else {
        std::string str(bytes, len);
        bool is_empty_response = false;
        bool result = generator_->HandleRecvData(str, len, is_empty_response);
        if (result) {
          if (start_sent_) {
            inactivity_ = generator_->GetInactivity();
            start_sent_ = false;
          }
          if (is_empty_response) {
            input_msg.len_ = 0;
          }
          else {
            input_msg.len_ = str.length();
            input_msg.data_ = new char[str.length()];
            std::memcpy(input_msg.data_, str.c_str(), str.length());
            input_msg.recv_result_ = true;
          }
        }
        else {
          std::string error("error");
          input_msg.len_ = error.length();
          input_msg.data_ = new char[error.length()];
          std::memcpy(input_msg.data_, error.c_str(), error.length());
          input_msg.recv_result_ = true;
        }
        temp_socket->pending_ = false;
        if (input_msg.len_ != 0) {
          input_queue_.push(input_msg);
          SignalRead();
        }
      }
    }
  }

  void BoshSocket::OnWriteEvent(rtc::AsyncSocket * socket) {
    // In case no message to send, then return
    if (output_queue_.empty())
    {
      return;
    }
    auto temp_socket = primary_socket_;
    if (temp_socket->socket_ != socket) {
      temp_socket = secondary_socket_;
    }
    std::string temp_data(output_queue_.front().data_, output_queue_.front().len_);
    std::string generated_data = temp_data;
    output_queue_.pop();
    if (temp_data.compare(0, 7, "CONNECT") != 0) {
      if (temp_data == "end") {
        generated_data = generator_->GenerateLogout();
      }
      else if (temp_data == "start") {
        generated_data = generator_->GenerateLoginStart();
        start_sent_ = true;
      }
      else if (temp_data == "restart") {
        generated_data = generator_->GenerateLoginRestart();
      }
      else if (temp_data == "empty") {
        generated_data = generator_->GenerateEmptyRequest();
        std::cout << "socket: " << std::hex << this << std::dec << " RID: " << generator_->GetRid() << " genlen: " << generated_data.length() << std::endl;
      }
      else {
        generated_data = generator_->GenerateRequest(temp_data);
      }
    }
    else {
      temp_socket->connect_sent_ = true;
    }
    buffer_.WriteBytes(generated_data.c_str(), generated_data.length());
    if (buffer_.Length() != 0) {
      while (buffer_.Length() != 0) {
        int written = socket->Send(buffer_.Data(), buffer_.Length());
        if (written > 0) {
          buffer_.Consume(written);
          continue;
        }
        if (!socket->IsBlocking())
          LOG(LS_ERROR) << "Send error: " << socket->GetError();
        return;
      }
      temp_socket->pending_ = true;
    }
  }

  void BoshSocket::OnConnectEvent(rtc::AsyncSocket * socket) {
    auto temp_socket = primary_socket_;
    if (temp_socket->socket_ != socket) {
      temp_socket = secondary_socket_;
    }
#if defined(FEATURE_ENABLE_SSL)
    if (temp_socket->state_ == buzz::AsyncSocket::STATE_TLS_CONNECTING) {
      temp_socket->state_ = buzz::AsyncSocket::STATE_TLS_OPEN;
      temp_socket->connected_ = true;
      temp_socket->pending_ = false;
      SignalSSLConnected();
      return;
    }
#endif  // !defined(FEATURE_ENABLE_SSL)
    temp_socket->state_ = buzz::AsyncSocket::STATE_OPEN;
    if (use_proxy_ && ssl_ && !temp_socket->connect_sent_) {
      SendConnect(temp_socket);
    }
    else {
      temp_socket->connected_ = true;
      temp_socket->pending_ = false;
      if (BothConnected()) {
        SignalConnected();
      }
    }
  }

  void BoshSocket::OnCloseEvent(rtc::AsyncSocket * socket, int error) {
    SignalCloseEvent(error);
  }

#else
  void BoshSocket::OnEvent(rtc::StreamInterface* stream,
                           int events, int err) {
    auto temp_socket = primary_socket_;
    if(temp_socket->stream_ != stream) {
      temp_socket = secondary_socket_;
    }

    if ((events & rtc::SE_OPEN)) {
#if defined(FEATURE_ENABLE_SSL)
      if (temp_socket->state_ == buzz::AsyncSocket::STATE_TLS_CONNECTING) {
        temp_socket->state_ = buzz::AsyncSocket::STATE_TLS_OPEN;
        temp_socket->connected_ = true;
        temp_socket->pending_ = false;
        SignalSSLConnected();
        return;
      }
#endif
      temp_socket->state_ = buzz::AsyncSocket::STATE_OPEN;
      if(use_proxy_ && ssl_ && !temp_socket->connect_sent_) {
        SendConnect(temp_socket);
      }
      else {
        temp_socket->connected_ = true;
        temp_socket->pending_ = false;
        if (BothConnected()) {
          SignalConnected();
        }
        return;
      }
    }
    if ((events & rtc::SE_READ)) {
      temp_socket->last_receive_ = time(NULL);
      buzz::InputMsg input_msg;
      char bytes[4096];
      size_t read;
      rtc::StreamResult result = stream->Read(bytes, sizeof(bytes), &read, NULL);
      if (result == rtc::SR_SUCCESS) 
      {
        // After receiving the return of CONNECT, signal Connected
        // sendconnect always eq true in case of ssl and proxy
        if (temp_socket->connect_sent_) {
          temp_socket->connect_sent_ = false;
          temp_socket->connected_ = true;
          temp_socket->pending_ = false;
          if (BothConnected()) {
            SignalConnected();
          }
        }
        // Else, process like normal message
        else {
          std::string str(bytes, len);
          bool is_empty_response = false;
          bool result = generator_->HandleRecvData(str, len, is_empty_response);
          if (result) {
            if (start_sent_) {
              inactivity_ = generator_->GetInactivity();
              start_sent_ = false;
            }
            if (is_empty_response) {
              input_msg.len_ = 0;
            }
            else {
              input_msg.len_ = str.length();
              input_msg.data_ = new char[str.length()];
              std::memcpy(input_msg.data_, str.c_str(), str.length());
              input_msg.recv_result_ = true;
            }
          }
          else {
            std::string error("error");
            input_msg.len_ = error.length();
            input_msg.data_ = new char[error.length()];
            std::memcpy(input_msg.data_, error.c_str(), error.length());
            input_msg.recv_result_ = true;
          }
          temp_socket->pending_ = false;
          if (input_msg.len_ != 0) {
            input_queue_.push(input_msg);
            SignalRead();
          }
        }
      }
    }
    if ((events & rtc::SE_WRITE)) {
      if (output_queue_.empty())
      {
        return;
      }
      std::string temp_data(output_queue_.front().data_, output_queue_.front().len_);
      std::string generated_data = temp_data;
      output_queue_.pop();
      if (temp_data.compare(0, 7, "CONNECT") != 0) {
        if (temp_data == "end") {
          generated_data = generator_->GenerateLogout();
        }
        else if (temp_data == "start") {
          generated_data = generator_->GenerateLoginStart();
          start_sent_ = true;
        }
        else if (temp_data == "restart") {
          generated_data = generator_->GenerateLoginRestart();
        }
        else if (temp_data == "empty") {
          generated_data = generator_->GenerateEmptyRequest();
          std::cout << "socket: " << std::hex << this << std::dec << " RID: " << generator_->GetRid() << " genlen: " << generated_data.length() << std::endl;
        }
        else {
          generated_data = generator_->GenerateRequest(temp_data);
        }
      }
      else {
        temp_socket->connect_sent_ = true;
      }
      buffer_.WriteBytes(generated_data.c_str(), generated_data.length());

      if(buffer_.Length() != 0) {
        while (buffer_.Length() != 0) {
          rtc::StreamResult result;
          size_t written;
          int error;
          result = stream->Write(buffer_.Data(), buffer_.Length(),
            &written, &error);
          if (result == rtc::SR_ERROR) {
            LOG(LS_ERROR) << "Send error: " << error;
            return;
          }
          if (result == rtc::SR_BLOCK)
            return;
          ASSERT(result == rtc::SR_SUCCESS);
          ASSERT(written > 0);
          buffer_.Consume(written);
        }
        temp_socket->pending_ = true;
      }
    }
    else if ((events & rtc::SE_CLOSE))
      SignalCloseEvent(err);
  }
#endif

  buzz::AsyncSocket::State BoshSocket::state() {
    return last_socket_->state_;
  }

  buzz::AsyncSocket::Error BoshSocket::error() {
    return buzz::AsyncSocket::ERROR_NONE;
  }

  int BoshSocket::GetError() {
    return 0;
  }

  bool BoshSocket::BothPending() {
    return primary_socket_->pending_ && secondary_socket_->pending_;
  }

  bool BoshSocket::BothConnected() {
    return primary_socket_->connected_ && secondary_socket_->connected_;
  }

  buzz::ManagedSocket* BoshSocket::GetSocket() {
    if (last_socket_ == NULL) {
      last_socket_ = primary_socket_;
    }
    else if (last_socket_->pending_) {
      last_socket_ = (last_socket_ == primary_socket_) ? secondary_socket_ : primary_socket_;
    }
    return last_socket_;
  }

  bool BoshSocket::Connect(const rtc::SocketAddress& addr) {
    rtc::SocketAddress local_addr = addr;
    server_host_ = addr.hostname();
    server_port_ = (int)addr.port();
    if (addr.port() == buzz::HTTPS_PORT) {
      ssl_ = true;
    }
    if (use_proxy_) {
      local_addr = rtc::SocketAddress(proxy_host_, proxy_port_);
    }
    generator_ = new BoshXmppStanzaGenerator(domain_, lang_, addr.hostname(), addr.port());
    if (primary_socket_ == NULL && secondary_socket_ == NULL) {
      CreateCricketSocket(local_addr.family());
    }
    if (primary_socket_->socket_->Connect(local_addr) < 0) {
      return primary_socket_->socket_->IsBlocking();
    }
    if (secondary_socket_->socket_->Connect(local_addr) < 0) {
      return secondary_socket_->socket_->IsBlocking();
    }
    return true;
  }

  bool BoshSocket::Read(char * data, size_t len, size_t* len_read) {
    if (input_queue_.size() == 0) {
      return false;
    }
    memcpy(data, input_queue_.front().data_, input_queue_.front().len_);
    *len_read = input_queue_.front().len_;
    bool result = input_queue_.front().recv_result_;
    input_queue_.pop();
    return result;
  }

  bool BoshSocket::Write(const char * data, size_t len) {
    buzz::OutputMsg msg(data, len);
    output_queue_.push(msg);
    return true;
  }

  void BoshSocket::TrySending() {
    if (BothConnected()) {
      if (!BothPending()) {
        auto temp_socket = GetSocket();
#ifndef USE_SSLSTREAM
        OnWriteEvent(temp_socket->socket_);
#else // USE_SSLSTREAM
        OnEvent(temp_socket->stream_, rtc::SE_WRITE, 0);
#endif // USE_SSLSTREAM
      }
    }
  }

  void BoshSocket::TrySending(ManagedSocket* socket)
  {
    if (BothConnected()) {
#ifndef USE_SSLSTREAM
      OnWriteEvent(socket->socket_);
#else // USE_SSLSTREAM
      OnEvent(socket->stream_, rtc::SE_WRITE, 0);
#endif // USE_SSLSTREAM
    }
  }

  bool BoshSocket::Close() {
    // Set running flag to false to stop thread
    running_ = false;
    // Flush all remaining message in output queue.
    while (!OutputEmpty()) {
      TrySending();
    }
    if ((primary_socket_->state_ != buzz::AsyncSocket::STATE_OPEN) || (secondary_socket_->state_ != buzz::AsyncSocket::STATE_OPEN))
      return false;
#ifndef USE_SSLSTREAM
    if ((primary_socket_->socket_->Close() == 0) && (secondary_socket_->socket_->Close() == 0)) {
      primary_socket_->state_ = buzz::AsyncSocket::STATE_CLOSED;
      secondary_socket_->state_ = buzz::AsyncSocket::STATE_CLOSED;
      primary_socket_->connected_ = false;
      secondary_socket_->connected_ = false;
      SignalClosed();
      return true;
    }
    return false;
#else  // USE_SSLSTREAM
    primary_socket_->state_ = buzz::AsyncSocket::STATE_CLOSED;
    secondary_socket_->state_ = buzz::AsyncSocket::STATE_CLOSED;
    primary_socket_->stream_->Close();
    secondary_socket_->stream_->Close();
    primary_socket_->connected_ = false;
    secondary_socket_->connected_ = false;
    SignalClosed();
    return true;
#endif  // USE_SSLSTREAM
  }

  bool BoshSocket::StartTls(const std::string & domainname) {
    StartTls(primary_socket_, domainname);
    StartTls(secondary_socket_, domainname);
    return true;
  }

  bool BoshSocket::StartTls(buzz::ManagedSocket* socket, const std::string & domainname) {
#if defined(FEATURE_ENABLE_SSL)
    if (tls_ == buzz::TLS_DISABLED)
      return false;
#ifndef USE_SSLSTREAM
    rtc::SSLAdapter* ssl_adapter =
      static_cast<rtc::SSLAdapter *>(socket->socket_);
    if (ssl_adapter->StartSSL(domainname.c_str(), false) != 0) {
      return false;
    }
#else  // USE_SSLSTREAM
    rtc::SSLStreamAdapter* ssl_stream =
      static_cast<rtc::SSLStreamAdapter *>(socket->stream_);
    if (ssl_stream->StartSSLWithServer(domainname.c_str()) != 0) {
      return false;
    }
#endif  // USE_SSLSTREAM
    socket->state_ = buzz::AsyncSocket::STATE_TLS_CONNECTING;
    // Set the connected status to false
    socket->connected_ = false;
    return true;
#else  // !defined(FEATURE_ENABLE_SSL)
    return false;
#endif  // !defined(FEATURE_ENABLE_SSL)
  }

  void BoshSocket::SetProxy(const std::string & host, int port) {
    if (!host.empty()) {
      use_proxy_ = true;
      proxy_host_ = host;
      proxy_port_ = port;
      LOG(LS_INFO) << "Proxy detected: Host - " << proxy_host_ << " Port - " << proxy_port_;
    }
  }

  void BoshSocket::SetInfo(const std::string & domain,
                           const std::string & lang) {
    domain_ = domain;
    lang_ = lang;
  }

  void BoshSocket::SendConnect(buzz::ManagedSocket* socket) {
    std::stringstream ss;
    ss << "CONNECT " << server_host_ << ":" << server_port_ << " HTTP/1.1\r\n";
    ss << "Host: " << server_host_ << ":" << server_port_ << "\r\n";
    ss << "Proxy-Connection: keep-alive\r\n\r\n";

    buzz::OutputMsg msg(ss.str().c_str(), ss.str().length());
    output_queue_.push(msg);
#ifndef USE_SSLSTREAM
    OnWriteEvent(socket->socket_);
#else // USE_SSLSTREAM
    OnEvent(socket->stream_, rtc::SE_WRITE, 0);
#endif // USE_SSLSTREAM
  }
}
