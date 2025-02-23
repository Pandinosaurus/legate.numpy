/* Copyright 2021 NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __NUMPY_EQUAL_H__
#define __NUMPY_EQUAL_H__

#include "universal_function.h"
#include <functional>

namespace legate {
namespace numpy {
template <class T>
struct EqualOperation : std::equal_to<T> {
  constexpr static auto op_code = NumPyOpCode::NUMPY_EQUAL;
};

template <class T>
using Equal = BinaryUniversalFunction<EqualOperation<T>>;
}  // namespace numpy
}  // namespace legate

#endif  // __NUMPY_EQUAL_H__
