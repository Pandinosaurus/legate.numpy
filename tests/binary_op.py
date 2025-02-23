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

from __future__ import division

import numpy as np

import legate.numpy as lg


def test():
    numpyX = np.array([1, 2, 3])
    numpyY = np.array([4, 5, 6])

    x = lg.array(numpyX)
    y = lg.array(numpyY)

    z = x + y
    # # print(z)
    assert np.array_equal(z, numpyX + numpyY)

    z = x + 2
    # print(z)
    assert np.array_equal(z, numpyX + 2)

    z = 2 + x
    # print(z)
    assert np.array_equal(z, 2 + numpyX)

    z = x - y
    # print(z)
    assert np.array_equal(z, numpyX - numpyY)

    z = x - 2
    # print(z)
    assert np.array_equal(z, numpyX - 2)

    z = 2 - x
    # print(z)
    assert np.array_equal(z, 2 - numpyX)

    z = x / y
    # print(z)
    assert np.array_equal(z, numpyX / numpyY)

    z = x / 2
    # print(z)
    assert np.array_equal(z, numpyX / 2)

    z = 2 / x
    # print(z)
    assert np.array_equal(z, 2 / numpyX)

    z = x * y
    # print(z)
    assert np.array_equal(z, numpyX * numpyY)

    z = x * 2
    # print(z)
    assert np.array_equal(z, numpyX * 2)

    z = 2 * x
    # print(z)
    assert np.array_equal(z, 2 * numpyX)

    z = x ** 5
    # print(z)
    assert np.array_equal(z, numpyX ** 5)

    return


if __name__ == "__main__":
    test()
