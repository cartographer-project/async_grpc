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

#include "async_grpc/opencensus_span.h"

#if BUILD_TRACING

namespace async_grpc {

std::unique_ptr<Span> OpencensusSpan::StartSpan(const std::string& name,
                                                const OpencensusSpan* parent) {
  return std::unique_ptr<OpencensusSpan>(new OpencensusSpan(name, parent));
}

std::unique_ptr<Span> OpencensusSpan::CreateChildSpan(const std::string& name) {
  return std::unique_ptr<OpencensusSpan>(new OpencensusSpan(name, this));
}

void OpencensusSpan::SetStatus(const ::grpc::Status& status) {
  span_.SetStatus((opencensus::trace::StatusCode)status.error_code());
}

void OpencensusSpan::End() { span_.End(); }

OpencensusSpan::OpencensusSpan(const std::string& name,
                               const OpencensusSpan* parent)
    : span_(opencensus::trace::Span::StartSpan(
          name, parent ? &parent->span_ : nullptr)) {}

}  // namespace async_grpc

#endif  // BUILD_TRACING
