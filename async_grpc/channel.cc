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

#include "async_grpc/channel.h"

namespace async_grpc {

Channel::Channel(
    const std::string& server_address, bool use_ssl,
    std::unique_ptr<auth::CredentialsProvider> credentials_provider)
    : channel_(::grpc::CreateChannel(
          server_address, use_ssl
                              ? ::grpc::SslCredentials(ssl_credentials_options_)
                              : ::grpc::InsecureChannelCredentials())),
      credentials_provider_(std::move(credentials_provider)) {}

std::shared_ptr<::grpc::Channel> Channel::GetGrpcChannel() { return channel_; }

}  // namespace async_grpc