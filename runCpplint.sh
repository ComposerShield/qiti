#!/usr/bin/env bash

# Run cpplint with your flags
cpplint \
  --recursive \
  --verbose=2 \
  --quiet \
  --filter=-whitespace,-build/header_guard,-build/include_subdir,-build/include_order,-build/c++11,-build/c++17,-readability/todo,-runtime/references,-runtime/int \
  source
