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

#ifndef CPP_GRPC_COMMON_COMPLETION_QUEUE_POOL_H_
#define CPP_GRPC_COMMON_COMPLETION_QUEUE_POOL_H_

#include <memory>
#include <thread>
#include <vector>

#include "common/mutex.h"
#include "grpc++/grpc++.h"

namespace async_grpc {

class AsyncClientInterface;

struct ClientEvent {
  enum class Event { FINISH = 0 };
  ClientEvent(Event event, AsyncClientInterface* async_client)
      : event(event), async_client(async_client) {}
  Event event;
  AsyncClientInterface* async_client;
  bool ok = false;
};

class CompletionQueue {
 public:
  CompletionQueue() = default;

  void Start();
  void Shutdown();

  ::grpc::CompletionQueue* completion_queue() { return &completion_queue_; }

 private:
  void RunCompletionQueue();

  ::grpc::CompletionQueue completion_queue_;
  std::unique_ptr<std::thread> thread_;
};

class CompletionQueuePool {
 public:
  static void SetNumberCompletionQueues(size_t number_completion_queues);
  static void Shutdown();

  // Returns a random completion queue.
  static ::grpc::CompletionQueue* GetCompletionQueue();

 private:
  using CompletionQueueRunner = std::function<void(::grpc::CompletionQueue*)>;

  CompletionQueuePool();
  ~CompletionQueuePool() = default;

  void Initialize();
  static CompletionQueuePool* completion_queue_pool();

  common::Mutex mutex_;
  bool initialized_ = false;
  size_t number_completion_queues_;
  std::vector<CompletionQueue> completion_queues_;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_COMMON_COMPLETION_QUEUE_POOL_H_
