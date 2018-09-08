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

#ifndef CPP_GRPC_OPENCENSUS_SPAN_H
#define CPP_GRPC_OPENCENSUS_SPAN_H

#if BUILD_TRACING

#include <memory>
#include <string>

#include "async_grpc/span.h"
#include "opencensus/trace/span.h"

namespace async_grpc {

// An implementation of the Span interface backed by Opencensus.
class OpencensusSpan : public Span {
 public:
  static std::unique_ptr<Span> StartSpan(
      const std::string& name, const OpencensusSpan* parent = nullptr);

  std::unique_ptr<Span> CreateChildSpan(const std::string& name) override;
  void SetStatus(const ::grpc::Status& status) override;
  void End() override;

 private:
  OpencensusSpan(const std::string& name,
                 const OpencensusSpan* parent = nullptr);

  opencensus::trace::Span span_;
};

}  // namespace async_grpc

#endif  // BUILD_TRACING

#endif  // CPP_GRPC_OPENCENSUS_SPAN_H