#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
git submodule update --init --recursive
python3 scripts/verify_dependencies.py
