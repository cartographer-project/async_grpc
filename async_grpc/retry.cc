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

#include "async_grpc/retry.h"

#include <chrono>
#include <cmath>
#include <thread>

#include "glog/logging.h"

namespace async_grpc {

RetryStrategy CreateRetryStrategy(RetryIndicator retry_indicator,
                                  RetryDelayCalculator retry_delay_calculator) {
  return [retry_indicator, retry_delay_calculator](
             int failed_attempts, const ::grpc::Status& status) {
    if (!retry_indicator(failed_attempts, status)) {
      return optional<Duration>();
    }
    return optional<Duration>(retry_delay_calculator(failed_attempts));
  };
}

RetryIndicator CreateLimitedRetryIndicator(int max_attempts) {
  return [max_attempts](int failed_attempts, const ::grpc::Status& status) {
    return failed_attempts < max_attempts;
  };
}

RetryIndicator CreateUnlimitedRetryIndicator() {
  return [](int failed_attempts, const ::grpc::Status& status) { return true; };
}

RetryIndicator CreateUnlimitedRetryIndicator(
    const std::set<::grpc::StatusCode>& unrecoverable_codes) {
  return
      [unrecoverable_codes](int failed_attempts, const ::grpc::Status& status) {
        return unrecoverable_codes.count(status.error_code()) <= 0;
      };
}

RetryDelayCalculator CreateBackoffDelayCalculator(Duration min_delay,
                                                  float backoff_factor) {
  return [min_delay, backoff_factor](int failed_attempts) -> Duration {
    CHECK_GE(failed_attempts, 0);
    using common::FromSeconds;
    using common::ToSeconds;
    return FromSeconds(std::pow(backoff_factor, failed_attempts - 1) *
                       ToSeconds(min_delay));
  };
}

RetryDelayCalculator CreateConstantDelayCalculator(Duration delay) {
  return [delay](int failed_attempts) -> Duration { return delay; };
}

RetryStrategy CreateLimitedBackoffStrategy(Duration min_delay,
                                           float backoff_factor,
                                           int max_attempts) {
  return CreateRetryStrategy(
      CreateLimitedRetryIndicator(max_attempts),
      CreateBackoffDelayCalculator(min_delay, backoff_factor));
}

RetryStrategy CreateUnlimitedConstantDelayStrategy(Duration delay) {
  return CreateRetryStrategy(CreateUnlimitedRetryIndicator(),
                             CreateConstantDelayCalculator(delay));
}

RetryStrategy CreateUnlimitedConstantDelayStrategy(
    Duration delay, const std::set<::grpc::StatusCode>& unrecoverable_codes) {
  return CreateRetryStrategy(CreateUnlimitedRetryIndicator(unrecoverable_codes),
                             CreateConstantDelayCalculator(delay));
}

bool RetryWithStrategy(RetryStrategy retry_strategy,
                       std::function<::grpc::Status()> op,
                       std::function<void()> reset) {
  optional<Duration> delay;
  int failed_attemps = 0;
  for (;;) {
    ::grpc::Status status = op();
    if (status.ok()) {
      return true;
    }
    if (!retry_strategy) {
      return false;
    }
    delay = retry_strategy(++failed_attemps, status);
    if (!delay.has_value()) {
      break;
    }
    LOG(INFO) << "Retrying after "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     delay.value())
                     .count()
              << " milliseconds.";
    std::this_thread::sleep_for(delay.value());
    if (reset) {
      reset();
    }
  }
  return false;
}

}  // namespace async_grpc
