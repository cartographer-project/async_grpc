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

#include "async_grpc/token_file_credentials.h"

#include <grpc++/support/status.h>
#include <fstream>
#include <string>

#include "async_grpc/common/make_unique.h"
#include "async_grpc/common/mutex.h"

namespace async_grpc {
namespace {

class OAuthPlugin : public grpc::MetadataCredentialsPlugin {
 public:
  OAuthPlugin(const std::string& file, const common::Duration& refresh_interval)
      : file_(file),
        refresh_interval_(refresh_interval),
        refresh_time_(std::chrono::steady_clock::now()) {}

  grpc::Status GetMetadata(
      grpc::string_ref service_url, grpc::string_ref method_name,
      const grpc::AuthContext& channel_auth_context,
      std::multimap<grpc::string, grpc::string>* metadata) override {
    std::string token = GetToken();
    if (token.empty()) {
      return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                          "No authentication token");
    }
    metadata->insert(std::make_pair("authorization", "Bearer " + token));
    return grpc::Status::OK;
  }

 private:
  const std::string file_;
  const common::Duration refresh_interval_;
  common::Mutex mutex_;
  std::string token_ GUARDED_BY(mutex_);
  std::chrono::steady_clock::time_point refresh_time_ GUARDED_BY(mutex_);

  std::string GetToken() {
    auto now = std::chrono::steady_clock::now();
    common::MutexLocker lock(&mutex_);
    if (refresh_time_ <= now) {
      std::string token(
          std::istreambuf_iterator<char>(std::ifstream(file_).rdbuf()),
          std::istreambuf_iterator<char>());
      token_ = token;
      refresh_time_ = now + refresh_interval_;
    }
    return token_;
  }
};

}  // namespace

std::shared_ptr<grpc::CallCredentials> TokenFileCredentials(
    const std::string& filename, const common::Duration& refresh_interval) {
  return grpc::MetadataCredentialsFromPlugin(
      common::make_unique<OAuthPlugin>(filename, refresh_interval));
}

}  // namespace async_grpc
