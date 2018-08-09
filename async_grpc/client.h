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

// Wraps a method invocation for all rpc types, unary, client streaming,
// server streaming, or bidirectional.
// It cannot be used for multiple invocations.
// It is not thread safe.
template <typename RpcServiceMethodConcept,
          ::grpc::internal::RpcMethod::RpcType StreamType =
              RpcServiceMethodTraits<RpcServiceMethodConcept>::StreamType>
class Client {};

// TODO(gaschler): Move specializations to separate header files.
template <typename RpcServiceMethodConcept>
class Client<RpcServiceMethodConcept, ::grpc::internal::RpcMethod::NORMAL_RPC> {
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
        retry_strategy_(retry_strategy) {}

  Client(std::shared_ptr<::grpc::Channel> channel)
      : channel_(channel),
        client_context_(common::make_unique<::grpc::ClientContext>()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_) {}

  bool Write(const RequestType& request, ::grpc::Status* status = nullptr) {
    ::grpc::Status internal_status;
    bool result = RetryWithStrategy(retry_strategy_,
                                    [this, &request, &internal_status] {
                                      WriteImpl(request, &internal_status);
                                      return internal_status;
                                    },
                                    [this] { Reset(); });

    if (status != nullptr) {
      *status = internal_status;
    }
    return result;
  }

  const ResponseType& response() { return response_; }

 private:
  void Reset() {
    client_context_ = common::make_unique<::grpc::ClientContext>();
  }

  bool WriteImpl(const RequestType& request, ::grpc::Status* status) {
    auto status_normal_rpc = MakeBlockingUnaryCall(request, &response_);
    if (status != nullptr) {
      *status = status_normal_rpc;
    }
    return status_normal_rpc.ok();
  }
  ::grpc::Status MakeBlockingUnaryCall(const RequestType& request,
                                       ResponseType* response) {
    return ::grpc::internal::BlockingUnaryCall(
        channel_.get(), rpc_method_, client_context_.get(), request, response);
  }

  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::grpc::ClientContext> client_context_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;

  ResponseType response_;
  RetryStrategy retry_strategy_;
};

template <typename RpcServiceMethodConcept>
class Client<RpcServiceMethodConcept,
             ::grpc::internal::RpcMethod::CLIENT_STREAMING> {
  using RpcServiceMethod = RpcServiceMethodTraits<RpcServiceMethodConcept>;
  using RequestType = typename RpcServiceMethod::RequestType;
  using ResponseType = typename RpcServiceMethod::ResponseType;

 public:
  Client(std::shared_ptr<::grpc::Channel> channel)
      : channel_(channel),
        client_context_(common::make_unique<::grpc::ClientContext>()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_) {}

  bool Write(const RequestType& request, ::grpc::Status* status = nullptr) {
    ::grpc::Status internal_status;
    WriteImpl(request, &internal_status);
    if (status != nullptr) {
      *status = internal_status;
    }
    return internal_status.ok();
  }

  bool StreamWritesDone() {
    InstantiateClientWriterIfNeeded();
    return client_writer_->WritesDone();
  }

  ::grpc::Status StreamFinish() {
    InstantiateClientWriterIfNeeded();
    return client_writer_->Finish();
  }

  const ResponseType& response() { return response_; }

 private:
  void Reset() {
    client_context_ = common::make_unique<::grpc::ClientContext>();
  }

  bool WriteImpl(const RequestType& request, ::grpc::Status* status) {
    InstantiateClientWriterIfNeeded();
    return client_writer_->Write(request);
  }

  void InstantiateClientWriterIfNeeded() {
    if (!client_writer_) {
      client_writer_.reset(
          ::grpc::internal::ClientWriterFactory<RequestType>::Create(
              channel_.get(), rpc_method_, client_context_.get(), &response_));
    }
  }

  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::grpc::ClientContext> client_context_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;

  std::unique_ptr<::grpc::ClientWriter<RequestType>> client_writer_;
  ResponseType response_;
};

