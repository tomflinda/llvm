import os
import sys
from os.path import dirname, join, abspath

sys.path.insert(0, abspath(join(dirname(__file__), "..")))

from gen_convert_common import (
    types,
    vector_sizes,
    saturation,
    rounding_modes,
    conditional_guard,
    close_conditional_guard,
    clc_core_fn_name,
)

# remove extra, non-opencl type
types.remove("schar")

# OpenCL built-in library: type conversion functions
#
# Copyright (c) 2013 Victor Oliveira <victormatheus@gmail.com>
# Copyright (c) 2013 Jesse Towner <jessetowner@lavabit.com>
# Copyright (c) 2024 Romaric Jodin <rjodin@chromium.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# This script generates the file convert-clc.cl, which contains all of the
# OpenCL functions in the form:
#
# convert_<destTypen><_sat><_roundingMode>(<sourceTypen>)

import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
    "--clspv", action="store_true", help="Generate the clspv variant of the code"
)
args = parser.parse_args()

clspv = args.clspv

types = [
    "char",
    "uchar",
    "short",
    "ushort",
    "int",
    "uint",
    "long",
    "ulong",
    "half",
    "float",
    "double",
]
int_types = ["char", "uchar", "short", "ushort", "int", "uint", "long", "ulong"]
unsigned_types = ["uchar", "ushort", "uint", "ulong"]
float_types = ["half", "float", "double"]
int64_types = ["long", "ulong"]
float64_types = ["double"]
float16_types = ["half"]
vector_sizes = ["", "2", "3", "4", "8", "16"]
half_sizes = [("2", ""), ("4", "2"), ("8", "4"), ("16", "8")]

saturation = ["", "_sat"]
rounding_modes = ["_rtz", "_rte", "_rtp", "_rtn"]

bool_type = {
    "char": "char",
    "uchar": "char",
    "short": "short",
    "ushort": "short",
    "int": "int",
    "uint": "int",
    "long": "long",
    "ulong": "long",
    "half": "short",
    "float": "int",
    "double": "long",
}

unsigned_type = {
    "char": "uchar",
    "uchar": "uchar",
    "short": "ushort",
    "ushort": "ushort",
    "int": "uint",
    "uint": "uint",
    "long": "ulong",
    "ulong": "ulong",
}

sizeof_type = {
    "char": 1,
    "uchar": 1,
    "short": 2,
    "ushort": 2,
    "int": 4,
    "uint": 4,
    "long": 8,
    "ulong": 8,
    "half": 2,
    "float": 4,
    "double": 8,
}

limit_max = {
    "char": "CHAR_MAX",
    "uchar": "UCHAR_MAX",
    "short": "SHRT_MAX",
    "ushort": "USHRT_MAX",
    "int": "INT_MAX",
    "uint": "UINT_MAX",
    "long": "LONG_MAX",
    "ulong": "ULONG_MAX",
    "half": "0x1.ffcp+15",
}

limit_min = {
    "char": "CHAR_MIN",
    "uchar": "0",
    "short": "SHRT_MIN",
    "ushort": "0",
    "int": "INT_MIN",
    "uint": "0",
    "long": "LONG_MIN",
    "ulong": "0",
    "half": "-0x1.ffcp+15",
}


def conditional_guard(src, dst):
    int64_count = 0
    float64_count = 0
    float16_count = 0
    if src in int64_types:
        int64_count = int64_count + 1
    elif src in float64_types:
        float64_count = float64_count + 1
    elif src in float16_types:
        float16_count = float16_count + 1
    if dst in int64_types:
        int64_count = int64_count + 1
    elif dst in float64_types:
        float64_count = float64_count + 1
    elif dst in float16_types:
        float16_count = float16_count + 1
    if float64_count > 0:
        # In embedded profile, if cl_khr_fp64 is supported cles_khr_int64 has to be
        print("#ifdef cl_khr_fp64")
        return True
    elif float16_count > 0:
        print("#if defined cl_khr_fp16")
        return True
    elif int64_count > 0:
        print("#if defined cles_khr_int64 || !defined(__EMBEDDED_PROFILE__)")
        return True
    return False

print(
    """/* !!!! AUTOGENERATED FILE generated by convert_type.py !!!!!

   DON'T CHANGE THIS FILE. MAKE YOUR CHANGES TO convert_type.py AND RUN:
   $ ./generate-conversion-type-cl.sh

   OpenCL type conversion functions

   Copyright (c) 2013 Victor Oliveira <victormatheus@gmail.com>
   Copyright (c) 2013 Jesse Towner <jessetowner@lavabit.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <clc/clc.h>
#include <core/clc_core.h>

#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif

#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif

#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#if defined(__EMBEDDED_PROFILE__) && !defined(cles_khr_int64)
#error Embedded profile that supports cl_khr_fp64 also has to support cles_khr_int64
#endif

#endif

#ifdef cles_khr_int64
#pragma OPENCL EXTENSION cles_khr_int64 : enable
#endif

"""
)


def generate_ocl_fn(src, dst, size="", mode="", sat=""):
    close_conditional = conditional_guard(src, dst)

    print(
        """_CLC_DEF _CLC_OVERLOAD
{DST}{N} convert_{DST}{N}{S}{M}({SRC}{N} x)
{{
  return {CORE_FN}(x);
}}
""".format(
            CORE_FN=clc_core_fn_name(dst, size=size, sat=sat, mode=mode),
            SRC=src,
            DST=dst,
            N=size,
            S=sat,
            M=mode,
        )
    )

    close_conditional_guard(close_conditional)


for src in types:
    for dst in types:
        for size in vector_sizes:
            for sat in saturation:
                generate_ocl_fn(src, dst, size, "", sat)
                for mode in rounding_modes:
                    generate_ocl_fn(src, dst, size, mode, sat)
