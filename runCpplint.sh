#!/usr/bin/env bash

# Run cpplint with your flags
cpplint \
  --recursive \
  --verbose=2 \
  --quiet \
  --filter=-whitespace,-build/include_subdir,-build/c++11,-build/c++17,-readability/todo,-runtime/references \
  source
