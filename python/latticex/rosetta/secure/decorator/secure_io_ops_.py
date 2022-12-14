# ==============================================================================
# Copyright 2020 The LatticeX Foundation
# This file is part of the Rosetta library.
#
# The Rosetta library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Rosetta library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="
import tensorflow as tf
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import ops
from latticex.rosetta.secure.decorator.secure_base_ import _secure_ops
from latticex.rosetta.secure.decorator.secure_base_ import _encode_party_id


# -----------------------------
# Secure arithmetic IO ops
# -----------------------------

def PrivateInput(x, data_owner, name=None):
    data_owner = _encode_party_id([data_owner])
    return _secure_ops.private_input(x, data_owner, name=name)


# export SecureInput name
SecureInput = PrivateInput


