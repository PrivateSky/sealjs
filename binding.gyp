{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "addon.cc",
        "example_bfv_basics_i.cc",
        "sync.cc",
        "./seal/*.cpp",
		"./seal/util/*.cpp"
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")",
        "."
        ],
      'defines': [
        "_DISABLE_EXTENDED_ALIGNED_STORAGE"
        #, "_CPPRTTI"
        ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'conditions': [
        ['OS=="win"', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
              #, "RuntimeTypeInfo": "true"
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
