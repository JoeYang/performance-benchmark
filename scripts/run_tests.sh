#!/bin/bash
set -e

echo "Running all tests..."
bazel test //...

echo ""
echo "All tests passed!"
