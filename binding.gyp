{
  "targets": [{
    "target_name": "u64",
    "sources": ["main.cc"],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ]
  }]
}
