/*!
 * goo.c - groups of unknown order for C89
 * Copyright (c) 2018-2019, Christopher Jeffrey (MIT License).
 * https://github.com/handshake-org/goosig
 */

#ifndef _GOO_INTERNAL_H
#define _GOO_INTERNAL_H

#include <stdlib.h>

#ifdef GOO_HAS_GMP
#include <gmp.h>
#else
#include "mini-gmp.h"
#endif

#include "drbg.h"

#define GOO_DEFAULT_G 2
#define GOO_DEFAULT_H 3
#define GOO_MIN_RSA_BITS 1024
#define GOO_MAX_RSA_BITS 4096
#define GOO_EXP_BITS 2048
#define GOO_WINDOW_SIZE 6
#define GOO_MAX_COMB_SIZE 512
#define GOO_CHAL_BITS 128
#define GOO_ELL_BITS 136
#define GOO_ELLDIFF_MAX 512
#define GOO_TABLEN (1 << (GOO_WINDOW_SIZE - 2))

#define GOO_MIN_RSA_BYTES ((GOO_MIN_RSA_BITS + 7) / 8)
#define GOO_MAX_RSA_BYTES ((GOO_MAX_RSA_BITS + 7) / 8)
#define GOO_EXP_BYTES ((GOO_EXP_BITS + 7) / 8)
#define GOO_CHAL_BYTES ((GOO_CHAL_BITS + 7) / 8)
#define GOO_ELL_BYTES ((GOO_ELL_BITS + 7) / 8)

/* SHA256("Goo Signature")
 *
 * This, combined with the group hash of
 * SHA256(g || h || n), gives us an IV of:
 *
 *   0x4332417d
 *   0xf3a92851
 *   0xd59e8673
 *   0x6cbbfa97
 *   0xd44855ed
 *   0x385a0490
 *   0xa2297690
 *   0x0e0ea0e4
 */
static const unsigned char GOO_HASH_PREFIX[32] = {
  0xc8, 0x30, 0xd5, 0xfd, 0xdc, 0xb2, 0x23, 0xcd,
  0x86, 0x00, 0x7a, 0xbf, 0x91, 0xc4, 0x40, 0x27,
  0x6b, 0x00, 0x80, 0x66, 0xbc, 0xb6, 0x45, 0x91,
  0xef, 0x80, 0x61, 0xc8, 0x9c, 0x1c, 0x58, 0x82
};

/* SHA256("Goo Expand") */
static const unsigned char GOO_PRNG_EXPAND[32] = {
  0x21, 0xa2, 0x7e, 0xd5, 0xef, 0xc0, 0x95, 0x45,
  0x0b, 0x7b, 0x4d, 0xdb, 0x61, 0x30, 0x49, 0x1f,
  0x24, 0x17, 0xec, 0x25, 0x8e, 0xb2, 0xf4, 0xb7,
  0xb2, 0xa6, 0xa9, 0x36, 0xf7, 0xcf, 0xec, 0xfb
};

/* SHA256("Goo Derive") */
static const unsigned char GOO_PRNG_DERIVE[32] = {
  0x99, 0x89, 0x61, 0x8e, 0x45, 0x0e, 0x09, 0xfb,
  0xed, 0x0b, 0xc9, 0x51, 0xa3, 0xb3, 0x09, 0xa9,
  0xb5, 0xd2, 0xba, 0xe3, 0xdb, 0x76, 0x96, 0xb7,
  0x6a, 0x89, 0x42, 0x81, 0xe5, 0x65, 0x34, 0xaf
};

/* SHA256("Goo Primality") */
static const unsigned char GOO_PRNG_PRIMALITY[32] = {
  0xf3, 0x31, 0x84, 0xc5, 0x6d, 0x6c, 0xc4, 0xf6,
  0x0e, 0x39, 0x62, 0xa3, 0xad, 0xa4, 0xef, 0x03,
  0x97, 0xa6, 0xd6, 0x0f, 0x14, 0xc1, 0xc3, 0xa6,
  0xd8, 0xa1, 0xe6, 0x7e, 0xb4, 0x33, 0x48, 0x55
};

