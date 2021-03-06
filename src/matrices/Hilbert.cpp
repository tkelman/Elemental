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
void Hilbert( Matrix<F>& A, Int n )
{
    DEBUG_ONLY(CallStackEntry cse("Hilbert"))
    A.Resize( n, n );
    auto hilbertFill = []( Int i, Int j ) { return F(1)/F(i+j+1); };
    IndexDependentFill( A, std::function<F(Int,Int)>(hilbertFill) );
}

template<typename F>
void Hilbert( AbstractDistMatrix<F>& A, Int n )
{
    DEBUG_ONLY(CallStackEntry cse("Hilbert"))
    A.Resize( n, n );
    auto hilbertFill = []( Int i, Int j ) { return F(1)/F(i+j+1); };
    IndexDependentFill( A, std::function<F(Int,Int)>(hilbertFill) );
}

template<typename F>
void Hilbert( AbstractBlockDistMatrix<F>& A, Int n )
{
    DEBUG_ONLY(CallStackEntry cse("Hilbert"))
    A.Resize( n, n );
    auto hilbertFill = []( Int i, Int j ) { return F(1)/F(i+j+1); };
    IndexDependentFill( A, std::function<F(Int,Int)>(hilbertFill) );
}

#define PROTO(F) \
  template void Hilbert( Matrix<F>& A, Int n ); \
  template void Hilbert( AbstractDistMatrix<F>& A, Int n ); \
  template void Hilbert( AbstractBlockDistMatrix<F>& A, Int n );

#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
