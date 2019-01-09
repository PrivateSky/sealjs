{
  "targets": [
    {
      "target_name": "nodeseal",
      "sources": [
        "addon.cc",

        "./seal/batchencoder.cpp",
        "./seal/biguint.cpp",
        "./seal/ciphertext.cpp",
        "./seal/ckks.cpp",
        "./seal/context.cpp",
        "./seal/decryptor.cpp",
        "./seal/encoder.cpp",
        "./seal/encryptionparams.cpp",
        "./seal/encryptor.cpp",
        "./seal/evaluator.cpp",
        "./seal/galoiskeys.cpp",
        "./seal/keygenerator.cpp",
        "./seal/memorymanager.cpp",
        "./seal/plaintext.cpp",
        "./seal/randomgen.cpp",
        "./seal/relinkeys.cpp",
        "./seal/smallmodulus.cpp",

        "./seal/util/aes.cpp",
        "./seal/util/baseconverter.cpp",
        "./seal/util/clipnormal.cpp",
        "./seal/util/globals.cpp",
        "./seal/util/hash.cpp",
        "./seal/util/mempool.cpp",
        "./seal/util/numth.cpp",
        "./seal/util/polyarith.cpp",
        "./seal/util/polyarithmod.cpp",
        "./seal/util/polyarithsmallmod.cpp",
        "./seal/util/smallntt.cpp",
        "./seal/util/uintarith.cpp",
        "./seal/util/uintarithmod.cpp",
        "./seal/util/uintarithsmallmod.cpp",
        "./seal/util/uintcore.cpp",

        "./demo/demo_seal.cc",
        "./demo/demo_sync.cc",

        "./src/base64.cpp",
        "./src/wrapper_globals.cc",
        "./src/homomorphic_context_wrapper.cc"

      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [
        '-fno-exceptions'
        , '-fno-rtti'
        , '-std=gnu++0x'
        ],

      'cflags_cc' : [ '-std=c++17' ],

      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")",
        "." ### "Additional Include Directories" for SEAL
        ],
      'defines': [
        "_DISABLE_EXTENDED_ALIGNED_STORAGE" ### _ENABLE_EXTENDED_ALIGNED_STORAGE or _DISABLE_EXTENDED_ALIGNED_STORAGE
        ###, "_CPPRTTI"
        ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'conditions': [
        ['OS=="win"', {

          'configurations': {
            'Debug': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'ExceptionHandling': 1,
                  'RuntimeTypeInfo': 'true' ### Set RTTI for each build configuration; not for 'OS=="win"' !!!
                },
              }
            },
            'Release': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'ExceptionHandling': 1,
                  'RuntimeTypeInfo': 'true'
                }
              }
            }
          }

        }],
        ['OS=="mac"', {
          "xcode_settings": {
            "CLANG_CXX_LIBRARY": "libc++",
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'MACOSX_DEPLOYMENT_TARGET': '10.7'
            ###, 'GCC_ENABLE_CPP_RTTI': 'YES' ###TODO: Please verify if this is necessary on Mac/Linux
          }
        }]
      ]
    }
  ]
}
