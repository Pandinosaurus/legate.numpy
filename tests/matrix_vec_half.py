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

    np.random.seed(42)
    An = np.random.randn(3, 7).astype(np.float16)
    Bn = np.random.randn(7).astype(np.float16)
    Cn = An.dot(Bn)

    A = lg.array(An)
    B = lg.array(Bn)
    C = A.dot(B)

    # print(C)
    # print(Cn)
    assert np.allclose(C, Cn)

    np.random.seed(42)
    An = np.random.randn(3, 7).astype(np.float16)
    Bn = np.random.randn(3).astype(np.float16)
    Cn = An.transpose().dot(Bn)

    A = lg.array(An)
    B = lg.array(Bn)
    C = A.transpose().dot(B)

    # print(C)
    # print(Cn)
    assert np.allclose(C, Cn)

    return


if __name__ == "__main__":
    test()
