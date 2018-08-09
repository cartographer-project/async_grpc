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

#include "async_grpc/server.h"

#include <future>

#include "async_grpc/client.h"
#include "async_grpc/execution_context.h"
#include "async_grpc/proto/math_service.pb.h"
#include "async_grpc/retry.h"
#include "async_grpc/rpc_handler.h"
#include "glog/logging.h"
#include "google/protobuf/descriptor.h"
#include "grpc++/grpc++.h"
#include "gtest/gtest.h"

namespace async_grpc {
namespace {

using EchoResponder = std::function<bool()>;

class MathServerContext : public ExecutionContext {
 public:
  int additional_increment() { return 10; }
  std::promise<EchoResponder> echo_responder;
};

struct GetSumMethod {
  static constexpr const char* MethodName() {
    return "/async_grpc.proto.Math/GetSum";
  }
  using IncomingType = Stream<proto::GetSumRequest>;
  using OutgoingType = proto::GetSumResponse;
};

class GetSumHandler : public RpcHandler<GetSumMethod> {
 public:
  void OnRequest(const proto::GetSumRequest& request) override {
    sum_ += GetContext<MathServerContext>()->additional_increment();
    sum_ += request.input();
  }

  void OnReadsDone() override {
    auto response = common::make_unique<proto::GetSumResponse>();
    response->set_output(sum_);
    Send(std::move(response));
  }

 private:
  int sum_ = 0;
};

struct GetRunningSumMethod {
  static constexpr const char* MethodName() {
    return "/async_grpc.proto.Math/GetRunningSum";
  }
  using IncomingType = Stream<proto::GetSumRequest>;
  using OutgoingType = Stream<proto::GetSumResponse>;
};

class GetRunningSumHandler : public RpcHandler<GetRunningSumMethod> {
 public:
  void OnRequest(const proto::GetSumRequest& request) override {
    sum_ += request.input();

    // Respond twice to demonstrate bidirectional streaming.
    auto response = common::make_unique<proto::GetSumResponse>();
    response->set_output(sum_);
    Send(std::move(response));
    response = common::make_unique<proto::GetSumResponse>();
    response->set_output(sum_);
    Send(std::move(response));
  }

  void OnReadsDone() override { Finish(::grpc::Status::OK); }

 private:
  int sum_ = 0;
};

struct GetSquareMethod {
  static constexpr const char* MethodName() {
    return "/async_grpc.proto.Math/GetSquare";
  }
  using IncomingType = proto::GetSquareRequest;
  using OutgoingType = proto::GetSquareResponse;
};

class GetSquareHandler : public RpcHandler<GetSquareMethod> {
 public:
  void OnRequest(const proto::GetSquareRequest& request) override {
    if (request.input() < 0) {
      Finish(::grpc::Status(::grpc::INTERNAL, "internal error"));
    }
    auto response = common::make_unique<proto::GetSquareResponse>();
    response->set_output(request.input() * request.input());
    std::cout << "on request: " << request.input() << std::endl;
    Send(std::move(response));
  }
};

struct GetEchoMethod {
  static constexpr const char* MethodName() {
    return "/async_grpc.proto.Math/GetEcho";
  }
  using IncomingType = proto::GetEchoRequest;
  using OutgoingType = proto::GetEchoResponse;
};

class GetEchoHandler : public RpcHandler<GetEchoMethod> {
 public:
  void OnRequest(const proto::GetEchoRequest& request) override {
    int value = request.input();
    Writer writer = GetWriter();
    GetContext<MathServerContext>()->echo_responder.set_value(
        [writer, value]() {
          auto response = common::make_unique<proto::GetEchoResponse>();
          response->set_output(value);
          return writer.Write(std::move(response));
        });
  }
};

struct GetSequenceMethod {
  static constexpr const char* MethodName() {
    return "/async_grpc.proto.Math/GetSequence";
  }
  using IncomingType = proto::GetSequenceRequest;
  using OutgoingType = Stream<proto::GetSequenceResponse>;
};

class GetSequenceHandler : public RpcHandler<GetSequenceMethod> {
 public:
  void OnRequest(const proto::GetSequenceRequest& request) override {
    for (int i = 0; i < request.input(); ++i) {
      auto response = common::make_unique<proto::GetSequenceResponse>();
      response->set_output(i);
      Send(std::move(response));
    }
    Finish(::grpc::Status::OK);
  }
};

// TODO(cschuet): Due to the hard-coded part these tests will become flaky when
// run in parallel. It would be nice to find a way to solve that. gRPC also
// allows to communicate over UNIX domain sockets.
const std::string kServerAddress = "localhost:50051";
const std::size_t kNumThreads = 1;

class ServerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Server::Builder server_builder;
    server_builder.SetServerAddress(kServerAddress);
    server_builder.SetNumGrpcThreads(kNumThreads);
    server_builder.SetNumEventThreads(kNumThreads);
    server_builder.RegisterHandler<GetSumHandler>();
    server_builder.RegisterHandler<GetSquareHandler>();
    server_builder.RegisterHandler<GetRunningSumHandler>();
    server_builder.RegisterHandler<GetEchoHandler>();
    server_builder.RegisterHandler<GetSequenceHandler>();
    server_ = server_builder.Build();

