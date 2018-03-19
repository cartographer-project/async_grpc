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
  _maybe(native.http_archive,
      name = "com_github_nelhage_boost",
      sha256 = "5c88fc077f6b8111e997fec5146e5f9940ae9a2016eb9949447fcb4b482bcdb3",
      strip_prefix = "rules_boost-7289bb1d8f938fdf98078297768c122ee9e11c9e",
      urls = [
          "https://mirror.bazel.build/github.com/nelhage/rules_boost/archive/7289bb1d8f938fdf98078297768c122ee9e11c9e.tar.gz",
          "https://github.com/nelhage/rules_boost/archive/7289bb1d8f938fdf98078297768c122ee9e11c9e.tar.gz",
      ],
  )

  _maybe(native.http_archive,
      name = "com_github_antonovvk_bazel_rules",
      sha256 = "ba75b07d3fd297375a6688e9a16583eb616e7a74b3d5e8791e7a222cf36ab26e",
      strip_prefix = "bazel_rules-98ddd7e4f7c63ea0868f08bcc228463dac2f9f12",
      urls = [
          "https://mirror.bazel.build/github.com/antonovvk/bazel_rules/archive/98ddd7e4f7c63ea0868f08bcc228463dac2f9f12.tar.gz",
          "https://github.com/antonovvk/bazel_rules/archive/98ddd7e4f7c63ea0868f08bcc228463dac2f9f12.tar.gz",
      ],
  )

  _maybe(native.http_archive,
      name = "com_github_gflags_gflags",
      sha256 = "6e16c8bc91b1310a44f3965e616383dbda48f83e8c1eaa2370a215057b00cabe",
      strip_prefix = "gflags-77592648e3f3be87d6c7123eb81cbad75f9aef5a",
      urls = [
          "https://mirror.bazel.build/github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
          "https://github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
      ],
  )

  _maybe(native.http_archive,
      name = "com_google_glog",
      sha256 = "1ee310e5d0a19b9d584a855000434bb724aa744745d5b8ab1855c85bff8a8e21",
      strip_prefix = "glog-028d37889a1e80e8a07da1b8945ac706259e5fd8",
      urls = [
          "https://mirror.bazel.build/github.com/google/glog/archive/028d37889a1e80e8a07da1b8945ac706259e5fd8.tar.gz",
          "https://github.com/google/glog/archive/028d37889a1e80e8a07da1b8945ac706259e5fd8.tar.gz",
      ],
  )

  _maybe(native.new_http_archive,
      name = "net_zlib_zlib",
      build_file = "@com_github_cschuet_async_grpc//bazel/third_party:zlib.BUILD",
      sha256 = "6d4d6640ca3121620995ee255945161821218752b551a1a180f4215f7d124d45",
      strip_prefix = "zlib-cacf7f1d4e3d44d871b605da3b647f07d718623f",
      urls = [
          "https://mirror.bazel.build/github.com/madler/zlib/archive/cacf7f1d4e3d44d871b605da3b647f07d718623f.tar.gz",
          "https://github.com/madler/zlib/archive/cacf7f1d4e3d44d871b605da3b647f07d718623f.tar.gz",
      ],
  )

  _maybe(native.http_archive,
      name = "com_google_googletest",
      sha256 = "c18f281fd6621bb264570b99860a0241939b4a251c9b1af709b811d33bc63af8",
      strip_prefix = "googletest-e3bd4cbeaeef3cee65a68a8bd3c535cb779e9b6d",
      urls = [
          "https://mirror.bazel.build/github.com/google/googletest/archive/e3bd4cbeaeef3cee65a68a8bd3c535cb779e9b6d.tar.gz",
          "https://github.com/google/googletest/archive/e3bd4cbeaeef3cee65a68a8bd3c535cb779e9b6d.tar.gz",
      ],
  )

  _maybe(native.http_archive,
      name = "com_google_protobuf",
      sha256 = "0cc6607e2daa675101e9b7398a436f09167dffb8ca0489b0307ff7260498c13c",
      strip_prefix = "protobuf-3.5.0",
      urls = [
          "https://mirror.bazel.build/github.com/google/protobuf/archive/v3.5.0.tar.gz",
          "https://github.com/google/protobuf/archive/v3.5.0.tar.gz",
      ],
  )

  _maybe(native.http_archive,
      name = "com_github_grpc_grpc",
      sha256 = "2fdde7d64e6fb1a397bf2aa23aeddcdcf276652d9e48270e94eb0dc94d7c1345",
      strip_prefix = "grpc-20e7074e4101b4fdbae1764caa952301b38957c4",
      urls = [
          "https://mirror.bazel.build/github.com/grpc/grpc/archive/20e7074e4101b4fdbae1764caa952301b38957c4.tar.gz",
          "https://github.com/grpc/grpc/archive/20e7074e4101b4fdbae1764caa952301b38957c4.tar.gz",
      ],
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
    repo_rule(name=name, **kwargs)
