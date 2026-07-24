#!/bin/bash
set -e
export VITASDK=/usr/local/vitasdk
export PATH="$VITASDK/bin:$PATH"

vdpm -f zlib
vdpm -f libpng
