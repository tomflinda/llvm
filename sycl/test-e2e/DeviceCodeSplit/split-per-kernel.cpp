// RUN: %{build} -Wno-error=unused-command-line-argument -fsycl-device-code-split=per_kernel -o %t.out \
// RUN: -fsycl-dead-args-optimization
// RUN: %{run} %t.out

#include <sycl/detail/core.hpp>
#include <sycl/kernel_bundle.hpp>

class Kern1;
class Kern2;
class Kern3;

int main() {
  sycl::queue Q;
  int Data = 0;
  {
    sycl::buffer<int, 1> Buf(&Data, sycl::range<1>(1));
    auto KernelID1 = sycl::get_kernel_id<Kern1>();
    auto KernelID2 = sycl::get_kernel_id<Kern2>();
    auto KernelID3 = sycl::get_kernel_id<Kern3>();
    auto KB = sycl::get_kernel_bundle<sycl::bundle_state::executable>(
        Q.get_context(), {KernelID1});
    auto Krn = KB.get_kernel(KernelID1);

    assert(!KB.has_kernel(KernelID2));
    assert(!KB.has_kernel(KernelID3));

    Q.submit([&](sycl::handler &Cgh) {
      auto Acc = Buf.get_access<sycl::access::mode::read_write>(Cgh);
      Cgh.single_task<Kern1>(Krn, [=]() { Acc[0] = 1; });
    });
  }
  assert(Data == 1);

  {
    sycl::buffer<int, 1> Buf(&Data, sycl::range<1>(1));
    auto KernelID1 = sycl::get_kernel_id<Kern1>();
    auto KernelID2 = sycl::get_kernel_id<Kern2>();
    auto KernelID3 = sycl::get_kernel_id<Kern3>();
    auto KB = sycl::get_kernel_bundle<sycl::bundle_state::executable>(
        Q.get_context(), {KernelID2});
    auto Krn = KB.get_kernel(KernelID2);

    assert(!KB.has_kernel(KernelID1));
    assert(!KB.has_kernel(KernelID3));

    Q.submit([&](sycl::handler &Cgh) {
      auto Acc = Buf.get_access<sycl::access::mode::read_write>(Cgh);
      Cgh.single_task<Kern2>(Krn, [=]() { Acc[0] = 2; });
    });
  }
  assert(Data == 2);

  {
    sycl::buffer<int, 1> Buf(&Data, sycl::range<1>(1));
    auto KernelID1 = sycl::get_kernel_id<Kern1>();
    auto KernelID2 = sycl::get_kernel_id<Kern2>();
    auto KernelID3 = sycl::get_kernel_id<Kern3>();
    auto KB = sycl::get_kernel_bundle<sycl::bundle_state::executable>(
        Q.get_context(), {KernelID3});
    auto Krn = KB.get_kernel(KernelID3);

    assert(!KB.has_kernel(KernelID1));
    assert(!KB.has_kernel(KernelID2));

    Q.submit([&](sycl::handler &Cgh) {
      auto Acc = Buf.get_access<sycl::access::mode::read_write>(Cgh);
      Cgh.single_task<Kern3>(Krn, [=]() { Acc[0] = 3; });
    });
  }
  assert(Data == 3);

  return 0;
}
