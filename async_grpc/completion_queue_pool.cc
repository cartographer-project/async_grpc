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

#include <cstdlib>

#include "async_grpc/async_client.h"
#include "async_grpc/completion_queue_pool.h"
#include "common/make_unique.h"
#include "glog/logging.h"

namespace async_grpc {
namespace {

size_t kDefaultNumberCompletionQueues = 2;

}  // namespace

void CompletionQueue::Start() {
  CHECK(!thread_) << "CompletionQueue already started.";
  thread_ =
      common::make_unique<std::thread>([this]() { RunCompletionQueue(); });
}

void CompletionQueue::Shutdown() {
  CHECK(thread_) << "CompletionQueue not yet started.";
  LOG(INFO) << "Shutting down client completion queue " << this;
  completion_queue_.Shutdown();
  thread_->join();
}

void CompletionQueue::RunCompletionQueue() {
  bool ok;
  void* tag;
  while (completion_queue_.Next(&tag, &ok)) {
    auto* client_event = static_cast<ClientEvent*>(tag);
    client_event->ok = ok;
    client_event->async_client->HandleEvent(*client_event);
  }
}

CompletionQueuePool* CompletionQueuePool::completion_queue_pool() {
  static CompletionQueuePool* const kInstance = new CompletionQueuePool();
  return kInstance;
}

void CompletionQueuePool::SetNumberCompletionQueues(
    size_t number_completion_queues) {
  CompletionQueuePool* pool = completion_queue_pool();
  CHECK(!pool->initialized_)
      << "Can't change number of completion queues after initialization.";
  CHECK_GT(number_completion_queues, 0u);
  pool->number_completion_queues_ = number_completion_queues;
}

::grpc::CompletionQueue* CompletionQueuePool::GetCompletionQueue() {
  CompletionQueuePool* pool = completion_queue_pool();
  pool->Initialize();
  const unsigned int qid = rand() % pool->completion_queues_.size();
  return pool->completion_queues_.at(qid).completion_queue();
}

void CompletionQueuePool::Start() {
  CompletionQueuePool* pool = completion_queue_pool();
  pool->Initialize();
}

void CompletionQueuePool::Shutdown() {
  LOG(INFO) << "Shutting down CompletionQueuePool";
  CompletionQueuePool* pool = completion_queue_pool();
  common::MutexLocker locker(&pool->mutex_);
  for (size_t i = 0; i < pool->completion_queues_.size(); ++i) {
    pool->completion_queues_.at(i).Shutdown();
  }
  pool->completion_queues_.clear();
  pool->initialized_ = false;
}

CompletionQueuePool::CompletionQueuePool()
    : number_completion_queues_(kDefaultNumberCompletionQueues) {
}

CompletionQueuePool::~CompletionQueuePool() {
  LOG(INFO) << "~CompletionQueuePool";
}

void CompletionQueuePool::Initialize() {
  common::MutexLocker locker(&mutex_);
  if (initialized_) {
    return;
  }
  completion_queues_.resize(number_completion_queues_);
  for (auto& completion_queue : completion_queues_) {
    completion_queue.Start();
  }
  initialized_ = true;
}

}  // namespace async_grpc
