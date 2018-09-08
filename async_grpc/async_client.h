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

#include <memory>

#include "async_grpc/rpc_service_method_traits.h"
#include "common/make_unique.h"
#include "completion_queue_pool.h"
#include "glog/logging.h"
#include "grpc++/grpc++.h"
#include "grpc++/impl/codegen/async_stream.h"
#include "grpc++/impl/codegen/async_unary_call.h"

namespace async_grpc {

class ClientEvent;

class AsyncClientInterface {
 public:
  virtual ~AsyncClientInterface() = default;

  virtual void HandleEvent(const ClientEvent& client_event) = 0;
};

template <typename RpcServiceMethodConcept,
          ::grpc::internal::RpcMethod::RpcType StreamType =
              RpcServiceMethodTraits<RpcServiceMethodConcept>::StreamType>
class AsyncClient : public AsyncClientInterface {};

template <typename RpcServiceMethodConcept>
class AsyncClient<RpcServiceMethodConcept,
                  ::grpc::internal::RpcMethod::NORMAL_RPC>
    : public AsyncClientInterface {
  using RpcServiceMethod = RpcServiceMethodTraits<RpcServiceMethodConcept>;
  using RequestType = typename RpcServiceMethod::RequestType;
  using ResponseType = typename RpcServiceMethod::ResponseType;
  using CallbackType =
      std::function<void(const ::grpc::Status&, const ResponseType*)>;

 public:
  AsyncClient(std::shared_ptr<::grpc::Channel> channel, CallbackType callback)
      : channel_(channel),
        callback_(callback),
        completion_queue_(CompletionQueuePool::GetCompletionQueue()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_),
        finish_event_(ClientEvent::Event::FINISH, this) {}

  void WriteAsync(const RequestType& request) {
    response_reader_ =
        std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>>(
            ::grpc::internal::ClientAsyncResponseReaderFactory<
                ResponseType>::Create(channel_.get(), completion_queue_,
                                      rpc_method_, &client_context_, request,
                                      /*start=*/false));
    response_reader_->StartCall();
    response_reader_->Finish(&response_, &status_, (void*)&finish_event_);
  }

  void HandleEvent(const ClientEvent& client_event) override {
    switch (client_event.event) {
      case ClientEvent::Event::FINISH:
        HandleFinishEvent(client_event);
        break;
      default:
        LOG(FATAL) << "Unhandled event type";
    }
  }

  void HandleFinishEvent(const ClientEvent& client_event) {
    if (callback_) {
      callback_(status_, status_.ok() ? &response_ : nullptr);
    }
  }

 private:
  ::grpc::ClientContext client_context_;
  std::shared_ptr<::grpc::Channel> channel_;
  CallbackType callback_;
  ::grpc::CompletionQueue* completion_queue_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;
  std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>>
      response_reader_;
  ClientEvent finish_event_;
  ::grpc::Status status_;
  ResponseType response_;
};

template <typename RpcServiceMethodConcept>
class AsyncClient<RpcServiceMethodConcept,
                  ::grpc::internal::RpcMethod::SERVER_STREAMING>
    : public AsyncClientInterface {
  using RpcServiceMethod = RpcServiceMethodTraits<RpcServiceMethodConcept>;
  using RequestType = typename RpcServiceMethod::RequestType;
  using ResponseType = typename RpcServiceMethod::ResponseType;
  using CallbackType =
      std::function<void(const ::grpc::Status&, const ResponseType*)>;

 public:
  AsyncClient(std::shared_ptr<::grpc::Channel> channel, CallbackType callback)
      : channel_(channel),
        callback_(callback),
        completion_queue_(CompletionQueuePool::GetCompletionQueue()),
        rpc_method_name_(RpcServiceMethod::MethodName()),
        rpc_method_(rpc_method_name_.c_str(), RpcServiceMethod::StreamType,
                    channel_),
        write_event_(ClientEvent::Event::WRITE, this),
        read_event_(ClientEvent::Event::READ, this),
        finish_event_(ClientEvent::Event::FINISH, this) {}

  void WriteAsync(const RequestType& request) {
    // Start the call.
    response_reader_ = std::unique_ptr<::grpc::ClientAsyncReader<ResponseType>>(
        ::grpc::internal::ClientAsyncReaderFactory<ResponseType>::Create(
            channel_.get(), completion_queue_, rpc_method_, &client_context_,
            request,
            /*start=*/true, (void*)&write_event_));
    LOG(INFO) << "Write started.";
  }

  void HandleEvent(const ClientEvent& client_event) override {
    switch (client_event.event) {
      case ClientEvent::Event::WRITE:
        HandleWriteEvent(client_event);
        break;
      case ClientEvent::Event::READ:
        HandleReadEvent(client_event);
        break;
      case ClientEvent::Event::FINISH:
        HandleFinishEvent(client_event);
        break;
      default:
        LOG(FATAL) << "Unhandled event type.";
    }
  }

  void HandleWriteEvent(const ClientEvent& client_event) {
    LOG(INFO) << "HandleWriteEvent()";
    if (!client_event.ok) {
      ::grpc::Status status(::grpc::INTERNAL, "Write failed.");
      if (callback_) {
        callback_(status, nullptr);
        callback_ = nullptr;
      }
      finish_status_ = status;
      response_reader_->Finish(&finish_status_, (void*)&finish_event_);
      return;
    }

    response_reader_->Read(&response_, (void*)&read_event_);
  }

  void HandleReadEvent(const ClientEvent& client_event) {
    if (client_event.ok) {
      if (callback_) {
        callback_(::grpc::Status(), &response_);
        if (!client_event.ok) callback_ = nullptr;
      }
      response_reader_->Read(&response_, (void*)&read_event_);
    } else {
      finish_status_ = ::grpc::Status();
      response_reader_->Finish(&finish_status_, (void*)&finish_event_);
    }
  }

  void HandleFinishEvent(const ClientEvent& client_event) {
    if (callback_) {
      callback_(client_event.ok
                    ? ::grpc::Status()
                    : ::grpc::Status(::grpc::INTERNAL, "Finish failed"),
                nullptr);
      callback_ = nullptr;
    }
  }

 private:
  ::grpc::ClientContext client_context_;
  std::shared_ptr<::grpc::Channel> channel_;
  CallbackType callback_;
  ::grpc::CompletionQueue* completion_queue_;
  const std::string rpc_method_name_;
  const ::grpc::internal::RpcMethod rpc_method_;
  std::unique_ptr<::grpc::ClientAsyncReader<ResponseType>> response_reader_;
  ClientEvent write_event_;
  ClientEvent read_event_;
  ClientEvent finish_event_;
  ::grpc::Status status_;
  ResponseType response_;
  ::grpc::Status finish_status_;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_ASYNC_CLIENT_H
