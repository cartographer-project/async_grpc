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

#ifndef CPP_GRPC_SPAN_H
#define CPP_GRPC_SPAN_H

#include <memory>

#include "grpc++/grpc++.h"

namespace async_grpc {

// Span represents a trace span. All implementations of the Span interface
// must be thread-safe.
class Span {
 public:
  Span() = default;
  virtual ~Span() = default;

  // Creates a new child span with this span as the parent.
  virtual std::unique_ptr<Span> CreateChildSpan(const std::string& name) = 0;

  // Sets the status of the Span. See status_code.h for canonical codes.
  virtual void SetStatus(const ::grpc::Status& status) = 0;

  // Marks the end of a Span. No further changes can be made to the Span after
  // End is called.
  virtual void End() = 0;
};

}  // namespace async_grpc

#endif  // CPP_GRPC_SPAN_H