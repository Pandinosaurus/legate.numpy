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

#include "max.h"
#include "proj.h"

#include <float.h>

using namespace Legion;

namespace legate {
namespace numpy {

#if 0
    template<typename T> template<typename TASK>
    /*static*/ void MaxTask<T>::set_layout_constraints(LegateVariant variant, 
                                 TaskLayoutConstraintSet &layout_constraints)
    {
      // Need a reduction accessor for the first region
      layout_constraints.add_layout_constraint(0, 
          Legate::get_reduction_layout(MaxReduction<T>::REDOP_ID));
      for (int idx = 1; idx < TASK::REGIONS; idx++)
        layout_constraints.add_layout_constraint(idx, Legate::get_soa_layout());
    }
#endif

template <typename T>
/*static*/ void MaxTask<T>::cpu_variant(const Task* task,
                                        const std::vector<PhysicalRegion>& regions,
                                        Context ctx,
                                        Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  // We still need to unpack the axis
  derez.unpack_dimension();
  const int collapse_dim = derez.unpack_dimension();
  const int init_dim     = derez.unpack_dimension();
  const T initial_value =
    (task->futures.size() == 1) ? task->futures[0].get_result<T>() : MaxReduction<T>::identity;
  switch (init_dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) return;
      const AccessorWO<T, 1> out =
        (collapse_dim >= 0) ? derez.unpack_accessor_WO<T, 1>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_WO<T, 1>(regions[0], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) out[x] = initial_value;
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) return;
      const AccessorWO<T, 2> out =
        (collapse_dim >= 0) ? derez.unpack_accessor_WO<T, 2>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_WO<T, 2>(regions[0], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) out[x][y] = initial_value;
      break;
    }
    default: assert(false);  // shouldn't see any other cases
  }
  const int dim = derez.unpack_dimension();
  switch (dim) {
    // Should never get the case of 1 as this would just be a copy since
    // reducing our only dimension should have called MaxReducTask
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) return;
      const AccessorRW<T, 2> inout =
        (collapse_dim >= 0) ? derez.unpack_accessor_RW<T, 2, 1>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_RW<T, 2>(regions[0], rect);
      const AccessorRO<T, 2> in = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          MaxReduction<T>::template fold<true /*exclusive*/>(inout[x][y], in[x][y]);
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) return;
      const AccessorRW<T, 3> inout =
        (collapse_dim >= 0) ? derez.unpack_accessor_RW<T, 3, 2>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_RW<T, 3>(regions[0], rect);
      const AccessorRO<T, 3> in = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
            MaxReduction<T>::template fold<true /*exclusive*/>(inout[x][y][z], in[x][y][z]);
      break;
    }
    default: assert(false);
  }
}

#ifdef LEGATE_USE_OPENMP
template <typename T>
/*static*/ void MaxTask<T>::omp_variant(const Task* task,
                                        const std::vector<PhysicalRegion>& regions,
                                        Context ctx,
                                        Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int axis         = derez.unpack_dimension();
  const int collapse_dim = derez.unpack_dimension();
  const int init_dim     = derez.unpack_dimension();
  const T initial_value =
    (task->futures.size() == 1) ? task->futures[0].get_result<T>() : MaxReduction<T>::identity;
  switch (init_dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) return;
      const AccessorWO<T, 1> out =
        (collapse_dim >= 0) ? derez.unpack_accessor_WO<T, 1>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_WO<T, 1>(regions[0], rect);
#pragma omp parallel for
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) out[x] = initial_value;
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) return;
      const AccessorWO<T, 2> out =
        (collapse_dim >= 0) ? derez.unpack_accessor_WO<T, 2>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_WO<T, 2>(regions[0], rect);
