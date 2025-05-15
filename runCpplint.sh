#!/usr/bin/env bash

# Run cpplint with your flags
cpplint \
  --recursive \
  --verbose=2 \
  --quiet \
  --filter=-whitespace,-legal,-readability/casting,-build/include_what_you_use,-build/header_guard,-build/include_subdir,-build/include_order,-build/c++11,-readability/todo,-runtime/references,-runtime/printf,-runtime/string,-runtime/int \
  source
