#!/bin/sh

# Copyright 2018 The Cartographer Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -o errexit
set -o verbose

VERSION="v1.14.2"
# Digest: d3e16042d57439bf554649d110dbc61efa1d5cf7

# Build and install gRPC.
git clone --branch ${VERSION} --depth 1 https://github.com/grpc/grpc
cd grpc
git submodule update --init
make
sudo make install
