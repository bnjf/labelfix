#!/bin/bash

# vim:ts=2 sts=2 sw=2 et ai fdm=marker:

set -eu

cc \
  -Ionnv/usr/src/uts/common/fs/zfs/ -Ionnv/usr/src/uts/common/ \
  labelfix.c \
  onnv/usr/src/uts/common/fs/zfs/{fletcher,sha256,zio_checksum}.c \
  -lnvpair \
  -o labelfix
