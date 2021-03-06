/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_LDL_MULTIPLYAFTER_HPP
#define EL_LDL_MULTIPLYAFTER_HPP

namespace El {
namespace ldl {

template<typename F> 
void MultiplyAfter( const Matrix<F>& A, Matrix<F>& B, bool conjugated )
{
    DEBUG_ONLY(
        CallStackEntry cse("ldl::MultiplyAfter");
        if( A.Height() != A.Width() )
            LogicError("A must be square");
        if( A.Height() != B.Height() )
            LogicError("A and B must be the same height");
    )
    const Orientation orientation = ( conjugated ? ADJOINT : TRANSPOSE );
    const auto d = A.GetDiagonal();
    Trmm( LEFT, LOWER, orientation, UNIT, F(1), A, B );
    DiagonalScale( LEFT, NORMAL, d, B );
    Trmm( LEFT, LOWER, NORMAL, UNIT, F(1), A, B );
}

template<typename F> 
void MultiplyAfter
( const AbstractDistMatrix<F>& APre, AbstractDistMatrix<F>& B, bool conjugated )
{
    DEBUG_ONLY(
        CallStackEntry cse("ldl::MultiplyAfter");
        AssertSameGrids( APre, B );
        if( APre.Height() != APre.Width() )
            LogicError("A must be square");
        if( APre.Height() != B.Height() )
            LogicError("A and B must be the same height");
    )
    const Orientation orientation = ( conjugated ? ADJOINT : TRANSPOSE );

    auto APtr = ReadProxy<F,MC,MR>( &APre );
    auto& A = *APtr;
    const auto d = A.GetDiagonal();

    Trmm( LEFT, LOWER, orientation, UNIT, F(1), A, B );
    DiagonalScale( LEFT, NORMAL, d, B );
    Trmm( LEFT, LOWER, NORMAL, UNIT, F(1), A, B );
}

template<typename F> 
void MultiplyAfter
( const Matrix<F>& A, const Matrix<F>& dSub, 
  const Matrix<Int>& p, Matrix<F>& B, bool conjugated )
{
    DEBUG_ONLY(
        CallStackEntry cse("ldl::MultiplyAfter");
        if( A.Height() != A.Width() )
            LogicError("A must be square");
        if( A.Height() != B.Height() )
            LogicError("A and B must be the same height");
        if( p.Height() != A.Height() )
            LogicError("A and p must be the same height");
        // TODO: Check for dSub
    )
    const Orientation orientation = ( conjugated ? ADJOINT : TRANSPOSE );
    const auto d = A.GetDiagonal();

    Matrix<Int> pInv;
    InvertPermutation( p, pInv );
    
    PermuteRows( B, p, pInv );
    Trmm( LEFT, LOWER, orientation, UNIT, F(1), A, B );
    QuasiDiagonalScale( LEFT, LOWER, d, dSub, B, conjugated );
    Trmm( LEFT, LOWER, NORMAL, UNIT, F(1), A, B );
    PermuteRows( B, pInv, p );
}

template<typename F> 
void MultiplyAfter
( const AbstractDistMatrix<F>& APre, const AbstractDistMatrix<F>& dSub, 
  const AbstractDistMatrix<Int>& p, AbstractDistMatrix<F>& BPre, 
  bool conjugated )
{
    DEBUG_ONLY(
        CallStackEntry cse("ldl::MultiplyAfter");
        AssertSameGrids( APre, BPre, p );
        if( APre.Height() != APre.Width() )
            LogicError("A must be square");
        if( APre.Height() != BPre.Height() )
            LogicError("A and B must be the same height");
        if( APre.Height() != p.Height() )
            LogicError("A and p must be the same height");
        // TODO: Check for dSub
    )
    const Orientation orientation = ( conjugated ? ADJOINT : TRANSPOSE );
    const Grid& g = APre.Grid();

    auto APtr = ReadProxy<F,MC,MR>( &APre );      auto& A = *APtr; 
    auto BPtr = ReadWriteProxy<F,MC,MR>( &BPre ); auto& B = *BPtr;

    const auto d = A.GetDiagonal();

    DistMatrix<Int,VC,STAR> pInv(g);
    InvertPermutation( p, pInv );

    PermuteRows( B, p, pInv );
    Trmm( LEFT, LOWER, orientation, UNIT, F(1), A, B );
    QuasiDiagonalScale( LEFT, LOWER, d, dSub, B, conjugated );
    Trmm( LEFT, LOWER, NORMAL, UNIT, F(1), A, B );
    PermuteRows( B, pInv, p );
}

} // namespace ldl
} // namespace El

#endif // ifndef EL_LDL_MULTIPLYAFTER_HPP