#pragma omp parallel for
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) out[x][y] = initial_value;
      break;
    }
    default: assert(false);  // shouldn't see any other cases
  }
  const int dim = derez.unpack_dimension();
  switch (dim) {
    // Should never get the case of 1 as this would just be a copy since
    // reducing our only dimension should have called MaxReducTask
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) return;
      const AccessorRW<T, 2> inout =
        (collapse_dim >= 0) ? derez.unpack_accessor_RW<T, 2, 1>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_RW<T, 2>(regions[0], rect);
      const AccessorRO<T, 2> in = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
      if (axis == 0) {
// Flip dimension order to avoid races between threads on the collapsing dim
#pragma omp parallel for
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
            MaxReduction<T>::template fold<true /*exclusive*/>(inout[x][y], in[x][y]);
      } else {
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            MaxReduction<T>::template fold<true /*exclusive*/>(inout[x][y], in[x][y]);
      }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) return;
      const AccessorRW<T, 3> inout =
        (collapse_dim >= 0) ? derez.unpack_accessor_RW<T, 3, 2>(
                                regions[0], rect, collapse_dim, task->index_point[collapse_dim])
                            : derez.unpack_accessor_RW<T, 3>(regions[0], rect);
      const AccessorRO<T, 3> in = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
      if (axis == 0) {
// Flip dimension order to avoid races between threads on the collapsing dim
#pragma omp parallel for
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(inout[x][y][z], in[x][y][z]);
      } else {
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(inout[x][y][z], in[x][y][z]);
      }
      break;
    }
    default: assert(false);
  }
}
#endif

template <typename T>
/*static*/ T MaxReducTask<T>::cpu_variant(const Task* task,
                                          const std::vector<PhysicalRegion>& regions,
                                          Context ctx,
                                          Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int dim = derez.unpack_dimension();
  T result      = MaxReduction<T>::identity;
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      const AccessorRO<T, 1> in = derez.unpack_accessor_RO<T, 1>(regions[0], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        MaxReduction<T>::template fold<true /*exclusive*/>(result, in[x]);
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      const AccessorRO<T, 2> in = derez.unpack_accessor_RO<T, 2>(regions[0], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          MaxReduction<T>::template fold<true /*exclusive*/>(result, in[x][y]);
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      const AccessorRO<T, 3> in = derez.unpack_accessor_RO<T, 3>(regions[0], rect);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
            MaxReduction<T>::template fold<true /*exclusive*/>(result, in[x][y][z]);
      break;
    }
    default: assert(false);
  }
  return result;
}

#ifdef LEGATE_USE_OPENMP
template <typename T>
/*static*/ T MaxReducTask<T>::omp_variant(const Task* task,
                                          const std::vector<PhysicalRegion>& regions,
                                          Context ctx,
                                          Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int dim = derez.unpack_dimension();
  T result      = MaxReduction<T>::identity;
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      const AccessorRO<T, 1> in = derez.unpack_accessor_RO<T, 1>(regions[0], rect);
#pragma omp parallel
      {
        T local = MaxReduction<T>::identity;
#pragma omp for nowait
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          MaxReduction<T>::template fold<true /*exclusive*/>(local, in[x]);
        MaxReduction<T>::template fold<false /*exclusive*/>(result, local);
      }
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      const AccessorRO<T, 2> in = derez.unpack_accessor_RO<T, 2>(regions[0], rect);
#pragma omp parallel
      {
        T local = MaxReduction<T>::identity;
#pragma omp for nowait
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            MaxReduction<T>::template fold<true /*exclusive*/>(local, in[x][y]);
        MaxReduction<T>::template fold<false /*exclusive*/>(result, local);
      }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      const AccessorRO<T, 3> in = derez.unpack_accessor_RO<T, 3>(regions[0], rect);
#pragma omp parallel
      {
        T local = MaxReduction<T>::identity;
#pragma omp for nowait
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(local, in[x][y][z]);
        MaxReduction<T>::template fold<false /*exclusive*/>(result, local);
      }
      break;
    }
    default: assert(false);
  }
  return result;
}
#endif

