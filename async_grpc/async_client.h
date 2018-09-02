/*
 * Copyright 2018 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CPP_GRPC_ASYNC_CLIENT_H
#define CPP_GRPC_ASYNC_CLIENT_H

#include "async_grpc/rpc_service_method_traits.h"

#include "grpc++/grpc++.h"
#include "grpc++/impl/codegen/client_unary_call.h"
#include "grpc++/impl/codegen/proto_utils.h"
#include "grpc++/impl/codegen/sync_stream.h"

namespace async_grpc {

template <typename RpcServiceMethodConcept,
          ::grpc::internal::RpcMethod::RpcType StreamType =
              RpcServiceMethodTraits<RpcServiceMethodConcept>::StreamType>
class AsyncClient {};

template <typename RpcServiceMethodConcept, ::grpc::internal::RpcMethod::NORMAL_RPC>
class AsyncClient {
 public:
  AsyncClient(std::shared_ptr<::grpc::Channel> channel) : channel_(channel) {}

 private:
  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::grpc::ClientContext> client_context_;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_ASYNC_CLIENT_H
