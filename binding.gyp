{
  "targets": [{
    "target_name": "u64",
    "sources": ["main.cc","uint64.cc","u64str.c"],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ]
  }]
}
