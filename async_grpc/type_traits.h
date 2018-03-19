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

#ifndef CPP_GRPC_TYPE_TRAITS_H_
#define CPP_GRPC_TYPE_TRAITS_H_

#include <grpc++/grpc++.h>

#include <cstdint>
#include <type_traits>

// This helper allows us to stamp out traits structs which allow to check for
// the existence of member functions.
// Example:
//   struct Foo { static char* foo() { return nullptr; } };
//   DEFINE_HAS_SIGNATURE(has_foo, T::foo, char*(*)(void));
//   static_assert(has_foo_v<Foo>, "foo() is not implemented")
#define DEFINE_HAS_SIGNATURE(traitsName, funcName, signature)                  \
  template <typename U>                                                        \
  class traitsName {                                                           \
   private:                                                                    \
    template <typename T, T>                                                   \
    struct helper;                                                             \
                                                                               \
    template <typename T>                                                      \
    static std::uint8_t check(helper<signature, &funcName>*);                  \
    template <typename T>                                                      \
    static std::uint16_t check(...);                                           \
                                                                               \
   public:                                                                     \
    static constexpr bool value = sizeof(check<U>(0)) == sizeof(std::uint8_t); \
  };

#define DEFINE_HAS_MEMBER_TYPE(traitsName, Type)           \
  template <class T>                                       \
  class traitsName {                                       \
   private:                                                \
    struct Fallback {                                      \
      struct Type {};                                      \
    };                                                     \
    struct Derived : T, Fallback {};                       \
                                                           \
    template <class U>                                     \
    static std::uint16_t& check(typename U::Type*);        \
    template <typename U>                                  \
    static std::uint8_t& check(U*);                        \
                                                           \
   public:                                                 \
    static constexpr bool value =                          \
        sizeof(check<Derived>(0)) == sizeof(std::uint8_t); \
  };

namespace async_grpc {

template <typename Request>
class Stream {
  using type = Request;
};

template <template <typename> class, typename T>
struct Strip {
  using type = T;
};

template <template <typename> class T, typename Param>
struct Strip<T, T<Param>> {
  using type = Param;
};

template <typename T>
using StripStream = typename Strip<Stream, T>::type;

template <typename Incoming, typename Outgoing>
struct RpcType
    : public std::integral_constant<::grpc::internal::RpcMethod::RpcType,
                                    ::grpc::internal::RpcMethod::NORMAL_RPC> {};

template <typename Incoming, typename Outgoing>
struct RpcType<Stream<Incoming>, Outgoing>
    : public std::integral_constant<
          ::grpc::internal::RpcMethod::RpcType,
          ::grpc::internal::RpcMethod::CLIENT_STREAMING> {};

template <typename Incoming, typename Outgoing>
struct RpcType<Incoming, Stream<Outgoing>>
    : public std::integral_constant<
          ::grpc::internal::RpcMethod::RpcType,
          ::grpc::internal::RpcMethod::SERVER_STREAMING> {};

template <typename Incoming, typename Outgoing>
struct RpcType<Stream<Incoming>, Stream<Outgoing>>
    : public std::integral_constant<
          ::grpc::internal::RpcMethod::RpcType,
          ::grpc::internal::RpcMethod::BIDI_STREAMING> {};

}  // namespace async_grpc

#endif  // CPP_GRPC_TYPE_TRAITS_H_