    client_channel_ = ::grpc::CreateChannel(
        kServerAddress, ::grpc::InsecureChannelCredentials());
  }

  std::unique_ptr<Server> server_;
  std::shared_ptr<::grpc::Channel> client_channel_;
};

TEST_F(ServerTest, StartAndStopServerTest) {
  server_->Start();
  server_->Shutdown();
}

TEST_F(ServerTest, ProcessRpcStreamTest) {
  server_->SetExecutionContext(common::make_unique<MathServerContext>());
  server_->Start();

  Client<GetSumMethod> client(client_channel_);
  for (int i = 0; i < 3; ++i) {
    proto::GetSumRequest request;
    request.set_input(i);
    EXPECT_TRUE(client.Write(request));
  }
  EXPECT_TRUE(client.StreamWritesDone());
  EXPECT_TRUE(client.StreamFinish().ok());
  EXPECT_EQ(client.response().output(), 33);

  server_->Shutdown();
}

TEST_F(ServerTest, ProcessUnaryRpcTest) {
  server_->Start();

  Client<GetSquareMethod> client(client_channel_);
  proto::GetSquareRequest request;
  request.set_input(11);
  EXPECT_TRUE(client.Write(request));
  EXPECT_EQ(client.response().output(), 121);

  server_->Shutdown();
}

TEST_F(ServerTest, ProcessBidiStreamingRpcTest) {
  server_->Start();

  Client<GetRunningSumMethod> client(client_channel_);
  for (int i = 0; i < 3; ++i) {
    proto::GetSumRequest request;
    request.set_input(i);
    EXPECT_TRUE(client.Write(request));
  }
  client.StreamWritesDone();
  proto::GetSumResponse response;
  std::list<int> expected_responses = {0, 0, 1, 1, 3, 3};
  while (client.StreamRead(&response)) {
    EXPECT_EQ(expected_responses.front(), response.output());
    expected_responses.pop_front();
  }
  EXPECT_TRUE(expected_responses.empty());
  EXPECT_TRUE(client.StreamFinish().ok());

  server_->Shutdown();
}

TEST_F(ServerTest, WriteFromOtherThread) {
  server_->SetExecutionContext(common::make_unique<MathServerContext>());
  server_->Start();

  Server* server = server_.get();
  std::thread response_thread([server]() {
    std::future<EchoResponder> responder_future =
        server->GetContext<MathServerContext>()->echo_responder.get_future();
    responder_future.wait();
    auto responder = responder_future.get();
    CHECK(responder());
  });

  Client<GetEchoMethod> client(client_channel_);
  proto::GetEchoRequest request;
  request.set_input(13);
  EXPECT_TRUE(client.Write(request));
  response_thread.join();
  EXPECT_EQ(client.response().output(), 13);

  server_->Shutdown();
}

TEST_F(ServerTest, ProcessServerStreamingRpcTest) {
  server_->Start();

  Client<GetSequenceMethod> client(client_channel_);
  proto::GetSequenceRequest request;
  request.set_input(12);

  client.Write(request);
  proto::GetSequenceResponse response;
  for (int i = 0; i < 12; ++i) {
    EXPECT_TRUE(client.StreamRead(&response));
    EXPECT_EQ(response.output(), i);
  }
  EXPECT_FALSE(client.StreamRead(&response));
  EXPECT_TRUE(client.StreamFinish().ok());

  server_->Shutdown();
}

TEST_F(ServerTest, RetryWithUnrecoverableError) {
  server_->Start();

  Client<GetSquareMethod> client(
      client_channel_, common::FromSeconds(5),
      CreateUnlimitedConstantDelayStrategy(common::FromSeconds(1),
                                           {::grpc::INTERNAL}));
  proto::GetSquareRequest request;
  request.set_input(-11);
  EXPECT_FALSE(client.Write(request));

  server_->Shutdown();
}

}  // namespace
}  // namespace async_grpc
