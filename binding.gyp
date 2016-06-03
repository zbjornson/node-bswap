{
  "targets": [
    {
      "target_name": "bswap",
      "sources": [ "src/bswap.cc" ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
