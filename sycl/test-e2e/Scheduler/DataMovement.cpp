// RUN: %{build} -o %t.out %debug_option
// RUN: %{run} %t.out
//
// XFAIL: spirv-backend && run-mode
// XFAIL-TRACKER: https://github.com/llvm/llvm-project/issues/122075
//
//==-------------------------- DataMovement.cpp ----------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// The test checks that no additional host allocation is performed by the SYCL
// RT if host ptr is used

#include <sycl/detail/core.hpp>

#include <vector>

#include "../helpers.hpp"

using sycl_access_mode = sycl::access::mode;

template <typename T> class CustomAllocator {
public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;

public:
  template <typename U> struct rebind {
    typedef CustomAllocator<U> other;
  };

  void construct(pointer Ptr, const_reference Val) {
    new (Ptr) value_type(Val);
  }

  void destroy(pointer Ptr) {}

  pointer address(reference Val) const { return &Val; }
  const_pointer address(const_reference Val) { return &Val; }

  pointer allocate(size_t Size) {
    throw std::runtime_error("Attempt to make host allocation for the buffer!");
  }

  // Release allocated memory
  void deallocate(pointer Ptr, size_t size) {
    throw std::runtime_error("Attempt to dealocate user's memory!");
  }

  bool operator==(const CustomAllocator &) { return true; }
  bool operator!=(const CustomAllocator &rhs) { return false; }
};

int main() {
  TestQueue Queue1(sycl::default_selector_v);
  TestQueue Queue2(sycl::default_selector_v);

  std::vector<int> Data(1);

  sycl::buffer<int, 1, CustomAllocator<int>> Buf(
      Data.data(), Data.size(), {sycl::property::buffer::use_host_ptr()});

  Queue1.submit([&](sycl::handler &CGH) {
    auto BufAcc = Buf.get_access<sycl_access_mode::read_write>(CGH);
    CGH.single_task<class first_kernel>([=]() { BufAcc[0] = 41; });
  });

  Queue1.wait_and_throw();

  { sycl::host_accessor HostAcc(Buf, sycl::read_only); }

  Queue2.submit([&](sycl::handler &CGH) {
    auto BufAcc = Buf.get_access<sycl_access_mode::read_write>(CGH);
    CGH.single_task<class second_kernel>([=]() { BufAcc[0] = 42; });
  });

  Queue2.wait_and_throw();

  { sycl::host_accessor HostAcc(Buf, sycl::read_only); }

  return 0;
}
