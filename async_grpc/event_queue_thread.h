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

#ifndef CPP_GRPC_EVENT_QUEUE_THREAD_H
#define CPP_GRPC_EVENT_QUEUE_THREAD_H

#include <memory>
#include <thread>

#include "async_grpc/rpc.h"
#include "async_grpc/common/blocking_queue.h"

namespace async_grpc {

class EventQueueThread {
 public:
  using EventQueueRunner = std::function<void(EventQueue*)>;

  EventQueueThread();

  EventQueue* event_queue();

  void Start(EventQueueRunner runner);
  void Shutdown();

 private:
  std::unique_ptr<EventQueue> event_queue_;
  std::unique_ptr<std::thread> thread_;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_EVENT_QUEUE_THREAD_H
