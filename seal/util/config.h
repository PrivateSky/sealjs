// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#define SEAL_VERSION "3.1.0"

/// After building Microsoft SEAL library: defined on Windows10; undefined on Ubuntu ==> common code: defined.
#define SEAL_DEBUG

#define SEAL_USE_IF_CONSTEXPR

#define SEAL_USE_MAYBE_UNUSED

#define SEAL_USE_STD_BYTE

#define SEAL_USE_SHARED_MUTEX

/* #undef SEAL_ENFORCE_HE_STD_SECURITY */

///////////// #define SEAL_USE_INTRIN

/** After building Microsoft SEAL library: defined on Windows10; undefined on Ubuntu ==> common code: undefined.
 * #undef SEAL_USE__UMUL128
*/

/** After building Microsoft SEAL library: defined on Windows10; undefined on Ubuntu ==> common code: undefined.
 * #undef SEAL_USE__BITSCANREVERSE64
*/

/* #undef SEAL_USE___BUILTIN_CLZLL */

/** After building Microsoft SEAL library: undefined on Windows10; defined on Ubuntu ==> common code: undefined.
 * #undef SEAL_USE___INT128
*/

#define SEAL_USE__ADDCARRY_U64

#define SEAL_USE__SUBBORROW_U64

/** After building Microsoft SEAL library: defined on both Windows10 and Ubuntu;
 *  !!! But node-gyp (g++ 7.3) throws some compile errors on Ubuntu, not fixed yet ==> common code: undefined.
 * #undef SEAL_USE_AES_NI_PRNG
*/

/* #undef SEAL_USE_MSGSL */

/* #undef SEAL_USE_MSGSL_SPAN */

/* #undef SEAL_USE_MSGSL_MULTISPAN */
