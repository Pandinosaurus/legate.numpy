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

#include "tanh.h"

// instantiate Tanh's tasks for all Legate types
namespace legate {
namespace numpy {
template void Tanh<__half>::instantiate_tasks();
template void Tanh<float>::instantiate_tasks();
template void Tanh<double>::instantiate_tasks();
template void Tanh<int16_t>::instantiate_tasks();
template void Tanh<int32_t>::instantiate_tasks();
template void Tanh<int64_t>::instantiate_tasks();
template void Tanh<uint16_t>::instantiate_tasks();
template void Tanh<uint32_t>::instantiate_tasks();
template void Tanh<uint64_t>::instantiate_tasks();
template void Tanh<bool>::instantiate_tasks();
template void Tanh<complex<float>>::instantiate_tasks();
template void Tanh<complex<double>>::instantiate_tasks();
}  // namespace numpy
}  // namespace legate