/* SHA256("Goo Sign") */
static const unsigned char GOO_PRNG_SIGN[32] = {
  0x22, 0xe6, 0x4a, 0x95, 0x3d, 0x87, 0x74, 0x2d,
  0x7c, 0xe6, 0xdd, 0x66, 0x3d, 0x4c, 0xea, 0xf3,
  0x55, 0xce, 0xa1, 0x74, 0x6a, 0xb8, 0x12, 0x20,
  0x66, 0x68, 0xa1, 0xb2, 0xf1, 0xe3, 0x2d, 0xb3
};

/* SHA256("Goo Encrypt") */
static const unsigned char GOO_PRNG_ENCRYPT[32] = {
  0xc5, 0xba, 0xf3, 0x82, 0xd5, 0xf1, 0xee, 0x45,
  0xbc, 0xab, 0xab, 0x07, 0xdb, 0xd8, 0xee, 0x7d,
  0x85, 0xed, 0x78, 0x68, 0x61, 0xd4, 0x21, 0xc7,
  0xc2, 0xfb, 0x55, 0x90, 0xf0, 0x85, 0x61, 0xb4
};

/* SHA256("Goo Decrypt") */
static const unsigned char GOO_PRNG_DECRYPT[32] = {
  0x19, 0x03, 0x6b, 0xc4, 0x38, 0xd5, 0x8c, 0x14,
  0x34, 0x5c, 0x41, 0x94, 0xc5, 0x24, 0x7f, 0xf9,
  0xcf, 0x27, 0xc7, 0xef, 0x47, 0xe6, 0xf4, 0xc3,
  0xf4, 0x1a, 0x01, 0xc7, 0x8d, 0x58, 0x3e, 0xe7
};

/* SHA256("Goo Local") */
static const unsigned char GOO_PRNG_LOCAL[32] = {
  0x21, 0x15, 0x7f, 0x0d, 0xbe, 0x3e, 0x90, 0x38,
  0xde, 0xa5, 0xd7, 0xdb, 0xf9, 0x28, 0x90, 0x01,
  0xe5, 0x5a, 0xa5, 0x75, 0xd2, 0xb3, 0x10, 0x67,
  0x5d, 0x34, 0x34, 0x51, 0x40, 0xad, 0x68, 0x8e
};

typedef struct goo_combspec_s {
  long points_per_add;
  long adds_per_shift;
  long shifts;
  long bits_per_window;
  long size;
} goo_combspec_t;

typedef struct goo_comb_s {
  long points_per_add;
  long adds_per_shift;
  long shifts;
  long bits_per_window;
  long bits;
  long points_per_subcomb;
  long size;
  mpz_t *items;
  long **wins;
} goo_comb_t;

typedef struct goo_comb_item_s {
  goo_comb_t g;
  goo_comb_t h;
} goo_comb_item_t;

typedef struct goo_prng_s {
  goo_drbg_t ctx;
  mpz_t save;
  unsigned long total;
  mpz_t tmp;
} goo_prng_t;

typedef struct goo_sig_s {
  mpz_t C2;
  mpz_t C3;
  mpz_t t;
  mpz_t chal;
  mpz_t ell;
  mpz_t Aq;
  mpz_t Bq;
  mpz_t Cq;
  mpz_t Dq;
  mpz_t Eq;
  mpz_t z_w;
  mpz_t z_w2;
  mpz_t z_s1;
  mpz_t z_a;
  mpz_t z_an;
  mpz_t z_s1w;
  mpz_t z_sa;
  mpz_t z_s2;
} goo_sig_t;

typedef struct goo_group_s {
  /* Group parameters */
  mpz_t n;
  mpz_t g;
  mpz_t h;
  mpz_t nh;
  size_t bits;
  size_t size;
  size_t rand_bits;

  /* PRNG */
  goo_prng_t prng;

  /* Cached SHA midstate */
  goo_sha256_t sha;

  /* WNAF */
  mpz_t table_p1[GOO_TABLEN];
  mpz_t table_n1[GOO_TABLEN];
  mpz_t table_n2[GOO_TABLEN];
  mpz_t table_p2[GOO_TABLEN];
  long wnaf0[GOO_MAX_RSA_BITS + 1];
  long wnaf1[GOO_ELL_BITS + 1];
  long wnaf2[GOO_ELL_BITS + 1];

  /* Combs */
  long combs_len;
  goo_comb_item_t combs[2];

  /* Used for goo_group_hash() */
  unsigned char slab[GOO_MAX_RSA_BYTES];
} goo_group_t;

