/**
 * @file    obfs3/client.h
 * @author  Yawning Angel (yawning at schwanenlied dot me)
 * @brief   obfs3 (The Threebfuscator) Client
 */

/*
 * Copyright (c) 2014, Yawning Angel <yawning at schwanenlied dot me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SCHWANENLIED_PT_OBFS3_CLIENT_H__
#define SCHWANENLIED_PT_OBFS3_CLIENT_H__

#define OBFS3_LOGGER "obfs3"
#ifdef OBFS3_CLIENT_IMPL
#define _LOGGER OBFS3_LOGGER
#endif

#include <random>

#include "schwanenlied/common.h"
#include "schwanenlied/socks5_server.h"
#include "schwanenlied/crypto/aes.h"
#include "schwanenlied/crypto/hmac_sha256.h"
#include "schwanenlied/crypto/rand_openssl.h"
#include "schwanenlied/crypto/uniform_dh.h"

namespace schwanenlied {
namespace pt {

/** obfs3 (The Threebfuscator) */
namespace obfs3 {

/**
 * obfs3 (The Threebfuscator) Client
 *
 * This implements a wire compatible obfs3 client using Socks5Server.
 *
 * @todo Investigate using evbuffer_peek()/evbuffer_add_buffer() for better
 * performance (current code is more obviously correct).
 */
class Client : public Socks5Server::Session {
 public:
  /** Client factory */
  class SessionFactory : public Socks5Server::SessionFactory {
   public:
    Socks5Server::Session* create_session(Socks5Server& server,
                                          struct event_base* base,
                                          const evutil_socket_t sock,
                                          const ::std::string& addr,
                                          const bool scrub_addrs) override {
      return static_cast<Socks5Server::Session*>(new Client(server, base, sock,
                                                            addr, scrub_addrs));
    }
  };

  Client(Socks5Server& server,
         struct event_base* base,
         const evutil_socket_t sock,
         const ::std::string& addr,
         const bool scrub_addrs) :
      Session(server, base, sock, addr, false, scrub_addrs),
      logger_(::el::Loggers::getLogger(OBFS3_LOGGER)),
      sent_magic_(false),
      received_magic_(false),
      initiator_magic_(crypto::HmacSha256::kDigestLength, 0),
      responder_magic_(crypto::HmacSha256::kDigestLength, 0),
      pad_dist_(0, kMaxPadding / 2) {}

  ~Client() = default;

 protected:
  bool on_outgoing_connected() override;

  bool on_incoming_data() override;

  bool on_outgoing_data_connecting() override;

  bool on_outgoing_data() override;

 private:
  Client(const Client&) = delete;
  void operator=(const Client&) = delete;

  static constexpr uint16_t kMaxPadding = 8194; /** obfs3 MAX_PADDING */

  /** @{ */
  /**
   * Given a shared secret, derive the AES-CTR-128 keys per the obfs3 spec
   *
   * @param[in] shared_secret The shared secret to use as the key material
   *
   * @returns true  - Success
   * @returns false - Failure
   */
  bool kdf_obfs3(const crypto::SecureBuffer& shared_secret);
  /** @} */

  /** @{ */
  ::el::Logger* logger_;    /**< The obfs3 session logger_ */
  /** @} */

  /** @{ */
  crypto::UniformDH uniform_dh_;    /**< The UniformDH keypair */
  crypto::Aes128Ctr initiator_aes_; /**< E(INIT_KEY, DATA) */
  crypto::Aes128Ctr responder_aes_; /**< E(RESP_KEY, DATA) */
  crypto::RandOpenSSL rand_;        /**< CSPRNG */
  /** @} */

  /** @{ */
  bool sent_magic_;     /**< Sent initator_magic_ to the peer? */
  bool received_magic_; /**< Received responder_magic_ from the peer? */
  crypto::SecureBuffer initiator_magic_; /**< HMAC(SHARED_SECRET, "Initiator magic") */
  crypto::SecureBuffer responder_magic_; /**< HMAC(SHARED_SECRET, "Responder magic") */
  ::std::uniform_int_distribution<uint32_t> pad_dist_;  /** Padding distribution */
  /** @} */
};

} // namespace obfs3
} // namespace pt
} // namespace schwanenlied

#endif // SCHWANENLIED_PT_OBFS3_CLIENT_H__
