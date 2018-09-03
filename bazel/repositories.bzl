# Copyright 2018 The Cartographer Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""External dependencies."""

def repositories():
    _maybe(
        native.http_archive,
        name = "com_github_antonovvk_bazel_rules",
        sha256 = "ba75b07d3fd297375a6688e9a16583eb616e7a74b3d5e8791e7a222cf36ab26e",
        strip_prefix = "bazel_rules-98ddd7e4f7c63ea0868f08bcc228463dac2f9f12",
        urls = [
            "https://mirror.bazel.build/github.com/antonovvk/bazel_rules/archive/98ddd7e4f7c63ea0868f08bcc228463dac2f9f12.tar.gz",
            "https://github.com/antonovvk/bazel_rules/archive/98ddd7e4f7c63ea0868f08bcc228463dac2f9f12.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_github_gflags_gflags",
        sha256 = "6e16c8bc91b1310a44f3965e616383dbda48f83e8c1eaa2370a215057b00cabe",
        strip_prefix = "gflags-77592648e3f3be87d6c7123eb81cbad75f9aef5a",
        urls = [
            "https://mirror.bazel.build/github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
            "https://github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_google_glog",
        sha256 = "1ee310e5d0a19b9d584a855000434bb724aa744745d5b8ab1855c85bff8a8e21",
        strip_prefix = "glog-028d37889a1e80e8a07da1b8945ac706259e5fd8",
        urls = [
            "https://mirror.bazel.build/github.com/google/glog/archive/028d37889a1e80e8a07da1b8945ac706259e5fd8.tar.gz",
            "https://github.com/google/glog/archive/028d37889a1e80e8a07da1b8945ac706259e5fd8.tar.gz",
        ],
    )

    _maybe(
        native.new_http_archive,
        name = "net_zlib_zlib",
        build_file = "@com_github_googlecartographer_async_grpc//bazel/third_party:zlib.BUILD",
        sha256 = "6d4d6640ca3121620995ee255945161821218752b551a1a180f4215f7d124d45",
        strip_prefix = "zlib-cacf7f1d4e3d44d871b605da3b647f07d718623f",
        urls = [
            "https://mirror.bazel.build/github.com/madler/zlib/archive/cacf7f1d4e3d44d871b605da3b647f07d718623f.tar.gz",
            "https://github.com/madler/zlib/archive/cacf7f1d4e3d44d871b605da3b647f07d718623f.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_google_googletest",
        sha256 = "c18f281fd6621bb264570b99860a0241939b4a251c9b1af709b811d33bc63af8",
        strip_prefix = "googletest-e3bd4cbeaeef3cee65a68a8bd3c535cb779e9b6d",
        urls = [
            "https://mirror.bazel.build/github.com/google/googletest/archive/e3bd4cbeaeef3cee65a68a8bd3c535cb779e9b6d.tar.gz",
            "https://github.com/google/googletest/archive/e3bd4cbeaeef3cee65a68a8bd3c535cb779e9b6d.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_google_protobuf",
        sha256 = "0cc6607e2daa675101e9b7398a436f09167dffb8ca0489b0307ff7260498c13c",
        strip_prefix = "protobuf-3.5.0",
        urls = [
            "https://mirror.bazel.build/github.com/google/protobuf/archive/v3.5.0.tar.gz",
            "https://github.com/google/protobuf/archive/v3.5.0.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_github_grpc_grpc",
        strip_prefix = "grpc-1.14.2",
        urls = [
            "https://github.com/grpc/grpc/archive/v1.14.2.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_github_census_instrumentation_opencensus_cpp",
        strip_prefix = "opencensus-cpp-6202fb80e26f7e9780167a0f12c81834e16cf05a",
        urls = [
            "https://mirror.bazel.build/github.com/census-instrumentation/opencensus-cpp/archive/6202fb80e26f7e9780167a0f12c81834e16cf05a.tar.gz",
            "https://github.com/census-instrumentation/opencensus-cpp/archive/6202fb80e26f7e9780167a0f12c81834e16cf05a.tar.gz",
        ],
    )

    _maybe(
        native.http_archive,
        name = "com_github_google_benchmark",
        urls = ["https://github.com/google/benchmark/archive/master.zip"],
        strip_prefix = "benchmark-master",
        sha256 = "4339fd994efd0cc8166a9bbb87f9e9f518b5de0bf29cf36999bd434c35ca7505",
    )

    _maybe(
        native.new_http_archive,
        name = "com_github_curl",
        urls = ["https://github.com/curl/curl/archive/e2ef8d6fa11b2345e10b89db525920f2a0d5fd79.zip"],
        strip_prefix = "curl-e2ef8d6fa11b2345e10b89db525920f2a0d5fd79",
        build_file = "@com_github_googlecartographer_async_grpc//bazel/third_party:curl.BUILD",
    )

    _maybe(
        native.new_http_archive,
        name = "com_github_rapidjson",
        urls = ["https://github.com/Tencent/rapidjson/archive/master.zip"],
        strip_prefix = "rapidjson-master",
        build_file = "@com_github_googlecartographer_async_grpc//bazel/third_party:rapidjson.BUILD",
    )

    native.bind(
        name = "grpc_cpp_plugin",
        actual = "@com_github_grpc_grpc//:grpc_cpp_plugin",
    )
    native.bind(
        name = "grpc++_codegen_proto",
        actual = "@com_github_grpc_grpc//:grpc++_codegen_proto",
    )

def _maybe(repo_rule, name, **kwargs):
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)
