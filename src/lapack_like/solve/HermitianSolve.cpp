/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename F>
void HermitianSolve
( UpperOrLower uplo, Orientation orientation, Matrix<F>& A, Matrix<F>& B, 
  const LDLPivotCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HermitianSolve"))
    SymmetricSolve( uplo, orientation, A, B, true, ctrl );
}

template<typename F>
void HermitianSolve
( UpperOrLower uplo, Orientation orientation, 
  AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B,
  const LDLPivotCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HermitianSolve"))
    SymmetricSolve( uplo, orientation, A, B, true, ctrl );
}

#define PROTO(F) \
  template void HermitianSolve \
  ( UpperOrLower uplo, Orientation orientation, \
    Matrix<F>& A, Matrix<F>& B, const LDLPivotCtrl<Base<F>>& ctrl ); \
  template void HermitianSolve \
  ( UpperOrLower uplo, Orientation orientation, \
    AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, \
    const LDLPivotCtrl<Base<F>>& ctrl );

#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
