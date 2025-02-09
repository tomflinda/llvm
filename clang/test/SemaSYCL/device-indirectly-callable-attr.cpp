// RUN: %clang_cc1 -fsycl-is-device -fsyntax-only -verify %s
// RUN: not %clang_cc1 -fsycl-is-device -ast-dump %s | FileCheck %s
// RUN: %clang_cc1 -verify -DNO_SYCL %s
// RUN: %clang_cc1 -fsycl-is-host -fsyntax-only -verify -DSYCL_HOST %s

#if !defined(NO_SYCL) || defined(SYCL_HOST)

[[intel::device_indirectly_callable]] // expected-warning {{'device_indirectly_callable' attribute only applies to functions}}
int N;

[[intel::device_indirectly_callable(3)]] // expected-error {{'device_indirectly_callable' attribute takes no arguments}}
void
bar() {}

[[intel::device_indirectly_callable]] // expected-error {{'device_indirectly_callable' attribute cannot be applied to a function without external linkage}}
static void
func1() {}

namespace {
[[intel::device_indirectly_callable]] // expected-error {{'device_indirectly_callable' attribute cannot be applied to a function without external linkage}}
void
func2() {}

struct UnnX {};
}

[[intel::device_indirectly_callable]] // expected-error {{'device_indirectly_callable' attribute cannot be applied to a function without external linkage}}
void func4(UnnX) {}

class A {
  [[intel::device_indirectly_callable]] A() {}

  [[intel::device_indirectly_callable]] int func3() {}
};

class B {
  [[intel::device_indirectly_callable]] virtual int foo() {}

  [[intel::device_indirectly_callable]] virtual int bar() = 0;
};

void helper() {}

[[intel::device_indirectly_callable]]
void foo() {
  helper();
}

#else

[[intel::device_indirectly_callable]] // expected-warning {{'device_indirectly_callable' attribute ignored}}
void
baz() {}

#endif // NO_SYCL

// CHECK: FunctionDecl {{.*}} helper
//
// CHECK: FunctionDecl {{.*}} foo
// CHECK: SYCLDeviceAttr
// CHECK: SYCLDeviceIndirectlyCallableAttr