template <typename T>
/*static*/ void BinMaxTask<T>::cpu_variant(const Task* task,
                                           const std::vector<PhysicalRegion>& regions,
                                           Context ctx,
                                           Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int dim = derez.unpack_dimension();
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 2) {
        const AccessorRW<T, 1> out = derez.unpack_accessor_RW<T, 1>(regions[0], rect);
        const AccessorRO<T, 1> in  = derez.unpack_accessor_RO<T, 1>(regions[1], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          MaxReduction<T>::template fold<true /*exclusive*/>(out[x], in[x]);
      } else {
        assert(task->regions.size() == 3);
        const AccessorWO<T, 1> out = derez.unpack_accessor_WO<T, 1>(regions[0], rect);
        const AccessorRO<T, 1> in1 = derez.unpack_accessor_RO<T, 1>(regions[1], rect);
        const AccessorRO<T, 1> in2 = derez.unpack_accessor_RO<T, 1>(regions[2], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) out[x] = MAX(in1[x], in2[x]);
      }
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 2) {
        const AccessorRW<T, 2> out = derez.unpack_accessor_RW<T, 2>(regions[0], rect);
        const AccessorRO<T, 2> in  = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y], in[x][y]);
      } else {
        const AccessorWO<T, 2> out = derez.unpack_accessor_WO<T, 2>(regions[0], rect);
        const AccessorRO<T, 2> in1 = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
        const AccessorRO<T, 2> in2 = derez.unpack_accessor_RO<T, 2>(regions[2], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) out[x][y] = MAX(in1[x][y], in2[x][y]);
      }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 2) {
        const AccessorRW<T, 3> out = derez.unpack_accessor_RW<T, 3>(regions[0], rect);
        const AccessorRO<T, 3> in  = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y][z], in[x][y][z]);
      } else {
        const AccessorWO<T, 3> out = derez.unpack_accessor_WO<T, 3>(regions[0], rect);
        const AccessorRO<T, 3> in1 = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
        const AccessorRO<T, 3> in2 = derez.unpack_accessor_RO<T, 3>(regions[2], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              out[x][y][z] = MAX(in1[x][y][z], in2[x][y][z]);
      }
      break;
    }
    default: assert(false);
  }
}

#ifdef LEGATE_USE_OPENMP
template <typename T>
/*static*/ void BinMaxTask<T>::omp_variant(const Task* task,
                                           const std::vector<PhysicalRegion>& regions,
                                           Context ctx,
                                           Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int dim = derez.unpack_dimension();
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 2) {
        const AccessorRW<T, 1> out = derez.unpack_accessor_RW<T, 1>(regions[0], rect);
        const AccessorRO<T, 1> in  = derez.unpack_accessor_RO<T, 1>(regions[1], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          MaxReduction<T>::template fold<true /*exclusive*/>(out[x], in[x]);
      } else {
        assert(task->regions.size() == 3);
        const AccessorWO<T, 1> out = derez.unpack_accessor_WO<T, 1>(regions[0], rect);
        const AccessorRO<T, 1> in1 = derez.unpack_accessor_RO<T, 1>(regions[1], rect);
        const AccessorRO<T, 1> in2 = derez.unpack_accessor_RO<T, 1>(regions[2], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) out[x] = MAX(in1[x], in2[x]);
      }
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 2) {
        const AccessorRW<T, 2> out = derez.unpack_accessor_RW<T, 2>(regions[0], rect);
        const AccessorRO<T, 2> in  = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y], in[x][y]);
      } else {
        const AccessorWO<T, 2> out = derez.unpack_accessor_WO<T, 2>(regions[0], rect);
        const AccessorRO<T, 2> in1 = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
        const AccessorRO<T, 2> in2 = derez.unpack_accessor_RO<T, 2>(regions[2], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) out[x][y] = MAX(in1[x][y], in2[x][y]);
      }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 2) {
        const AccessorRW<T, 3> out = derez.unpack_accessor_RW<T, 3>(regions[0], rect);
        const AccessorRO<T, 3> in  = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y][z], in[x][y][z]);
      } else {
        const AccessorWO<T, 3> out = derez.unpack_accessor_WO<T, 3>(regions[0], rect);
        const AccessorRO<T, 3> in1 = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
        const AccessorRO<T, 3> in2 = derez.unpack_accessor_RO<T, 3>(regions[2], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              out[x][y][z] = MAX(in1[x][y][z], in2[x][y][z]);
      }
      break;
    }
    default: assert(false);
  }
}
#endif  // LEGATE_USE_OPENMP

