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

#ifndef ASYNC_GRPC_CHANNEL_H
#define ASYNC_GRPC_CHANNEL_H

#include <memory>

#include "async_grpc/auth/credentials_provider.h"
#include "grpc++/grpc++.h"

namespace async_grpc {

template <typename RpcServiceMethodConcept>
class Client;

class Channel {
 public:
  Channel(const std::string& server_address, bool use_ssl, std::unique_ptr<auth::CredentialsProvider> credentials_provider);
  std::shared_ptr<::grpc::Channel> GetGrpcChannel();

 private:
  ::grpc::SslCredentialsOptions ssl_credentials_options_;
  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<auth::CredentialsProvider> credentials_provider_;
};

}  // namespace async_grpc

#endif  // ASYNC_GRPC_CHANNEL_H
