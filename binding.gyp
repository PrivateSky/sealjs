{
  "targets": [
    {
      "target_name": "nodeseal",
      "sources": [
        "addon.cc",
        "./seal/*.cpp",
        "./seal/util/*.cpp",
        "./demo/*.cc",
        "./src/base64.cpp",
        "./src/*.cc"
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
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
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
              ###, "RuntimeTypeInfo": "true"
            }
          }
        }],
        ['OS=="mac"', {
          "xcode_settings": {
            "CLANG_CXX_LIBRARY": "libc++",
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'MACOSX_DEPLOYMENT_TARGET': '10.7'
          }
        }]
      ]
    }
  ]
}