template <typename T>
/*static*/ void BinMaxBroadcast<T>::cpu_variant(const Task* task,
                                                const std::vector<PhysicalRegion>& regions,
                                                Context ctx,
                                                Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int dim = derez.unpack_dimension();
  assert(task->futures.size() == 1);
  const T in2 = task->futures[0].get_result<T>();
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 1) {
        const AccessorRW<T, 1> out = derez.unpack_accessor_RW<T, 1>(regions[0], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          MaxReduction<T>::template fold<true /*exclusive*/>(out[x], in2);
      } else {
        const AccessorWO<T, 1> out = derez.unpack_accessor_WO<T, 1>(regions[0], rect);
        const AccessorRO<T, 1> in1 = derez.unpack_accessor_RO<T, 1>(regions[1], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) out[x] = MAX(in1[x], in2);
      }
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 1) {
        const AccessorRW<T, 2> out = derez.unpack_accessor_RW<T, 2>(regions[0], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y], in2);
      } else {
        const AccessorWO<T, 2> out = derez.unpack_accessor_WO<T, 2>(regions[0], rect);
        const AccessorRO<T, 2> in1 = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) out[x][y] = MAX(in1[x][y], in2);
      }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 1) {
        const AccessorRW<T, 3> out = derez.unpack_accessor_RW<T, 3>(regions[0], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y][z], in2);
      } else {
        const AccessorWO<T, 3> out = derez.unpack_accessor_WO<T, 3>(regions[0], rect);
        const AccessorRO<T, 3> in1 = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              out[x][y][z] = MAX(in1[x][y][z], in2);
      }
      break;
    }
    default: assert(false);
  }
}

#ifdef LEGATE_USE_OPENMP
template <typename T>
/*static*/ void BinMaxBroadcast<T>::omp_variant(const Task* task,
                                                const std::vector<PhysicalRegion>& regions,
                                                Context ctx,
                                                Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  const int dim = derez.unpack_dimension();
  assert(task->futures.size() == 1);
  const T in2 = task->futures[0].get_result<T>();
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 1) {
        const AccessorRW<T, 1> out = derez.unpack_accessor_RW<T, 1>(regions[0], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          MaxReduction<T>::template fold<true /*exclusive*/>(out[x], in2);
      } else {
        const AccessorWO<T, 1> out = derez.unpack_accessor_WO<T, 1>(regions[0], rect);
        const AccessorRO<T, 1> in1 = derez.unpack_accessor_RO<T, 1>(regions[1], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) out[x] = MAX(in1[x], in2);
      }
      break;
    }
    case 2: {
      if (task->regions.size() == 1) {
        const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
        if (rect.empty()) break;
        const AccessorRW<T, 2> out = derez.unpack_accessor_RW<T, 2>(regions[0], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y], in2);
      } else {
        const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
        if (rect.empty()) break;
        const AccessorWO<T, 2> out = derez.unpack_accessor_WO<T, 2>(regions[0], rect);
        const AccessorRO<T, 2> in1 = derez.unpack_accessor_RO<T, 2>(regions[1], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) out[x][y] = MAX(in1[x][y], in2);
      }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      if (task->regions.size() == 1) {
        const AccessorRW<T, 3> out = derez.unpack_accessor_RW<T, 3>(regions[0], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              MaxReduction<T>::template fold<true /*exclusive*/>(out[x][y][z], in2);
      } else {
        const AccessorWO<T, 3> out = derez.unpack_accessor_WO<T, 3>(regions[0], rect);
        const AccessorRO<T, 3> in1 = derez.unpack_accessor_RO<T, 3>(regions[1], rect);
#pragma omp parallel for
        for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
          for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
            for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++)
              out[x][y][z] = MAX(in1[x][y][z], in2);
      }
      break;
    }
    default: assert(false);
  }
}
#endif

