/*
 * Copyright 2017 The Cartographer Authors
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

#ifndef CPP_GRPC_RPC_HANDLER_INTERFACE_H_
#define CPP_GRPC_RPC_HANDLER_INTERFACE_H_

#include "async_grpc/common/make_unique.h"
#include "async_grpc/execution_context.h"
#include "google/protobuf/message.h"
#include "grpc++/grpc++.h"

namespace async_grpc {

class Rpc;
class RpcHandlerInterface {
 public:
  virtual ~RpcHandlerInterface() = default;
  virtual void SetExecutionContext(ExecutionContext* execution_context) = 0;
  virtual void SetRpc(Rpc* rpc) = 0;
  virtual void Initialize(){};
  virtual void OnRequestInternal(
      const ::google::protobuf::Message* request) = 0;
  virtual void OnReadsDone(){};
  virtual void OnFinish(){};
  template <class RpcHandlerType>
  static std::unique_ptr<RpcHandlerType> Instantiate() {
    return common::make_unique<RpcHandlerType>();
  }
};

using RpcHandlerFactory = std::function<std::unique_ptr<RpcHandlerInterface>(
    Rpc*, ExecutionContext*)>;

struct RpcHandlerInfo {
  const google::protobuf::Descriptor* request_descriptor;
  const google::protobuf::Descriptor* response_descriptor;
  const RpcHandlerFactory rpc_handler_factory;
  const ::grpc::internal::RpcMethod::RpcType rpc_type;
  const std::string fully_qualified_name;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_RPC_HANDLER_INTERFACE_H_
