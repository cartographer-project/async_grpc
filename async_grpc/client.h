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

#ifndef CPP_GRPC_CLIENT_H
#define CPP_GRPC_CLIENT_H

#include "async_grpc/retry.h"
#include "async_grpc/rpc_handler_interface.h"
#include "async_grpc/rpc_service_method_traits.h"

#include "grpc++/grpc++.h"
#include "grpc++/impl/codegen/client_unary_call.h"
#include "grpc++/impl/codegen/proto_utils.h"
#include "grpc++/impl/codegen/sync_stream.h"

#include "glog/logging.h"

namespace async_grpc {
template <typename RpcServiceMethodConcept>
class Client {
  using RpcServiceMethod = RpcServiceMethodTraits<RpcServiceMethodConcept>;
  using RequestType = typename RpcServiceMethod::RequestType;
  using ResponseType = typename RpcServiceMethod::ResponseType;

 public:
  Client(std::shared_ptr<::grpc::Channel> channel, RetryStrategy retry_strategy)
      : channel_(channel),
        client_context_(common::make_unique<::grpc::ClientContext>()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_),
        retry_strategy_(retry_strategy) {
    CHECK(!retry_strategy ||
          rpc_method_.method_type() == ::grpc::internal::RpcMethod::NORMAL_RPC)
        << "Retry is currently only support for NORMAL_RPC.";
  }

  Client(std::shared_ptr<::grpc::Channel> channel)
      : channel_(channel),
        client_context_(common::make_unique<::grpc::ClientContext>()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_) {}

  bool StreamRead(ResponseType *response) {
    switch (rpc_method_.method_type()) {
      case ::grpc::internal::RpcMethod::BIDI_STREAMING:
        InstantiateClientReaderWriterIfNeeded();
        return client_reader_writer_->Read(response);
      case ::grpc::internal::RpcMethod::SERVER_STREAMING:
        CHECK(client_reader_);
        return client_reader_->Read(response);
      default:
        LOG(FATAL) << "This method is for server or bidirectional streaming "
                      "RPC only.";
    }
  }

  bool Write(const RequestType &request) {
    return RetryWithStrategy(retry_strategy_,
                             [this, &request] { return WriteImpl(request); },
                             [this] { Reset(); });
  }

  bool StreamWritesDone() {
    switch (rpc_method_.method_type()) {
      case ::grpc::internal::RpcMethod::CLIENT_STREAMING:
        InstantiateClientWriterIfNeeded();
        return client_writer_->WritesDone();
      case ::grpc::internal::RpcMethod::BIDI_STREAMING:
        InstantiateClientReaderWriterIfNeeded();
        return client_reader_writer_->WritesDone();
      default:
        LOG(FATAL) << "This method is for client or bidirectional streaming "
                      "RPC only.";
    }
  }

  ::grpc::Status StreamFinish() {
    switch (rpc_method_.method_type()) {
      case ::grpc::internal::RpcMethod::CLIENT_STREAMING:
        InstantiateClientWriterIfNeeded();
        return client_writer_->Finish();
      case ::grpc::internal::RpcMethod::BIDI_STREAMING:
        InstantiateClientReaderWriterIfNeeded();
        return client_reader_writer_->Finish();
      case ::grpc::internal::RpcMethod::SERVER_STREAMING:
        CHECK(client_reader_);
        return client_reader_->Finish();
      default:
        LOG(FATAL) << "This method is for streaming RPC only.";
    }
  }

  const ResponseType &response() {
    CHECK(rpc_method_.method_type() ==
              ::grpc::internal::RpcMethod::NORMAL_RPC ||
          rpc_method_.method_type() ==
              ::grpc::internal::RpcMethod::CLIENT_STREAMING);
    return response_;
  }

 private:
  void Reset() {
    client_context_ = common::make_unique<::grpc::ClientContext>();
  }

  bool WriteImpl(const RequestType &request) {
    switch (rpc_method_.method_type()) {
      case ::grpc::internal::RpcMethod::NORMAL_RPC:
        return MakeBlockingUnaryCall(request, &response_).ok();
      case ::grpc::internal::RpcMethod::CLIENT_STREAMING:
        InstantiateClientWriterIfNeeded();
        return client_writer_->Write(request);
      case ::grpc::internal::RpcMethod::BIDI_STREAMING:
        InstantiateClientReaderWriterIfNeeded();
        return client_reader_writer_->Write(request);
      case ::grpc::internal::RpcMethod::SERVER_STREAMING:
        InstantiateClientReader(request);
        return true;
    }
    LOG(FATAL) << "Not reached.";
  }

  void InstantiateClientWriterIfNeeded() {
    CHECK_EQ(rpc_method_.method_type(),
             ::grpc::internal::RpcMethod::CLIENT_STREAMING);
    if (!client_writer_) {
      client_writer_.reset(
          ::grpc::internal::ClientWriterFactory<RequestType>::Create(
              channel_.get(), rpc_method_, client_context_.get(), &response_));
    }
  }

  void InstantiateClientReaderWriterIfNeeded() {
    CHECK_EQ(rpc_method_.method_type(),
             ::grpc::internal::RpcMethod::BIDI_STREAMING);
    if (!client_reader_writer_) {
      client_reader_writer_.reset(
          ::grpc::internal::ClientReaderWriterFactory<
              RequestType, ResponseType>::Create(channel_.get(), rpc_method_,
                                                 client_context_.get()));
    }
  }

  void InstantiateClientReader(const RequestType &request) {
    CHECK_EQ(rpc_method_.method_type(),
             ::grpc::internal::RpcMethod::SERVER_STREAMING);
    client_reader_.reset(
        ::grpc::internal::ClientReaderFactory<ResponseType>::Create(
            channel_.get(), rpc_method_, client_context_.get(), request));
  }

  ::grpc::Status MakeBlockingUnaryCall(const RequestType &request,
                                       ResponseType *response) {
    CHECK_EQ(rpc_method_.method_type(),
             ::grpc::internal::RpcMethod::NORMAL_RPC);
    return ::grpc::internal::BlockingUnaryCall(
        channel_.get(), rpc_method_, client_context_.get(), request, response);
  }

  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::grpc::ClientContext> client_context_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;

  std::unique_ptr<::grpc::ClientWriter<RequestType>> client_writer_;
  std::unique_ptr<::grpc::ClientReaderWriter<RequestType, ResponseType>>
      client_reader_writer_;
  std::unique_ptr<::grpc::ClientReader<ResponseType>> client_reader_;
  ResponseType response_;
  RetryStrategy retry_strategy_;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_CLIENT_H