template <typename T>
/*static*/ T BinMaxScalar<T>::cpu_variant(const Task* task,
                                          const std::vector<PhysicalRegion>& regions,
                                          Context ctx,
                                          Runtime* runtime)
{
  assert(task->futures.size() == 2);
  T one = task->futures[0].get_result<T>();
  T two = task->futures[1].get_result<T>();
  return MAX(one, two);
}

template <typename T>
/*static*/ void MaxRadixTask<T>::cpu_variant(const Task* task,
                                             const std::vector<PhysicalRegion>& regions,
                                             Context ctx,
                                             Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  assert(task->regions.size() <= MAX_REDUCTION_RADIX);
  const int radix         = derez.unpack_dimension();
  const int extra_dim_out = derez.unpack_dimension();
  const int extra_dim_in  = derez.unpack_dimension();
  const int dim           = derez.unpack_dimension();
  const coord_t offset    = (extra_dim_in >= 0) ? task->index_point[extra_dim_in] * radix : 0;
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      const AccessorWO<T, 1> out =
        (extra_dim_out >= 0) ? derez.unpack_accessor_WO<T, 1>(
                                 regions[0], rect, extra_dim_out, task->index_point[extra_dim_out])
                             : derez.unpack_accessor_WO<T, 1>(regions[0], rect);
      AccessorRO<T, 1> in[MAX_REDUCTION_RADIX];
      unsigned num_inputs = 0;
      for (unsigned idx = 1; idx < task->regions.size(); idx++)
        if (task->regions[idx].region.exists())
          in[num_inputs++] =
            derez.unpack_accessor_RO<T, 1>(regions[idx], rect, extra_dim_in, offset + idx - 1);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) {
        T val = in[0][x];
        for (unsigned idx = 1; idx < num_inputs; idx++)
          MaxReduction<T>::template fold<true /*exclusive*/>(val, in[idx][x]);
        out[x] = val;
      }
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      const AccessorWO<T, 2> out =
        (extra_dim_out >= 0) ? derez.unpack_accessor_WO<T, 2>(
                                 regions[0], rect, extra_dim_out, task->index_point[extra_dim_out])
                             : derez.unpack_accessor_WO<T, 2>(regions[0], rect);
      AccessorRO<T, 2> in[MAX_REDUCTION_RADIX];
      unsigned num_inputs = 0;
      for (unsigned idx = 1; idx < task->regions.size(); idx++)
        if (task->regions[idx].region.exists())
          in[num_inputs++] =
            derez.unpack_accessor_RO<T, 2>(regions[idx], rect, extra_dim_in, offset + idx - 1);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) {
          T val = in[0][x][y];
          for (unsigned idx = 1; idx < num_inputs; idx++)
            MaxReduction<T>::template fold<true /*exclusive*/>(val, in[idx][x][y]);
          out[x][y] = val;
        }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      const AccessorWO<T, 3> out =
        (extra_dim_out >= 0) ? derez.unpack_accessor_WO<T, 3>(
                                 regions[0], rect, extra_dim_out, task->index_point[extra_dim_out])
                             : derez.unpack_accessor_WO<T, 3>(regions[0], rect);
      AccessorRO<T, 3> in[MAX_REDUCTION_RADIX];
      unsigned num_inputs = 0;
      for (unsigned idx = 1; idx < task->regions.size(); idx++)
        if (task->regions[idx].region.exists())
          in[num_inputs++] =
            derez.unpack_accessor_RO<T, 3>(regions[idx], rect, extra_dim_in, offset + idx - 1);
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++) {
            T val = in[0][x][y][z];
            for (unsigned idx = 1; idx < num_inputs; idx++)
              MaxReduction<T>::template fold<true /*exclusive*/>(val, in[idx][x][y][z]);
            out[x][y][z] = val;
          }
      break;
    }
    default: assert(false);
  }
}

