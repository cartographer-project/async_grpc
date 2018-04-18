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

#ifndef CPP_GRPC_COMMON_TOKEN_FILE_H_
#define CPP_GRPC_COMMON_TOKEN_FILE_H_

#include <grpc++/security/credentials.h>
#include <memory>
#include <string>

#include "async_grpc/common/time.h"

namespace async_grpc {

// Returns CallCredentials that read OAuth tokens from the specified file.
// The file is a flat file that contains only an OAuth bearer token. The
// credentials are cached for the length of one refresh_interval.
std::shared_ptr<grpc::CallCredentials> TokenFileCredentials(
    const std::string& filename, const common::Duration& refresh_interval);

}  // namespace async_grpc

#endif  // CPP_GRPC_COMMON_TOKEN_FILE_H_
