/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.
 
   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
using namespace El;

namespace El {

template<typename F>
void Ridge
( const DistSparseMatrix<F>& A, const DistMultiVec<F>& Y, Base<F> alpha, 
        DistMultiVec<F>& X, const BisectCtrl& ctrl )
{
    DEBUG_ONLY(
        CallStackEntry cse("Ridge");
        if( A.Height() != Y.Height() )
            LogicError("Heights of A and Y must match");
    )
    const Int m = A.Height();
    const Int n = A.Width();
    DistSparseMatrix<F> C(A.Comm());

    X.SetComm( Y.Comm() );
    Zeros( X, n, Y.Width() );
    if( m >= n )
    {
        Herk( LOWER, ADJOINT, Base<F>(1), A, C );
        UpdateDiagonal( C, F(alpha*alpha) );
        MakeHermitian( LOWER, C );

        Multiply( ADJOINT, F(1), A, Y, F(0), X );
        HermitianSolve( C, X, ctrl );
    }
    else
    {
        Herk( LOWER, NORMAL, Base<F>(1), A, C );
        UpdateDiagonal( C, F(alpha*alpha) );
        MakeHermitian( LOWER, C );

        DistMultiVec<F> YCopy(Y.Comm());
        YCopy = Y;
        HermitianSolve( C, YCopy, ctrl );
        Multiply( ADJOINT, F(1), A, YCopy, F(0), X );
    }
}

#define PROTO(F) \
  template void Ridge \
  ( const DistSparseMatrix<F>& A, const DistMultiVec<F>& Y, Base<F> alpha, \
    DistMultiVec<F>& X, const BisectCtrl& ctrl );
 
#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