#ifdef LEGATE_USE_OPENMP
template <typename T>
/*static*/ void MaxRadixTask<T>::omp_variant(const Task* task,
                                             const std::vector<PhysicalRegion>& regions,
                                             Context ctx,
                                             Runtime* runtime)
{
  LegateDeserializer derez(task->args, task->arglen);
  assert(task->regions.size() <= MAX_REDUCTION_RADIX);
  const int radix         = derez.unpack_dimension();
  const int extra_dim_out = derez.unpack_dimension();
  const int extra_dim_in  = derez.unpack_dimension();
  const int dim           = derez.unpack_dimension();
  const coord_t offset    = (extra_dim_in >= 0) ? task->index_point[extra_dim_in] * radix : 0;
  switch (dim) {
    case 1: {
      const Rect<1> rect = NumPyProjectionFunctor::unpack_shape<1>(task, derez);
      if (rect.empty()) break;
      const AccessorWO<T, 1> out =
        (extra_dim_out >= 0) ? derez.unpack_accessor_WO<T, 1>(
                                 regions[0], rect, extra_dim_out, task->index_point[extra_dim_out])
                             : derez.unpack_accessor_WO<T, 1>(regions[0], rect);
      AccessorRO<T, 1> in[MAX_REDUCTION_RADIX];
      unsigned num_inputs = 0;
      for (unsigned idx = 1; idx < task->regions.size(); idx++)
        if (task->regions[idx].region.exists())
          in[num_inputs++] =
            derez.unpack_accessor_RO<T, 1>(regions[idx], rect, extra_dim_in, offset + idx - 1);
#pragma omp parallel for
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++) {
        T val = in[0][x];
        for (unsigned idx = 1; idx < num_inputs; idx++)
          MaxReduction<T>::template fold<true /*exclusive*/>(val, in[idx][x]);
        out[x] = val;
      }
      break;
    }
    case 2: {
      const Rect<2> rect = NumPyProjectionFunctor::unpack_shape<2>(task, derez);
      if (rect.empty()) break;
      const AccessorWO<T, 2> out =
        (extra_dim_out >= 0) ? derez.unpack_accessor_WO<T, 2>(
                                 regions[0], rect, extra_dim_out, task->index_point[extra_dim_out])
                             : derez.unpack_accessor_WO<T, 2>(regions[0], rect);
      AccessorRO<T, 2> in[MAX_REDUCTION_RADIX];
      unsigned num_inputs = 0;
      for (unsigned idx = 1; idx < task->regions.size(); idx++)
        if (task->regions[idx].region.exists())
          in[num_inputs++] =
            derez.unpack_accessor_RO<T, 2>(regions[idx], rect, extra_dim_in, offset + idx - 1);
#pragma omp parallel for
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++) {
          T val = in[0][x][y];
          for (unsigned idx = 1; idx < num_inputs; idx++)
            MaxReduction<T>::template fold<true /*exclusive*/>(val, in[idx][x][y]);
          out[x][y] = val;
        }
      break;
    }
    case 3: {
      const Rect<3> rect = NumPyProjectionFunctor::unpack_shape<3>(task, derez);
      if (rect.empty()) break;
      const AccessorWO<T, 3> out =
        (extra_dim_out >= 0) ? derez.unpack_accessor_WO<T, 3>(
                                 regions[0], rect, extra_dim_out, task->index_point[extra_dim_out])
                             : derez.unpack_accessor_WO<T, 3>(regions[0], rect);
      AccessorRO<T, 3> in[MAX_REDUCTION_RADIX];
      unsigned num_inputs = 0;
      for (unsigned idx = 1; idx < task->regions.size(); idx++)
        if (task->regions[idx].region.exists())
          in[num_inputs++] =
            derez.unpack_accessor_RO<T, 3>(regions[idx], rect, extra_dim_in, offset + idx - 1);
#pragma omp parallel for
      for (coord_t x = rect.lo[0]; x <= rect.hi[0]; x++)
        for (coord_t y = rect.lo[1]; y <= rect.hi[1]; y++)
          for (coord_t z = rect.lo[2]; z <= rect.hi[2]; z++) {
            T val = in[0][x][y][z];
            for (unsigned idx = 1; idx < num_inputs; idx++)
              MaxReduction<T>::template fold<true /*exclusive*/>(val, in[idx][x][y][z]);
            out[x][y][z] = val;
          }
      break;
    }
    default: assert(false);
  }
}
#endif

