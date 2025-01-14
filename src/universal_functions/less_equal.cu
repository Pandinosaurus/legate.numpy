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

#include "less_equal.h"

// Instantiate LessEqual's tasks' gpu variants
namespace legate {
namespace numpy {
template void LessEqual<__half>::instantiate_task_gpu_variants();
template void LessEqual<float>::instantiate_task_gpu_variants();
template void LessEqual<double>::instantiate_task_gpu_variants();
template void LessEqual<int16_t>::instantiate_task_gpu_variants();
template void LessEqual<int32_t>::instantiate_task_gpu_variants();
template void LessEqual<int64_t>::instantiate_task_gpu_variants();
template void LessEqual<uint16_t>::instantiate_task_gpu_variants();
template void LessEqual<uint32_t>::instantiate_task_gpu_variants();
template void LessEqual<uint64_t>::instantiate_task_gpu_variants();
template void LessEqual<bool>::instantiate_task_gpu_variants();
template void LessEqual<complex<float>>::instantiate_task_gpu_variants();
template void LessEqual<complex<double>>::instantiate_task_gpu_variants();
}  // namespace numpy
}  // namespace legate