/**
 * Moduli of unknown factorization.
 *
 * See:
 *
 *   https://en.wikipedia.org/wiki/RSA_Factoring_Challenge
 *   https://en.wikipedia.org/wiki/RSA_numbers
 *   https://en.wikipedia.org/wiki/RSA_numbers#RSA-617
 *   https://en.wikipedia.org/wiki/RSA_numbers#RSA-2048
 *   https://ssl-tools.net/subjects/3c8008731e5ff9a0e7a6b0fb906fc6e439cbe862
 *   https://ssl-tools.net/subjects/28ecf0993d30f9e4e607bef4f5c487f64a2a71a6
 *   https://web.archive.org/web/20130507091636/http://www.rsa.com/rsalabs/node.asp?id=2092
 *   https://web.archive.org/web/20130507115513/http://www.rsa.com/rsalabs/node.asp?id=2093
 *   https://web.archive.org/web/20130507115513/http://www.rsa.com/rsalabs/challenges/factoring/challengenumbers.txt
 *   https://web.archive.org/web/20130502202924/http://www.rsa.com/rsalabs/node.asp?id=2094
 *   http://www.ontko.com/pub/rayo/primes/rsa_fact.html
 */

/* America Online Root CA 1 (2048)
 *
 * BLAKE2b-256: de2fcb98f153761deabe5ba0187418f19f3129f2204d34ae6ae93f6578fce139
 * SHA-256: 7bd082427ff18b35c8e2cdb2b848d9c139877a273663d1eeea1d2a3d72b01140
 * SHA-3: 978c1dc3a011927c7ce28a2747344f8f643e4d6a8cb263ca1a7ae786794f4d0a
 * Digit Sum: 2776
 * Checksum: 940443
 */

const unsigned char GOO_AOL1[256] = {
  0xa8, 0x2f, 0xe8, 0xa4, 0x69, 0x06, 0x03, 0x47,
  0xc3, 0xe9, 0x2a, 0x98, 0xff, 0x19, 0xa2, 0x70,
  0x9a, 0xc6, 0x50, 0xb2, 0x7e, 0xa5, 0xdf, 0x68,
  0x4d, 0x1b, 0x7c, 0x0f, 0xb6, 0x97, 0x68, 0x7d,
  0x2d, 0xa6, 0x8b, 0x97, 0xe9, 0x64, 0x86, 0xc9,
  0xa3, 0xef, 0xa0, 0x86, 0xbf, 0x60, 0x65, 0x9c,
  0x4b, 0x54, 0x88, 0xc2, 0x48, 0xc5, 0x4a, 0x39,
  0xbf, 0x14, 0xe3, 0x59, 0x55, 0xe5, 0x19, 0xb4,
  0x74, 0xc8, 0xb4, 0x05, 0x39, 0x5c, 0x16, 0xa5,
  0xe2, 0x95, 0x05, 0xe0, 0x12, 0xae, 0x59, 0x8b,
  0xa2, 0x33, 0x68, 0x58, 0x1c, 0xa6, 0xd4, 0x15,
  0xb7, 0xd8, 0x9f, 0xd7, 0xdc, 0x71, 0xab, 0x7e,
  0x9a, 0xbf, 0x9b, 0x8e, 0x33, 0x0f, 0x22, 0xfd,
  0x1f, 0x2e, 0xe7, 0x07, 0x36, 0xef, 0x62, 0x39,
  0xc5, 0xdd, 0xcb, 0xba, 0x25, 0x14, 0x23, 0xde,
  0x0c, 0xc6, 0x3d, 0x3c, 0xce, 0x82, 0x08, 0xe6,
  0x66, 0x3e, 0xda, 0x51, 0x3b, 0x16, 0x3a, 0xa3,
  0x05, 0x7f, 0xa0, 0xdc, 0x87, 0xd5, 0x9c, 0xfc,
  0x72, 0xa9, 0xa0, 0x7d, 0x78, 0xe4, 0xb7, 0x31,
  0x55, 0x1e, 0x65, 0xbb, 0xd4, 0x61, 0xb0, 0x21,
  0x60, 0xed, 0x10, 0x32, 0x72, 0xc5, 0x92, 0x25,
  0x1e, 0xf8, 0x90, 0x4a, 0x18, 0x78, 0x47, 0xdf,
  0x7e, 0x30, 0x37, 0x3e, 0x50, 0x1b, 0xdb, 0x1c,
  0xd3, 0x6b, 0x9a, 0x86, 0x53, 0x07, 0xb0, 0xef,
  0xac, 0x06, 0x78, 0xf8, 0x84, 0x99, 0xfe, 0x21,
  0x8d, 0x4c, 0x80, 0xb6, 0x0c, 0x82, 0xf6, 0x66,
  0x70, 0x79, 0x1a, 0xd3, 0x4f, 0xa3, 0xcf, 0xf1,
  0xcf, 0x46, 0xb0, 0x4b, 0x0f, 0x3e, 0xdd, 0x88,
  0x62, 0xb8, 0x8c, 0xa9, 0x09, 0x28, 0x3b, 0x7a,
  0xc7, 0x97, 0xe1, 0x1e, 0xe5, 0xf4, 0x9f, 0xc0,
  0xc0, 0xae, 0x24, 0xa0, 0xc8, 0xa1, 0xd9, 0x0f,
  0xd6, 0x7b, 0x26, 0x82, 0x69, 0x32, 0x3d, 0xa7
};