template <typename T>
/*static*/ T MaxScalarTask<T>::cpu_variant(const Task* task,
                                           const std::vector<PhysicalRegion>& regions,
                                           Context ctx,
                                           Runtime* runtime)
{
  assert(task->futures.size() == 2);
  T one       = task->futures[0].get_result<T>();
  const T two = task->futures[1].get_result<T>();
  MaxReduction<T>::template fold<true /*exclusive*/>(one, two);
  return one;
}

INSTANTIATE_ALL_TASKS(MaxTask, static_cast<int>(NumPyOpCode::NUMPY_MAX) * NUMPY_TYPE_OFFSET)
INSTANTIATE_ALL_TASKS(MaxReducTask,
                      static_cast<int>(NumPyOpCode::NUMPY_MAX) * NUMPY_TYPE_OFFSET +
                        NUMPY_REDUCTION_VARIANT_OFFSET)
INSTANTIATE_ALL_TASKS(BinMaxTask, static_cast<int>(NumPyOpCode::NUMPY_MAXIMUM) * NUMPY_TYPE_OFFSET)
INSTANTIATE_ALL_TASKS(BinMaxBroadcast,
                      static_cast<int>(NumPyOpCode::NUMPY_MAXIMUM) * NUMPY_TYPE_OFFSET +
                        NUMPY_BROADCAST_VARIANT_OFFSET)
INSTANTIATE_ALL_TASKS(BinMaxScalar,
                      static_cast<int>(NumPyOpCode::NUMPY_MAXIMUM) * NUMPY_TYPE_OFFSET +
                        NUMPY_SCALAR_VARIANT_OFFSET)
INSTANTIATE_ALL_TASKS(MaxRadixTask,
                      static_cast<int>(NumPyOpCode::NUMPY_MAX_RADIX) * NUMPY_TYPE_OFFSET)
INSTANTIATE_ALL_TASKS(MaxScalarTask,
                      static_cast<int>(NumPyOpCode::NUMPY_MAX) * NUMPY_TYPE_OFFSET +
                        NUMPY_SCALAR_VARIANT_OFFSET)

}  // namespace numpy
}  // namespace legate

namespace  // unnamed
{
static void __attribute__((constructor)) register_tasks(void)
{
  REGISTER_ALL_TASKS(legate::numpy::MaxTask)
  REGISTER_ALL_TASKS_WITH_REDUCTION_RETURN(legate::numpy::MaxReducTask, MaxReduction)
  REGISTER_ALL_TASKS(legate::numpy::BinMaxTask)
  REGISTER_ALL_TASKS(legate::numpy::BinMaxBroadcast)
  REGISTER_ALL_TASKS_WITH_RETURN(legate::numpy::BinMaxScalar)
  REGISTER_ALL_TASKS(legate::numpy::MaxRadixTask)
  REGISTER_ALL_TASKS_WITH_RETURN(legate::numpy::MaxScalarTask)
}
}  // namespace
