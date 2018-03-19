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

FROM ubuntu:trusty

# Install debs for cmake build
COPY scripts/install_debs_cmake.sh scripts/
RUN scripts/install_debs_cmake.sh && rm -rf /var/lib/apt/lists/*

# Install proto3
COPY scripts/install_proto3.sh scripts/
RUN scripts/install_proto3.sh && rm -rf protobuf

# Install gRPC
COPY scripts/install_grpc.sh scripts/
RUN scripts/install_grpc.sh && rm -rf grpc

# Compile and install
COPY scripts/install_async_grpc_cmake.sh scripts/
COPY . async_grpc 
RUN scripts/install_async_grpc_cmake.sh && rm -rf async_grpc 