/* America Online Root CA 2 (4096)
 *
 * BLAKE2b-256: ff88e50b2bcd0486d537347617b626f802be32ad75872abbc8e4b68af208dc4b
 * SHA-256: 1c25b6b4a8e3b684dc828c0922b2359399c36b93f3ebaa6b71a1e412555545ba
 * SHA-3: 825230318e29939794b73cdf200a650c6fc144db7d60ec0c35e380d68e627798
 * Digit Sum: 5522
 * Checksum: 915896
 */

const unsigned char GOO_AOL2[512] = {
  0xcc, 0x41, 0x45, 0x1d, 0xe9, 0x3d, 0x4d, 0x10,
  0xf6, 0x8c, 0xb1, 0x41, 0xc9, 0xe0, 0x5e, 0xcb,
  0x0d, 0xb7, 0xbf, 0x47, 0x73, 0xd3, 0xf0, 0x55,
  0x4d, 0xdd, 0xc6, 0x0c, 0xfa, 0xb1, 0x66, 0x05,
  0x6a, 0xcd, 0x78, 0xb4, 0xdc, 0x02, 0xdb, 0x4e,
  0x81, 0xf3, 0xd7, 0xa7, 0x7c, 0x71, 0xbc, 0x75,
  0x63, 0xa0, 0x5d, 0xe3, 0x07, 0x0c, 0x48, 0xec,
  0x25, 0xc4, 0x03, 0x20, 0xf4, 0xff, 0x0e, 0x3b,
  0x12, 0xff, 0x9b, 0x8d, 0xe1, 0xc6, 0xd5, 0x1b,
  0xb4, 0x6d, 0x22, 0xe3, 0xb1, 0xdb, 0x7f, 0x21,
  0x64, 0xaf, 0x86, 0xbc, 0x57, 0x22, 0x2a, 0xd6,
  0x47, 0x81, 0x57, 0x44, 0x82, 0x56, 0x53, 0xbd,
  0x86, 0x14, 0x01, 0x0b, 0xfc, 0x7f, 0x74, 0xa4,
  0x5a, 0xae, 0xf1, 0xba, 0x11, 0xb5, 0x9b, 0x58,
  0x5a, 0x80, 0xb4, 0x37, 0x78, 0x09, 0x33, 0x7c,
  0x32, 0x47, 0x03, 0x5c, 0xc4, 0xa5, 0x83, 0x48,
  0xf4, 0x57, 0x56, 0x6e, 0x81, 0x36, 0x27, 0x18,
  0x4f, 0xec, 0x9b, 0x28, 0xc2, 0xd4, 0xb4, 0xd7,
  0x7c, 0x0c, 0x3e, 0x0c, 0x2b, 0xdf, 0xca, 0x04,
  0xd7, 0xc6, 0x8e, 0xea, 0x58, 0x4e, 0xa8, 0xa4,
  0xa5, 0x18, 0x1c, 0x6c, 0x45, 0x98, 0xa3, 0x41,
  0xd1, 0x2d, 0xd2, 0xc7, 0x6d, 0x8d, 0x19, 0xf1,
  0xad, 0x79, 0xb7, 0x81, 0x3f, 0xbd, 0x06, 0x82,
  0x27, 0x2d, 0x10, 0x58, 0x05, 0xb5, 0x78, 0x05,
  0xb9, 0x2f, 0xdb, 0x0c, 0x6b, 0x90, 0x90, 0x7e,
  0x14, 0x59, 0x38, 0xbb, 0x94, 0x24, 0x13, 0xe5,
  0xd1, 0x9d, 0x14, 0xdf, 0xd3, 0x82, 0x4d, 0x46,
  0xf0, 0x80, 0x39, 0x52, 0x32, 0x0f, 0xe3, 0x84,
  0xb2, 0x7a, 0x43, 0xf2, 0x5e, 0xde, 0x5f, 0x3f,
  0x1d, 0xdd, 0xe3, 0xb2, 0x1b, 0xa0, 0xa1, 0x2a,
  0x23, 0x03, 0x6e, 0x2e, 0x01, 0x15, 0x87, 0x5c,
  0xa6, 0x75, 0x75, 0xc7, 0x97, 0x61, 0xbe, 0xde,
  0x86, 0xdc, 0xd4, 0x48, 0xdb, 0xbd, 0x2a, 0xbf,
  0x4a, 0x55, 0xda, 0xe8, 0x7d, 0x50, 0xfb, 0xb4,
  0x80, 0x17, 0xb8, 0x94, 0xbf, 0x01, 0x3d, 0xea,
  0xda, 0xba, 0x7c, 0xe0, 0x58, 0x67, 0x17, 0xb9,
  0x58, 0xe0, 0x88, 0x86, 0x46, 0x67, 0x6c, 0x9d,
  0x10, 0x47, 0x58, 0x32, 0xd0, 0x35, 0x7c, 0x79,
  0x2a, 0x90, 0xa2, 0x5a, 0x10, 0x11, 0x23, 0x35,
  0xad, 0x2f, 0xcc, 0xe4, 0x4a, 0x5b, 0xa7, 0xc8,
  0x27, 0xf2, 0x83, 0xde, 0x5e, 0xbb, 0x5e, 0x77,
  0xe7, 0xe8, 0xa5, 0x6e, 0x63, 0xc2, 0x0d, 0x5d,
  0x61, 0xd0, 0x8c, 0xd2, 0x6c, 0x5a, 0x21, 0x0e,
  0xca, 0x28, 0xa3, 0xce, 0x2a, 0xe9, 0x95, 0xc7,
  0x48, 0xcf, 0x96, 0x6f, 0x1d, 0x92, 0x25, 0xc8,
  0xc6, 0xc6, 0xc1, 0xc1, 0x0c, 0x05, 0xac, 0x26,
  0xc4, 0xd2, 0x75, 0xd2, 0xe1, 0x2a, 0x67, 0xc0,
  0x3d, 0x5b, 0xa5, 0x9a, 0xeb, 0xcf, 0x7b, 0x1a,
  0xa8, 0x9d, 0x14, 0x45, 0xe5, 0x0f, 0xa0, 0x9a,
  0x65, 0xde, 0x2f, 0x28, 0xbd, 0xce, 0x6f, 0x94,
  0x66, 0x83, 0x48, 0x29, 0xd8, 0xea, 0x65, 0x8c,
  0xaf, 0x93, 0xd9, 0x64, 0x9f, 0x55, 0x57, 0x26,
  0xbf, 0x6f, 0xcb, 0x37, 0x31, 0x99, 0xa3, 0x60,
  0xbb, 0x1c, 0xad, 0x89, 0x34, 0x32, 0x62, 0xb8,
  0x43, 0x21, 0x06, 0x72, 0x0c, 0xa1, 0x5c, 0x6d,
  0x46, 0xc5, 0xfa, 0x29, 0xcf, 0x30, 0xde, 0x89,
  0xdc, 0x71, 0x5b, 0xdd, 0xb6, 0x37, 0x3e, 0xdf,
  0x50, 0xf5, 0xb8, 0x07, 0x25, 0x26, 0xe5, 0xbc,
  0xb5, 0xfe, 0x3c, 0x02, 0xb3, 0xb7, 0xf8, 0xbe,
  0x43, 0xc1, 0x87, 0x11, 0x94, 0x9e, 0x23, 0x6c,
  0x17, 0x8a, 0xb8, 0x8a, 0x27, 0x0c, 0x54, 0x47,
  0xf0, 0xa9, 0xb3, 0xc0, 0x80, 0x8c, 0xa0, 0x27,
  0xeb, 0x1d, 0x19, 0xe3, 0x07, 0x8e, 0x77, 0x70,
  0xca, 0x2b, 0xf4, 0x7d, 0x76, 0xe0, 0x78, 0x67
};

