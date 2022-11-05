// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================

#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"


REGISTER_OP("RttAssign")
  .Input("refv: Ref(string)")
  .Input("value: string")
  .Output("out: Ref(string)")
  .Attr("validate_shape: bool = false")
  .Attr("use_locking: bool = true");


REGISTER_OP("RttAssignSub")
    .Input("ref: Ref(string)")
    .Input("value: string")
    .Output("output_ref: Ref(string)")
    .Attr("use_locking: bool = false")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::MergeBothInputsShapeFn)
#endif
;

