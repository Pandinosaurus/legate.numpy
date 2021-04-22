# Copyright 2021 NVIDIA Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import numpy as np

import legate.numpy as lg


def test():
    npa = np.array([1, 4, 9])
    a = lg.array(npa)
    assert np.array_equal(lg.sqrt(a), np.sqrt(npa))

    npa = np.array([1, 4, 9], dtype=np.float)
    a = lg.array(npa)
    assert np.array_equal(lg.sqrt(a), np.sqrt(npa))

    npa = np.array([1, 4, 9], dtype=np.float32)
    a = lg.array(npa)
    assert np.array_equal(lg.sqrt(a), np.sqrt(npa))

    npa = np.array([1, 4, 9], dtype=np.float64)
    a = lg.array(npa)
    assert np.array_equal(lg.sqrt(a), np.sqrt(npa))
    return


if __name__ == "__main__":
    test()