/* RSA-2048 Factoring Challenge (2048)
 *
 * BLAKE2b-256: 6bd6195870dc2627bcce76c24d248d0b4f9fa69c8e0b5f4fe1a55307546e3fd7
 * SHA-256: 6ae9d033c1d76c4f535b5ad5c0073933a0b375b4120a75fbb66be814eab1a9ce
 * SHA-3: 27cd119bc094ae4caa250860ceeb294056f25fd613c4c3642765148821a2b754
 * Digit Sum: 2738
 * Checksum: 543967
 */

const unsigned char GOO_RSA2048[256] = {
  0xc7, 0x97, 0x0c, 0xee, 0xdc, 0xc3, 0xb0, 0x75,
  0x44, 0x90, 0x20, 0x1a, 0x7a, 0xa6, 0x13, 0xcd,
  0x73, 0x91, 0x10, 0x81, 0xc7, 0x90, 0xf5, 0xf1,
  0xa8, 0x72, 0x6f, 0x46, 0x35, 0x50, 0xbb, 0x5b,
  0x7f, 0xf0, 0xdb, 0x8e, 0x1e, 0xa1, 0x18, 0x9e,
  0xc7, 0x2f, 0x93, 0xd1, 0x65, 0x00, 0x11, 0xbd,
  0x72, 0x1a, 0xee, 0xac, 0xc2, 0xac, 0xde, 0x32,
  0xa0, 0x41, 0x07, 0xf0, 0x64, 0x8c, 0x28, 0x13,
  0xa3, 0x1f, 0x5b, 0x0b, 0x77, 0x65, 0xff, 0x8b,
  0x44, 0xb4, 0xb6, 0xff, 0xc9, 0x33, 0x84, 0xb6,
  0x46, 0xeb, 0x09, 0xc7, 0xcf, 0x5e, 0x85, 0x92,
  0xd4, 0x0e, 0xa3, 0x3c, 0x80, 0x03, 0x9f, 0x35,
  0xb4, 0xf1, 0x4a, 0x04, 0xb5, 0x1f, 0x7b, 0xfd,
  0x78, 0x1b, 0xe4, 0xd1, 0x67, 0x31, 0x64, 0xba,
  0x8e, 0xb9, 0x91, 0xc2, 0xc4, 0xd7, 0x30, 0xbb,
  0xbe, 0x35, 0xf5, 0x92, 0xbd, 0xef, 0x52, 0x4a,
  0xf7, 0xe8, 0xda, 0xef, 0xd2, 0x6c, 0x66, 0xfc,
  0x02, 0xc4, 0x79, 0xaf, 0x89, 0xd6, 0x4d, 0x37,
  0x3f, 0x44, 0x27, 0x09, 0x43, 0x9d, 0xe6, 0x6c,
  0xeb, 0x95, 0x5f, 0x3e, 0xa3, 0x7d, 0x51, 0x59,
  0xf6, 0x13, 0x58, 0x09, 0xf8, 0x53, 0x34, 0xb5,
  0xcb, 0x18, 0x13, 0xad, 0xdc, 0x80, 0xcd, 0x05,
  0x60, 0x9f, 0x10, 0xac, 0x6a, 0x95, 0xad, 0x65,
  0x87, 0x2c, 0x90, 0x95, 0x25, 0xbd, 0xad, 0x32,
  0xbc, 0x72, 0x95, 0x92, 0x64, 0x29, 0x20, 0xf2,
  0x4c, 0x61, 0xdc, 0x5b, 0x3c, 0x3b, 0x79, 0x23,
  0xe5, 0x6b, 0x16, 0xa4, 0xd9, 0xd3, 0x73, 0xd8,
  0x72, 0x1f, 0x24, 0xa3, 0xfc, 0x0f, 0x1b, 0x31,
  0x31, 0xf5, 0x56, 0x15, 0x17, 0x28, 0x66, 0xbc,
  0xcc, 0x30, 0xf9, 0x50, 0x54, 0xc8, 0x24, 0xe7,
  0x33, 0xa5, 0xeb, 0x68, 0x17, 0xf7, 0xbc, 0x16,
  0x39, 0x9d, 0x48, 0xc6, 0x36, 0x1c, 0xc7, 0xe5
};

