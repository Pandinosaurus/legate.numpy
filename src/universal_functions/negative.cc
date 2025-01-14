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

#include "negative.h"

// instantiate Negative's tasks for all Legate types
namespace legate {
namespace numpy {
template void Negative<__half>::instantiate_tasks();
template void Negative<float>::instantiate_tasks();
template void Negative<double>::instantiate_tasks();
template void Negative<int16_t>::instantiate_tasks();
template void Negative<int32_t>::instantiate_tasks();
template void Negative<int64_t>::instantiate_tasks();
template void Negative<uint16_t>::instantiate_tasks();
template void Negative<uint32_t>::instantiate_tasks();
template void Negative<uint64_t>::instantiate_tasks();
template void Negative<bool>::instantiate_tasks();
template void Negative<complex<float>>::instantiate_tasks();
template void Negative<complex<double>>::instantiate_tasks();
}  // namespace numpy
}  // namespace legate
