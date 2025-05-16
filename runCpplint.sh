#!/usr/bin/env bash

# Run cpplint with your flags
cpplint \
  --recursive \
  --verbose=2 \
  --quiet \
  --filter=-whitespace,-build/header_guard,-build/include_subdir,-build/include_order,-build/c++11,-readability/todo,-runtime/references,-runtime/printf,-runtime/string,-runtime/int \
  source