/* RSA-617 Factoring Challenge (2048)
 *
 * BLAKE2b-256: 87bccdf2b1261c9c237671787ceb02dcaa305b1c64064db1b23b36ca3deec065
 * SHA-256: 8a090bf2cdbf9fac321b2ffb48b75d4d196118fc27d7430dca10c1d06085d448
 * SHA-3: 108e6ee888ad418df6f074b782a80e32f05e67fa54a17c879a160cd3761177e5
 * Digit Sum: 2680
 * Checksum: 909408
 */

const unsigned char GOO_RSA617[256] = {
  0xb3, 0xd5, 0x39, 0x5c, 0x45, 0xb5, 0x6d, 0x1c,
  0xfc, 0xf4, 0x11, 0xff, 0x0f, 0x6d, 0xa6, 0xb9,
  0xae, 0x45, 0xb1, 0xb0, 0x6b, 0xcf, 0xab, 0x61,
  0x88, 0x0a, 0x91, 0x1b, 0x22, 0xcf, 0xb2, 0x8c,
  0x3e, 0x01, 0x1e, 0x6a, 0x07, 0xc7, 0xec, 0x34,
  0x5f, 0x67, 0x48, 0x66, 0x87, 0xb9, 0x58, 0x1c,
  0x47, 0x5b, 0x9d, 0xa0, 0x8c, 0xec, 0xad, 0x9b,
  0xef, 0x00, 0x43, 0x15, 0xed, 0x3b, 0x01, 0x20,
  0xc8, 0x8e, 0x31, 0xc1, 0x34, 0xaa, 0x68, 0x48,
  0xa7, 0x0e, 0x87, 0x9c, 0x38, 0xaf, 0x31, 0x53,
  0x9b, 0xc0, 0x65, 0xd9, 0x14, 0x32, 0x72, 0x94,
  0x13, 0xde, 0xeb, 0x47, 0x5e, 0x03, 0x30, 0x49,
  0xcd, 0x28, 0x36, 0x48, 0xbf, 0xf4, 0x86, 0x76,
  0xbb, 0x2d, 0x33, 0x6e, 0x5a, 0xbf, 0xe0, 0xa5,
  0xa6, 0xb4, 0x6e, 0x89, 0x34, 0xd7, 0x11, 0xa6,
  0x85, 0xc4, 0xc4, 0x2b, 0x1b, 0x9a, 0xc4, 0x22,
  0xee, 0xa8, 0xb6, 0x4a, 0x81, 0xaf, 0xc4, 0xe2,
  0x9a, 0x72, 0x6f, 0x53, 0xca, 0x56, 0x13, 0xcb,
  0x44, 0xc8, 0xc6, 0x66, 0x0e, 0x36, 0xb8, 0x85,
  0x2e, 0xc1, 0xe0, 0x90, 0xdd, 0x62, 0x96, 0x45,
  0x7b, 0x15, 0xb1, 0x64, 0xd1, 0xf2, 0xf7, 0xa5,
  0x1c, 0x00, 0x37, 0x36, 0xcc, 0x5d, 0x89, 0x02,
  0x05, 0x9a, 0x7b, 0xcb, 0xea, 0xf1, 0xc5, 0xa0,
  0xf0, 0xea, 0xe6, 0x31, 0x9a, 0xd7, 0xa1, 0x44,
  0x5b, 0x1d, 0xf1, 0xfc, 0x79, 0xd1, 0xfa, 0x26,
  0x33, 0x02, 0x86, 0x98, 0x00, 0xce, 0x7b, 0xf8,
  0xb8, 0xae, 0x34, 0x0c, 0x01, 0x53, 0xa5, 0x14,
  0xde, 0xf6, 0x58, 0xf6, 0x19, 0x5f, 0x36, 0x61,
  0x64, 0x46, 0x69, 0xdf, 0x0b, 0x95, 0x14, 0xe6,
  0xe1, 0x34, 0x4d, 0xfa, 0x5b, 0x22, 0x10, 0x45,
  0x1a, 0xb5, 0xe9, 0x83, 0x87, 0x38, 0xbd, 0x15,
  0xed, 0x4a, 0x7a, 0xa7, 0x9e, 0x96, 0xb7, 0x65
};

#endif
