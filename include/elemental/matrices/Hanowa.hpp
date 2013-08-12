/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_MATRICES_HANOWA_HPP
#define ELEM_MATRICES_HANOWA_HPP

#include "elemental/matrices/Diagonal.hpp"

namespace elem {

template<typename T>
inline void
Hanowa( Matrix<T>& A, Int n, T mu )
{
#ifndef RELEASE
    CallStackEntry entry("Hanowa");
#endif
    if( n % 2 != 0 )
        LogicError("n must be an even integer");
    A.ResizeTo( n, n );
    const Int m = n/2;
    std::vector<T> d(m);
    Matrix<T> ABlock;

    for( Int j=0; j<m; ++j )
        d[j] = mu;
    View( ABlock, A, 0, 0, m, m );
    Diagonal( ABlock, d );
    View( ABlock, A, m, m, m, m );
    Diagonal( ABlock, d );

    for( Int j=0; j<m; ++j )
        d[j] = -(j+1);
    View( ABlock, A, 0, m, m, m );
    Diagonal( ABlock, d );

    for( Int j=0; j<m; ++j )
        d[j] = j+1;
    View( ABlock, A, m, 0, m, m );
    Diagonal( ABlock, d );
}

template<typename T,Distribution U,Distribution V>
inline void
Hanowa( DistMatrix<T,U,V>& A, Int n, T mu )
{
#ifndef RELEASE
    CallStackEntry entry("Hanowa");
#endif
    if( n % 2 != 0 )
        LogicError("n must be an even integer");
    A.ResizeTo( n, n );
    const Int m = n/2;
    std::vector<T> d(m);
    DistMatrix<T,U,V> ABlock( A.Grid() );

    for( Int j=0; j<m; ++j )
        d[j] = mu;
    View( ABlock, A, 0, 0, m, m );
    Diagonal( ABlock, d );
    View( ABlock, A, m, m, m, m );
    Diagonal( ABlock, d );

    for( Int j=0; j<m; ++j )
        d[j] = -(j+1);
    View( ABlock, A, 0, m, m, m );
    Diagonal( ABlock, d );

    for( Int j=0; j<m; ++j )
        d[j] = j+1;
    View( ABlock, A, m, 0, m, m );
    Diagonal( ABlock, d );
}

// TODO: MakeHanowa?

} // namespace elem

#endif // ifndef ELEM_MATRICES_HANOWA_HPP