template <typename RpcServiceMethodConcept>
class Client<RpcServiceMethodConcept,
             ::grpc::internal::RpcMethod::SERVER_STREAMING> {
  using RpcServiceMethod = RpcServiceMethodTraits<RpcServiceMethodConcept>;
  using RequestType = typename RpcServiceMethod::RequestType;
  using ResponseType = typename RpcServiceMethod::ResponseType;

 public:
  Client(std::shared_ptr<::grpc::Channel> channel)
      : channel_(channel),
        client_context_(common::make_unique<::grpc::ClientContext>()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_) {}

  bool StreamRead(ResponseType* response) {
    CHECK(client_reader_);
    return client_reader_->Read(response);
  }

  bool Write(const RequestType& request, ::grpc::Status* status = nullptr) {
    ::grpc::Status internal_status;
    WriteImpl(request, &internal_status);
    if (status != nullptr) {
      *status = internal_status;
    }
    return internal_status.ok();
  }

  ::grpc::Status StreamFinish() {
    CHECK(client_reader_);
    return client_reader_->Finish();
  }

 private:
  void Reset() {
    client_context_ = common::make_unique<::grpc::ClientContext>();
  }

  bool WriteImpl(const RequestType& request, ::grpc::Status* status) {
    InstantiateClientReader(request);
    return true;
  }

  void InstantiateClientReader(const RequestType& request) {
    client_reader_.reset(
        ::grpc::internal::ClientReaderFactory<ResponseType>::Create(
            channel_.get(), rpc_method_, client_context_.get(), request));
  }

  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::grpc::ClientContext> client_context_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;

  std::unique_ptr<::grpc::ClientReader<ResponseType>> client_reader_;
};

template <typename RpcServiceMethodConcept>
class Client<RpcServiceMethodConcept,
             ::grpc::internal::RpcMethod::BIDI_STREAMING> {
  using RpcServiceMethod = RpcServiceMethodTraits<RpcServiceMethodConcept>;
  using RequestType = typename RpcServiceMethod::RequestType;
  using ResponseType = typename RpcServiceMethod::ResponseType;

 public:
  Client(std::shared_ptr<::grpc::Channel> channel)
      : channel_(channel),
        client_context_(common::make_unique<::grpc::ClientContext>()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_) {}

  bool StreamRead(ResponseType* response) {
    InstantiateClientReaderWriterIfNeeded();
    return client_reader_writer_->Read(response);
  }

  bool Write(const RequestType& request, ::grpc::Status* status = nullptr) {
    ::grpc::Status internal_status;
    WriteImpl(request, &internal_status);
    if (status != nullptr) {
      *status = internal_status;
    }
    return internal_status.ok();
  }

  bool StreamWritesDone() {
    InstantiateClientReaderWriterIfNeeded();
    return client_reader_writer_->WritesDone();
  }

  ::grpc::Status StreamFinish() {
    InstantiateClientReaderWriterIfNeeded();
    return client_reader_writer_->Finish();
  }

 private:
  void Reset() {
    client_context_ = common::make_unique<::grpc::ClientContext>();
  }

  bool WriteImpl(const RequestType& request, ::grpc::Status* status) {
    InstantiateClientReaderWriterIfNeeded();
    return client_reader_writer_->Write(request);
  }

  void InstantiateClientReaderWriterIfNeeded() {
    if (!client_reader_writer_) {
      client_reader_writer_.reset(
          ::grpc::internal::ClientReaderWriterFactory<
              RequestType, ResponseType>::Create(channel_.get(), rpc_method_,
                                                 client_context_.get()));
    }
  }

  std::shared_ptr<::grpc::Channel> channel_;
  std::unique_ptr<::grpc::ClientContext> client_context_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;

  std::unique_ptr<::grpc::ClientReaderWriter<RequestType, ResponseType>>
      client_reader_writer_;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_CLIENT_H
