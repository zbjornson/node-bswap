{
  "targets": [
    {
      "target_name": "bswap",
      "sources": [ "src/bswap.cc" ],
      "cflags":[
        "-fvisibility=hidden",
        "-falign-loops=32", # See readme; significant improvement for some cases
        "-Wno-unused-function", # CPU feature detection only used on Win
        "-Wno-unused-const-variable" # cpuid regs
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "EnableEnhancedInstructionSet": 3 # /arch:AVX
          # 0-not set, 1-sse, 2-sse2, 3-avx, 4-ia32, 5-avx2
        }
      },
      "xcode_settings": {
        "OTHER_CPLUSPLUSFLAGS": [
          "-fvisibility=hidden",
          "-Wno-unused-function", # CPU feature detection only used on Win
          "-Wno-unused-const-variable"
        ],
        "xcode_settings": {
          "GCC_SYMBOLS_PRIVATE_EXTERN": "YES", # -fvisibility=hidden
        }
      },
      "conditions": [
        ['target_arch != "arm64"', {
          "cflags": [
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
