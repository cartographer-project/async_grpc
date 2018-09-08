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

#include "async_grpc/client.h"

#include "async_grpc/proto/math_service.pb.h"
#include "async_grpc/retry.h"
#include "glog/logging.h"
#include "grpc++/grpc++.h"
#include "gtest/gtest.h"

namespace async_grpc {
namespace {

struct GetEchoMethod {
  static constexpr const char* MethodName() {
    return "/async_grpc.proto.Math/GetEcho";
  }
  using IncomingType = proto::GetEchoRequest;
  using OutgoingType = proto::GetEchoResponse;
};

const char* kWrongAddress = "wrong-domain-does-not-exist:50051";

TEST(ClientTest, TimesOut) {
  auto client_channel = ::grpc::CreateChannel(
      kWrongAddress, ::grpc::InsecureChannelCredentials());
  Client<GetEchoMethod> client(client_channel, common::FromSeconds(0.1));
  proto::GetEchoRequest request;
  grpc::Status status;
  EXPECT_FALSE(client.Write(request, &status));
}

TEST(ClientTest, TimesOutWithRetries) {
  auto client_channel = ::grpc::CreateChannel(
      kWrongAddress, ::grpc::InsecureChannelCredentials());
  Client<GetEchoMethod> client(
      client_channel, common::FromSeconds(0.5),
      CreateLimitedBackoffStrategy(common::FromSeconds(0.1), 1, 3));
  proto::GetEchoRequest request;
  grpc::Status status;
  EXPECT_FALSE(client.Write(request, &status));
}

}  // namespace
}  // namespace async_grpc
