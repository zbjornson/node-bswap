{
  "targets": [
    {
      "target_name": "bswap",
      "sources": [ "src/bswap.cc" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "cflags":[
        "-falign-loops=32", # See readme; significant improvement for some cases
        "-Wno-unused-function", # CPU feature detection only used on Win
        "-Wno-unused-const-variable", # cpuid regs
        "-Wno-cast-function-type" # https://github.com/nodejs/nan/issues/807
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "EnableEnhancedInstructionSet": 3 # /arch:AVX
          # 0-not set, 1-sse, 2-sse2, 3-avx, 4-ia32, 5-avx2
        }
      },
      "xcode_settings": {
        "OTHER_CPLUSPLUSFLAGS": [
          "-Wno-unused-function", # CPU feature detection only used on Win
          "-Wno-unused-const-variable"
        ]
      },
      "conditions": [
        ['target_arch != "arm64"', {
          "cflags"" [
            "-march=native"
          ],
          "xcode_settings" : {
            "OTHER_CPLUSPLUSFLAGS": [
              "-march=native"
            ]
          }
        }]
      ]
    }
  ]
}
