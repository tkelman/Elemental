/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_BLAS_SYR_HPP
#define ELEM_BLAS_SYR_HPP

namespace elem {

template<typename T>
inline void
Syr
( UpperOrLower uplo, T alpha, const Matrix<T>& x, Matrix<T>& A, 
  bool conjugate=false )
{
#ifndef RELEASE
    CallStackEntry entry("Syr");
    if( A.Height() != A.Width() )
        LogicError("A must be square");
    if( x.Width() != 1 && x.Height() != 1 )
        LogicError("x must be a vector");
    const Int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    if( xLength != A.Height() )
        LogicError("x must conform with A");
#endif
    const char uploChar = UpperOrLowerToChar( uplo );
    const Int m = A.Height();
    const Int incx = ( x.Width()==1 ? 1 : x.LDim() );
    if( conjugate )
    {
        blas::Her
        ( uploChar, m, alpha, x.LockedBuffer(), incx, A.Buffer(), A.LDim() );
    }
    else
    {
        blas::Syr
        ( uploChar, m, alpha, x.LockedBuffer(), incx, A.Buffer(), A.LDim() );
    }
}

template<typename T>
inline void
Syr
( UpperOrLower uplo,
  T alpha, const DistMatrix<T>& x,
                 DistMatrix<T>& A,
  bool conjugate=false )
{
#ifndef RELEASE
    CallStackEntry entry("Syr");
    if( A.Grid() != x.Grid() )
        LogicError("A and x must be distributed over the same grid");
    if( A.Height() != A.Width() )
        LogicError("A must be square");
    const Int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    if( A.Height() != xLength )
    {
        std::ostringstream msg;
        msg << "A must conform with x: \n"
            << "  A ~ " << A.Height() << " x " << A.Width() << "\n"
            << "  x ~ " << x.Height() << " x " << x.Width() << "\n";
        LogicError( msg.str() );
    }
#endif
    const Grid& g = A.Grid();

    const Int localHeight = A.LocalHeight();
    const Int localWidth = A.LocalWidth();
    const Int r = g.Height();
    const Int c = g.Width();
    const Int colShift = A.ColShift();
    const Int rowShift = A.RowShift();

    if( x.Width() == 1 )
    {
        DistMatrix<T,MC,STAR> x_MC_STAR(g);
        DistMatrix<T,MR,STAR> x_MR_STAR(g);

        x_MC_STAR.AlignWith( A );
        x_MR_STAR.AlignWith( A );
        //--------------------------------------------------------------------//
        x_MC_STAR = x;
        x_MR_STAR = x_MC_STAR;

        const T* xBuffer = x_MC_STAR.LockedBuffer();
        if( uplo == LOWER )
        {
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                const Int j = rowShift + jLoc*c;
                const Int heightAboveDiag = Length(j,colShift,r);

                const T beta = x_MR_STAR.GetLocal(jLoc,0);
                const T gamma = ( conjugate ? alpha*Conj(beta) : alpha*beta );
                T* ACol = A.Buffer(0,jLoc);
                for( Int iLoc=heightAboveDiag; iLoc<localHeight; ++iLoc )
                    ACol[iLoc] += gamma*xBuffer[iLoc];
            }
        }
        else
        {
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                const Int j = rowShift + jLoc*c;
                const Int heightToDiag = Length(j+1,colShift,r);

                const T beta = x_MR_STAR.GetLocal(jLoc,0);
                const T gamma = ( conjugate ? alpha*Conj(beta) : alpha*beta );
                T* ACol = A.Buffer(0,jLoc);
                for( Int iLoc=0; iLoc<heightToDiag; ++iLoc )
                    ACol[iLoc] += gamma*xBuffer[iLoc];
            }
        }
        //--------------------------------------------------------------------//
    }
    else
    {
        DistMatrix<T,STAR,MC> x_STAR_MC(g);
        DistMatrix<T,STAR,MR> x_STAR_MR(g);

        x_STAR_MC.AlignWith( A );
        x_STAR_MR.AlignWith( A );
        //--------------------------------------------------------------------//
        x_STAR_MR = x;
        x_STAR_MC = x_STAR_MR;

        const T* xBuffer = x_STAR_MC.LockedBuffer();
        const Int incx = x_STAR_MC.LDim();
        if( uplo == LOWER )
        {
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                const Int j = rowShift + jLoc*c;
                const Int heightAboveDiag = Length(j,colShift,r);

                const T beta = x_STAR_MR.GetLocal(0,jLoc);
                const T gamma = ( conjugate ? alpha*Conj(beta) : alpha*beta );
                T* ACol = A.Buffer(0,jLoc);
                for( Int iLoc=heightAboveDiag; iLoc<localHeight; ++iLoc )
                    ACol[iLoc] += gamma*xBuffer[iLoc*incx];
            }
        }
        else
        {
            for( Int jLoc=0; jLoc<localWidth; ++jLoc )
            {
                const Int j = rowShift + jLoc*c;
                const Int heightToDiag = Length(j+1,colShift,r);

                const T beta = x_STAR_MR.GetLocal(0,jLoc);
                const T gamma = ( conjugate ? alpha*Conj(beta) : alpha*beta );
                T* ACol = A.Buffer(0,jLoc);
                for( Int iLoc=0; iLoc<heightToDiag; ++iLoc )
                    ACol[iLoc] += gamma*xBuffer[iLoc*incx];
            }
        }
        //--------------------------------------------------------------------//
    }
}

} // namespace elem

#endif // ifndef ELEM_BLAS_SYR_HPP
