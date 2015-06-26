/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_SSLADAPTER_H_
#define WEBRTC_BASE_SSLADAPTER_H_

#include "webrtc/base/asyncsocket.h"
#include "webrtc/base/socketadapters.h"
#include "webrtc/base/logging.h"

namespace rtc {

///////////////////////////////////////////////////////////////////////////////

class SSLAdapter : public AsyncSocketAdapter {
 public:
  explicit SSLAdapter(AsyncSocket* socket)
    : AsyncSocketAdapter(socket), ignore_bad_cert_(false) { }

  bool ignore_bad_cert() const { return ignore_bad_cert_; }
  void set_ignore_bad_cert(bool ignore) { ignore_bad_cert_ = ignore; }

  // StartSSL returns 0 if successful.
  // If StartSSL is called while the socket is closed or connecting, the SSL
  // negotiation will begin as soon as the socket connects.
  virtual int StartSSL(const char* hostname, bool restartable) = 0;

  // Create the default SSL adapter for this platform. On failure, returns NULL
  // and deletes |socket|. Otherwise, the returned SSLAdapter takes ownership
  // of |socket|.
  static SSLAdapter* Create(AsyncSocket* socket);
  
  static SSLAdapter* CreateLogged(AsyncSocket* socket);

 private:
  // If true, the server certificate need not match the configured hostname.
  bool ignore_bad_cert_;
};

///////////////////////////////////////////////////////////////////////////////

typedef bool (*VerificationCallback)(void* cert);

// Call this on the main thread, before using SSL.
// Call CleanupSSLThread when finished with SSL.
bool InitializeSSL(VerificationCallback callback = NULL);

// Call to initialize additional threads.
bool InitializeSSLThread();

// Call to cleanup additional threads, and also the main thread.
bool CleanupSSL();

///////////////////////////////////////////////////////////////////////////////

// Implements a SSL socket adapter that logs everything it sends and receives (pre/post encryption/decryption)
class LoggingSSLSocketAdapter : public SSLAdapter
{
 public:
  LoggingSSLSocketAdapter(SSLAdapter* socket, LoggingSeverity level,
                 const char * label, bool hex_mode = false);

  virtual int Send(const void *pv, size_t cb);
  virtual int SendTo(const void *pv, size_t cb, const SocketAddress& addr);
  virtual int Recv(void *pv, size_t cb);
  virtual int RecvFrom(void *pv, size_t cb, SocketAddress *paddr);
  virtual int Close();
  virtual int StartSSL(const char* hostname, bool restartable);
  
 protected:
  virtual void OnConnectEvent(SSLAdapter * socket);
  virtual void OnCloseEvent(SSLAdapter * socket, int err);

 private:
  LoggingSeverity level_;
  std::string label_;
  bool hex_mode_;
  LogMultilineState lms_;
  DISALLOW_EVIL_CONSTRUCTORS(LoggingSSLSocketAdapter);

};    


}  // namespace rtc

#endif  // WEBRTC_BASE_SSLADAPTER_H_
