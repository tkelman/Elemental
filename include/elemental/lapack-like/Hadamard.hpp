/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_LAPACK_HADAMARD_HPP
#define ELEM_LAPACK_HADAMARD_HPP

//
// C(i,j) := A(i,j) B(i,j)
//

namespace elem {

template<typename T> 
inline void Hadamard( const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C )
{
#ifndef RELEASE
    CallStackEntry entry("Hadamard");
#endif
    if( A.Height() != B.Height() || A.Width() != B.Width() )
        LogicError("Hadamard product requires equal dimensions");
    C.ResizeTo( A.Height(), A.Width() );

    const Int height = A.Height();
    const Int width = A.Width();
    for( Int j=0; j<width; ++j )
        for( Int i=0; i<height; ++i )
            C.Set( i, j, A.Get(i,j)*B.Get(i,j) );
}

template<typename T,Distribution U,Distribution V> 
inline void Hadamard
( const DistMatrix<T,U,V>& A, const DistMatrix<T,U,V>& B, DistMatrix<T,U,V>& C )
{
#ifndef RELEASE
    CallStackEntry entry("Hadamard");
#endif
    if( A.Height() != B.Height() || A.Width() != B.Width() )
        LogicError("Hadamard product requires equal dimensions");
    if( A.Grid() != B.Grid() )
        LogicError("A and B must have the same grids");
    if( A.ColAlignment() != B.ColAlignment() || 
        A.RowAlignment() != B.RowAlignment() )
        LogicError("A and B must be aligned");
    const Grid& g = A.Grid();
    C.SetGrid( g );
    C.AlignWith( A );
    C.ResizeTo( A.Height(), A.Width() );

    const Int localHeight = A.LocalHeight();
    const Int localWidth = A.LocalWidth();
    for( Int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        {
            const T alpha = A.GetLocal(iLoc,jLoc); 
            const T beta = B.GetLocal(iLoc,jLoc);
            C.SetLocal( iLoc, jLoc, alpha*beta );
        }
    }
}

} // namespace elem

#endif // ifndef ELEM_LAPACK_HADAMARD_HPP
