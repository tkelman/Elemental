/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_MATRICES_TOEPLITZ_HPP
#define ELEM_MATRICES_TOEPLITZ_HPP

namespace elem {

template<typename T> 
inline void
Toeplitz( Matrix<T>& A, Int m, Int n, const std::vector<T>& a )
{
#ifndef RELEASE
    CallStackEntry entry("Toeplitz");
#endif
    const Int length = m+n-1;
    if( a.size() != Unsigned(length) )
        LogicError("a was the wrong size");
    A.ResizeTo( m, n );

    for( Int j=0; j<n; ++j )
        for( Int i=0; i<m; ++i )
            A.Set( i, j, a[i-j+(n-1)] );
}

template<typename T,Distribution U,Distribution V>
inline void
Toeplitz( DistMatrix<T,U,V>& A, Int m, Int n, const std::vector<T>& a )
{
#ifndef RELEASE
    CallStackEntry entry("Toeplitz");
#endif
    const Int length = m+n-1;
    if( a.size() != Unsigned(length) )
        LogicError("a was the wrong size");
    A.ResizeTo( m, n );

    const Int localHeight = A.LocalHeight();
    const Int localWidth = A.LocalWidth();
    const Int colShift = A.ColShift();
    const Int rowShift = A.RowShift();
    const Int colStride = A.ColStride();
    const Int rowStride = A.RowStride();
    for( Int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        const Int j = rowShift + jLoc*rowStride;
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        {
            const Int i = colShift + iLoc*colStride;
            A.SetLocal( iLoc, jLoc, a[i-j+(n-1)] );
        }
    }
}

} // namespace elem

#endif // ifndef ELEM_MATRICES_TOEPLITZ_HPP
