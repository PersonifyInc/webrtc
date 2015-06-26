/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif  // HAVE_CONFIG_H

#include "webrtc/base/ssladapter.h"

#include "webrtc/base/sslconfig.h"

#if SSL_USE_SCHANNEL

#include "schanneladapter.h"

#elif SSL_USE_OPENSSL  // && !SSL_USE_SCHANNEL

#include "openssladapter.h"

#elif SSL_USE_NSS     // && !SSL_USE_CHANNEL && !SSL_USE_OPENSSL

#include "nssstreamadapter.h"

#endif  // SSL_USE_OPENSSL && !SSL_USE_SCHANNEL && !SSL_USE_NSS

///////////////////////////////////////////////////////////////////////////////

namespace rtc {

SSLAdapter*
SSLAdapter::Create(AsyncSocket* socket) {
#if SSL_USE_SCHANNEL
  return new SChannelAdapter(socket);
#elif SSL_USE_OPENSSL  // && !SSL_USE_SCHANNEL
  return new OpenSSLAdapter(socket);
#else  // !SSL_USE_OPENSSL && !SSL_USE_SCHANNEL
  delete socket;
  return NULL;
#endif  // !SSL_USE_OPENSSL && !SSL_USE_SCHANNEL
}

SSLAdapter*
SSLAdapter::CreateLogged(AsyncSocket* socket) {
  return new LoggingSSLSocketAdapter(Create(socket), LS_SENSITIVE, "ssl_socket", true);
}

///////////////////////////////////////////////////////////////////////////////

#if SSL_USE_OPENSSL

bool InitializeSSL(VerificationCallback callback) {
  return OpenSSLAdapter::InitializeSSL(callback);
}

bool InitializeSSLThread() {
  return OpenSSLAdapter::InitializeSSLThread();
}

bool CleanupSSL() {
  return OpenSSLAdapter::CleanupSSL();
}

#elif SSL_USE_NSS  // !SSL_USE_OPENSSL

bool InitializeSSL(VerificationCallback callback) {
  return NSSContext::InitializeSSL(callback);
}

bool InitializeSSLThread() {
  return NSSContext::InitializeSSLThread();
}

bool CleanupSSL() {
  return NSSContext::CleanupSSL();
}

#else  // !SSL_USE_OPENSSL && !SSL_USE_NSS

bool InitializeSSL(VerificationCallback callback) {
  return true;
}

bool InitializeSSLThread() {
  return true;
}

bool CleanupSSL() {
  return true;
}

#endif  // !SSL_USE_OPENSSL && !SSL_USE_NSS

///////////////////////////////////////////////////////////////////////////////


LoggingSSLSocketAdapter::LoggingSSLSocketAdapter(SSLAdapter* socket,
                                           LoggingSeverity level,
                                           const char * label, bool hex_mode)
    : SSLAdapter(socket), level_(level), hex_mode_(hex_mode) {
  label_.append("[");
  label_.append(label);
  label_.append("]");
}

int LoggingSSLSocketAdapter::Send(const void *pv, size_t cb) {
  int res = SSLAdapter::Send(pv, cb);
  if (res > 0)
    LogMultiline(level_, label_.c_str(), false, pv, res, hex_mode_, &lms_);
  return res;
}

int LoggingSSLSocketAdapter::SendTo(const void *pv, size_t cb,
                             const SocketAddress& addr) {
  int res = SSLAdapter::SendTo(pv, cb, addr);
  if (res > 0)
    LogMultiline(level_, label_.c_str(), false, pv, res, hex_mode_, &lms_);
  return res;
}

int LoggingSSLSocketAdapter::Recv(void *pv, size_t cb) {
  int res = SSLAdapter::Recv(pv, cb);
  if (res > 0)
    LogMultiline(level_, label_.c_str(), true, pv, res, hex_mode_, &lms_);
  return res;
}

int LoggingSSLSocketAdapter::RecvFrom(void *pv, size_t cb, SocketAddress *paddr) {
  int res = SSLAdapter::RecvFrom(pv, cb, paddr);
  if (res > 0)
    LogMultiline(level_, label_.c_str(), true, pv, res, hex_mode_, &lms_);
  return res;
}

int LoggingSSLSocketAdapter::Close() {
  LogMultiline(level_, label_.c_str(), false, NULL, 0, hex_mode_, &lms_);
  LogMultiline(level_, label_.c_str(), true, NULL, 0, hex_mode_, &lms_);
  LOG_V(level_) << label_ << " Closed locally";
  return socket_->Close();
}

void LoggingSSLSocketAdapter::OnConnectEvent(SSLAdapter * socket) {
  LOG_V(level_) << label_ << " Connected";
  SSLAdapter::OnConnectEvent(socket);
}

void LoggingSSLSocketAdapter::OnCloseEvent(SSLAdapter * socket, int err) {
  LogMultiline(level_, label_.c_str(), false, NULL, 0, hex_mode_, &lms_);
  LogMultiline(level_, label_.c_str(), true, NULL, 0, hex_mode_, &lms_);
  LOG_V(level_) << label_ << " Closed with error: " << err;
  SSLAdapter::OnCloseEvent(socket, err);
}

int LoggingSSLSocketAdapter::StartSSL(const char* hostname, bool restartable) {
    return static_cast<SSLAdapter*>(socket_)->StartSSL(hostname, restartable);
}


}  // namespace rtc
