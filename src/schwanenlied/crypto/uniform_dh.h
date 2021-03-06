/**
 * @file    uniform_dh.h
 * @author  Yawning Angel (yawning at schwanenlied dot me)
 * @brief   UniformDH Key Exchange
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

#ifndef SCHWANENLIED_CRYPTO_UNIFORM_DH_H__
#define SCHWANENLIED_CRYPTO_UNIFORM_DH_H__

#include <string>

#include <openssl/dh.h>

#include "schwanenlied/common.h"
#include "schwanenlied/crypto/utils.h"

namespace schwanenlied {
namespace crypto {

/**
 * UniformDH key exchange
 *
 * This is a implementation of the UniformDH key exchange protocol as specified
 * in the obfs3 spec.  It uses OpenSSL's DH code for the modular exponentiation,
 * which is "constant time" for OpenSSL >= 0.9.7h.
 */
class UniformDH {
 public:
  /** The key length in bytes */
  static constexpr size_t kKeyLength = 1536 / 8;

  /**
   * Construct a UniformDH instance
   *
   * @warning Using a explicit private key should ONLY be used for testing.
   *
   * @param[in] priv_key    A pointer to the private key
   * @param[in] len         Length of the key (MUST be kKeyLength bytes)
   */
  UniformDH(const uint8_t* priv_key = nullptr,
            const size_t len = 0);

  ~UniformDH();

  /**
   * Given a peer's public key, calculate the shared secret
   *
   * Each instance should only be used once, and the code will destroy the
   * private key before this ever returns with true.
   *
   * @param[in] pub_key   A pointer to the peer's public key
   * @param[in] len       The length of the key (MUST be kKeyLength bytes)
   *
   * @returns true  - Success
   * @returns false - Failure
   */
  bool compute_key(const uint8_t* pub_key,
                   const size_t len);

  /** Obtain the public key belonging to this instance */
  const ::std::string public_key() const { return public_key_; }

  /** Obtain the shared secret derived in compute_key() */
  const SecureBuffer shared_secret() const {
    // Oh shit wtf do you think you're doing
    SL_ASSERT(has_shared_secret_ == true);
    return shared_secret_;
  }

 private:
  UniformDH(const UniformDH&) = delete;
  void operator=(const UniformDH&) = delete;

  DH* ctx_;                     /**< The DH context */
  ::std::string public_key_;    /**< The serialized form of the public key */
  bool has_shared_secret_;      /**< Is a valid shared secret present? */
  SecureBuffer shared_secret_;  /**< The shared secret */
};

} // namespace crypto
} // namespace schwanenlied

#endif // SCHWANENLIED_CRYPTO_UNIFORM_DH_H__
