/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ELEM_LAPACK_BIDIAG_UUNB_HPP
#define ELEM_LAPACK_BIDIAG_UUNB_HPP

#include "elemental/blas-like/level1/Conjugate.hpp"
#include "elemental/blas-like/level2/Gemv.hpp"
#include "elemental/blas-like/level2/Ger.hpp"
#include "elemental/lapack-like/Reflector.hpp"
#include "elemental/matrices/Zeros.hpp"

namespace elem {
namespace bidiag {

template<typename F> 
inline void UUnb
( DistMatrix<F>& A, DistMatrix<F,MD,STAR>& tP, DistMatrix<F,MD,STAR>& tQ )
{
#ifndef RELEASE
    CallStackEntry entry("bidiag::UUnb");
    if( A.Grid() != tP.Grid() || tP.Grid() != tQ.Grid() )
        LogicError("Process grids do not match");
    if( A.Height() < A.Width() )
        LogicError("A must be at least as tall as it is wide");
#endif
    const Grid& g = A.Grid();
    const Int tPHeight = std::max(A.Width()-1,0);
    const Int tQHeight = A.Width();
    tP.ResizeTo( tPHeight, 1 );
    tQ.ResizeTo( tQHeight, 1 );

    // Matrix views 
    DistMatrix<F>
        ATL(g), ATR(g),  A00(g), a01(g),     A02(g),  alpha12L(g), a12R(g),
        ABL(g), ABR(g),  a10(g), alpha11(g), a12(g),  aB1(g), AB2(g),
                         A20(g), a21(g),     A22(g);

    // Temporary matrices
    DistMatrix<F,STAR,MR  > a12_STAR_MR(g);
    DistMatrix<F,MC,  STAR> aB1_MC_STAR(g);
    DistMatrix<F,MR,  STAR> x12Adj_MR_STAR(g);
    DistMatrix<F,MC,  STAR> w21_MC_STAR(g);

    PartitionDownDiagonal
    ( A, ATL, ATR,
         ABL, ABR, 0 );
    while( ATL.Width() < A.Width() )
    {
        RepartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, /**/ a01,     A02,
         /*************/ /**********************/
               /**/       a10, /**/ alpha11, a12,
          ABL, /**/ ABR,  A20, /**/ a21,     A22, 1 );

        View2x1
        ( aB1, alpha11,
               a21 );
        View2x1
        ( AB2, a12,
               A22 );

        aB1_MC_STAR.AlignWith( aB1 );
        a12_STAR_MR.AlignWith( a12 );
        x12Adj_MR_STAR.AlignWith( AB2 );
        w21_MC_STAR.AlignWith( A22 );

        const bool thisIsMyRow = ( g.Row() == alpha11.ColAlignment() );
        const bool thisIsMyCol = ( g.Col() == alpha11.RowAlignment() );
        const bool nextIsMyCol = ( g.Col() == a12.RowAlignment() );
        //--------------------------------------------------------------------//

        // Find tauQ, u, and epsilonQ such that
        //     I - conj(tauQ) | 1 | | 1, u^H | | alpha11 | = | epsilonQ |
        //                    | u |            |    a21  |   |    0     |
        const F tauQ = Reflector( alpha11, a21 );
        tQ.Set(A00.Height(),0,tauQ );
        F epsilonQ=0;
        if( thisIsMyCol && thisIsMyRow )
            epsilonQ = alpha11.GetLocal(0,0);

        // Set aB1 = | 1 | and form x12^H := (aB1^H AB2)^H = AB2^H aB1
        //           | u |
        alpha11.Set(0,0,F(1));
        aB1_MC_STAR = aB1;
        Zeros( x12Adj_MR_STAR, a12.Width(), 1 );
        LocalGemv( ADJOINT, F(1), AB2, aB1_MC_STAR, F(0), x12Adj_MR_STAR );
        x12Adj_MR_STAR.SumOverCol();

        // Update AB2 := AB2 - conj(tauQ) aB1 x12
        //             = AB2 - conj(tauQ) aB1 aB1^H AB2 
        //             = (I - conj(tauQ) aB1 aB1^H) AB2
        LocalGer( -Conj(tauQ), aB1_MC_STAR, x12Adj_MR_STAR, AB2 );

        // Put epsilonQ back instead of the temporary value, 1
        if( thisIsMyCol && thisIsMyRow )
            alpha11.SetLocal(0,0,epsilonQ);

        if( A22.Width() != 0 )
        {
            // Due to the deficiencies in the BLAS ?gemv routines, this section
            // is easier if we temporarily conjugate a12
            Conjugate( a12 ); 

            // Expose the subvector we seek to zero, a12R
            PartitionRight( a12, alpha12L, a12R, 1 );

            // Find tauP, v, and epsilonP such that
            //     I - conj(tauP) | 1 | | 1, v^H | | alpha12L | = | epsilonP |
            //                    | v |            |  a12R^T  |   |    0     |
            const F tauP = Reflector( alpha12L, a12R );
            tP.Set(A00.Height(),0,tauP);
            F epsilonP=0;
            if( nextIsMyCol && thisIsMyRow )
                epsilonP = alpha12L.GetLocal(0,0);

            // Set a12^T = | 1 | and form w21 := A22 a12^T = A22 | 1 |
            //             | v |                                 | v |
            alpha12L.Set(0,0,F(1));
            a12_STAR_MR = a12;
            Zeros( w21_MC_STAR, a21.Height(), 1 );
            LocalGemv( NORMAL, F(1), A22, a12_STAR_MR, F(0), w21_MC_STAR );
            w21_MC_STAR.SumOverRow();

            // A22 := A22 - tauP w21 conj(a12)
            //      = A22 - tauP A22 a12^T conj(a12)
            //      = A22 (I - tauP a12^T conj(a12))
            //      = A22 conj(I - conj(tauP) a12^H a12)
            // which compensates for the fact that the reflector was generated
            // on the conjugated a12.
            LocalGer( -tauP, w21_MC_STAR, a12_STAR_MR, A22 );

            // Put epsilonP back instead of the temporary value, 1
            if( nextIsMyCol && thisIsMyRow )
                alpha12L.SetLocal(0,0,epsilonP);

            // Undue the temporary conjugation
            Conjugate( a12 );
        }
        //--------------------------------------------------------------------//

        SlidePartitionDownDiagonal
        ( ATL, /**/ ATR,  A00, a01,     /**/ A02,
               /**/       a10, alpha11, /**/ a12,
         /*************/ /**********************/
          ABL, /**/ ABR,  A20, a21,     /**/ A22 );
    }
}

} // namespace bidiag
} // namespace elem

#endif // ifndef ELEM_LAPACK_BIDIAG_UNBLOCKEDU_HPP
