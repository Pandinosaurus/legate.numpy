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

import random

import numpy as np

import legate.numpy as lg


def test():
    anp = np.random.randn(4, 5)
    b = random.randint(1, 13)
    a = lg.array(anp)

    # test divide with scalar on rhs
    assert np.array_equal(lg.divide(a, b), np.divide(anp, b))

    # test divide with scalar on lhs
    assert np.array_equal(lg.divide(b, a), np.divide(b, anp))

    return


if __name__ == "__main__":
    test()
