/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {
namespace lin_prog {

// These implementations are adaptations of the solver described at
//    http://www.stanford.edu/~boyd/papers/admm/linprog/linprog.html
// which is derived from the distributed ADMM article of Boyd et al.
//
// This ADMM attempts to solve the following linear program:
//     minimize    c^T x
//     subject to  A x = b, x >= 0
//

template<typename Real>
Int ADMM
( const Matrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c, 
  Matrix<Real>& z,
  const ADMMCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("lin_prog::ADMM"))
    if( IsComplex<Real>::val ) 
        LogicError("The datatype was assumed to be real");
    
    // Cache a custom partially-pivoted LU factorization of 
    //    |  rho*I   A^H | = | B11  B12 |
    //    |  A       0   |   | B21  B22 |
    // by (justifiably) avoiding pivoting in the first n steps of
    // the factorization, so that
    //    [I,rho*I] = lu(rho*I).
    // The factorization would then proceed with 
    //    B21 := B21 U11^{-1} = A (rho*I)^{-1} = A/rho
    //    B12 := L11^{-1} B12 = I A^H = A^H.
    // The Schur complement would then be
    //    B22 := B22 - B21 B12 = 0 - (A*A^H)/rho.
    // We then factor said matrix with LU with partial pivoting and
    // swap the necessary rows of B21 in order to implicitly commute
    // the row pivots with the Gauss transforms in the manner standard
    // for GEPP. Unless A A' is singular, pivoting should not be needed,
    // as Cholesky factorization of the negative matrix should be valid.
    //
    // The result is the factorization
    //   | I 0   | | rho*I A^H | = | I   0   | | rho*I U12 |,
    //   | 0 P22 | | A     0   |   | L21 L22 | | 0     U22 |
    // where [L22,U22] are stored within B22.
    Matrix<Real> U12, L21, B22, bPiv;
    Adjoint( A, U12 );
    L21 = A; Scale( 1/ctrl.rho, L21 );
    Herk( LOWER, NORMAL, -1/ctrl.rho, A, B22 );
    MakeHermitian( LOWER, B22 );
    // TODO: Replace with sparse-direct Cholesky version?
    Matrix<Int> perm2;
    LU( B22, perm2 );
    PermuteRows( L21, perm2 );
    bPiv = b;
    PermuteRows( bPiv, perm2 );

    // Possibly form the inverse of L22 U22
    Matrix<Real> X22;
    if( ctrl.inv )
    {
        X22 = B22;
        MakeTrapezoidal( LOWER, X22 );
        SetDiagonal( X22, Real(1) );
        TriangularInverse( LOWER, UNIT, X22 );
        Trsm( LEFT, UPPER, NORMAL, NON_UNIT, Real(1), B22, X22 );
    }

    Int numIter=0;
    const Int m = A.Height();
    const Int n = A.Width();
    Matrix<Real> g, xTmp, y, t;
    Zeros( g, m+n, 1 );
    PartitionDown( g, xTmp, y, n );
    Matrix<Real> x, u, zOld, xHat;
    Zeros( z, n, 1 );
    Zeros( u, n, 1 );
    Zeros( t, n, 1 );
    while( numIter < ctrl.maxIter )
    {
        zOld = z;

        // Find x from
        //  | rho*I  A^H | | x | = | rho*(z-u)-c | 
        //  | A      0   | | y |   | b           |
        // via our cached custom factorization:
        // 
        // |x| = inv(U) inv(L) P' |rho*(z-u)-c|
        // |y|                    |b          |
        //     = |rho*I U12|^{-1} |I   0  | |I 0   | |rho*(z-u)-c|
        //     = |0     U22|      |L21 L22| |0 P22'| |b          |
        //     = "                        " |rho*(z-u)-c|
        //                                  | P22' b    |
        xTmp = z;
        Axpy( Real(-1), u, xTmp );
        Scale( ctrl.rho, xTmp );
        Axpy( Real(-1), c, xTmp );
        y = bPiv;
        Gemv( NORMAL, Real(-1), L21, xTmp, Real(1), y );
        if( ctrl.inv )
        {
            Gemv( NORMAL, Real(1), X22, y, t );
            y = t;
        }
        else
        {
            Trsv( LOWER, NORMAL, UNIT, B22, y );
            Trsv( UPPER, NORMAL, NON_UNIT, B22, y );
        }
        Gemv( NORMAL, Real(-1), U12, y, Real(1), xTmp );
        Scale( 1/ctrl.rho, xTmp );

        // xHat := alpha*x + (1-alpha)*zOld
        xHat = xTmp;
        Scale( ctrl.alpha, xHat );
        Axpy( 1-ctrl.alpha, zOld, xHat );

        // z := pos(xHat+u)
        z = xHat;
        Axpy( Real(1), u, z );
        LowerClip( z, Real(0) );

        // u := u + (xHat-z)
        Axpy( Real(1),  xHat, u );
        Axpy( Real(-1), z,    u );

        const Real objective = Dot( c, xTmp );

        // rNorm := || x - z ||_2
        t = xTmp;
        Axpy( Real(-1), z, t );
        const Real rNorm = FrobeniusNorm( t );
        // sNorm := |rho| || z - zOld ||_2
        t = z;
        Axpy( Real(-1), zOld, t );
        const Real sNorm = Abs(ctrl.rho)*FrobeniusNorm( t );

        const Real epsPri = Sqrt(Real(n))*ctrl.absTol +
            ctrl.relTol*Max(FrobeniusNorm(xTmp),FrobeniusNorm(z));
        const Real epsDual = Sqrt(Real(n))*ctrl.absTol +
            ctrl.relTol*Abs(ctrl.rho)*FrobeniusNorm(u);

        if( ctrl.print )
        {
            t = xTmp;
            LowerClip( t, Real(0) );
            Axpy( Real(-1), xTmp, t );
            const Real clipDist = FrobeniusNorm( t );
            std::cout << numIter << ": "
              << "||x-z||_2=" << rNorm << ", "
              << "epsPri=" << epsPri << ", "
              << "|rho| ||z-zOld||_2=" << sNorm << ", "
              << "epsDual=" << epsDual << ", "
              << "||x-Pos(x)||_2=" << clipDist << ", "
              << "c'x=" << objective << std::endl;
        }
        if( rNorm < epsPri && sNorm < epsDual )
            break;
        ++numIter;
    }
    if( ctrl.maxIter == numIter )
        std::cout << "ADMM failed to converge" << std::endl;
    x = xTmp;
    return numIter;
}

template<typename Real>
Int ADMM
( const AbstractDistMatrix<Real>& APre, const AbstractDistMatrix<Real>& bPre,
  const AbstractDistMatrix<Real>& cPre,       AbstractDistMatrix<Real>& zPre, 
  const ADMMCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("lin_prog::ADMM"))

    auto APtr = ReadProxy<Real,MC,MR>( &APre );  auto& A = *APtr;
    auto bPtr = ReadProxy<Real,MC,MR>( &bPre );  auto& b = *bPtr;
    auto cPtr = ReadProxy<Real,MC,MR>( &cPre );  auto& c = *cPtr;
    auto zPtr = WriteProxy<Real,MC,MR>( &zPre ); auto& z = *zPtr;

    if( IsComplex<Real>::val ) 
        LogicError("The datatype was assumed to be real");
    
    // Cache a custom partially-pivoted LU factorization of 
    //    |  rho*I   A^H | = | B11  B12 |
    //    |  A       0   |   | B21  B22 |
    // by (justifiably) avoiding pivoting in the first n steps of
    // the factorization, so that
    //    [I,rho*I] = lu(rho*I).
    // The factorization would then proceed with 
    //    B21 := B21 U11^{-1} = A (rho*I)^{-1} = A/rho
    //    B12 := L11^{-1} B12 = I A^H = A^H.
    // The Schur complement would then be
    //    B22 := B22 - B21 B12 = 0 - (A*A^H)/rho.
    // We then factor said matrix with LU with partial pivoting and
    // swap the necessary rows of B21 in order to implicitly commute
    // the row pivots with the Gauss transforms in the manner standard
    // for GEPP. Unless A A' is singular, pivoting should not be needed,
    // as Cholesky factorization of the negative matrix should be valid.
    //
    // The result is the factorization
    //   | I 0   | | rho*I A^H | = | I   0   | | rho*I U12 |,
    //   | 0 P22 | | A     0   |   | L21 L22 | | 0     U22 |
    // where [L22,U22] are stored within B22.
    const Int m = A.Height();
    const Int n = A.Width();
    const Grid& grid = A.Grid();
    DistMatrix<Real> U12(grid), L21(grid), B22(grid), bPiv(grid);
    U12.Align( 0,                 n%U12.RowStride() );
    L21.Align( n%L21.ColStride(), 0                 );
    B22.Align( n%B22.ColStride(), n%B22.RowStride() );
    Adjoint( A, U12 );
    L21 = A; Scale( 1/ctrl.rho, L21 );
    Herk( LOWER, NORMAL, -1/ctrl.rho, A, B22 );
    MakeHermitian( LOWER, B22 );
    DistMatrix<Int,VC,STAR> perm2(grid);
    LU( B22, perm2 );
    PermuteRows( L21, perm2 );
    bPiv = b;
    PermuteRows( bPiv, perm2 );

    // Possibly form the inverse of L22 U22
    DistMatrix<Real> X22(grid);
    if( ctrl.inv )
    {
        X22 = B22;
        MakeTrapezoidal( LOWER, X22 );
        SetDiagonal( X22, Real(1) );
        TriangularInverse( LOWER, UNIT, X22 );
        Trsm( LEFT, UPPER, NORMAL, NON_UNIT, Real(1), B22, X22 );
    }

    Int numIter=0;
    DistMatrix<Real> g(grid), xTmp(grid), y(grid), t(grid);
    Zeros( g, m+n, 1 );
    PartitionDown( g, xTmp, y, n );
    DistMatrix<Real> x(grid), u(grid), zOld(grid), xHat(grid);
    Zeros( z, n, 1 );
    Zeros( u, n, 1 );
    Zeros( t, n, 1 );
    while( numIter < ctrl.maxIter )
    {
        zOld = z;

        // Find x from
        //  | rho*I  A^H | | x | = | rho*(z-u)-c | 
        //  | A      0   | | y |   | b           |
        // via our cached custom factorization:
        // 
        // |x| = inv(U) inv(L) P' |rho*(z-u)-c|
        // |y|                    |b          |
        //     = |rho*I U12|^{-1} |I   0  | |I 0   | |rho*(z-u)-c|
        //     = |0     U22|      |L21 L22| |0 P22'| |b          |
        //     = "                        " |rho*(z-u)-c|
        //                                  | P22' b    |
        xTmp = z;
        Axpy( Real(-1), u, xTmp );
        Scale( ctrl.rho, xTmp );
        Axpy( Real(-1), c, xTmp );
        y = bPiv;
        Gemv( NORMAL, Real(-1), L21, xTmp, Real(1), y );
        if( ctrl.inv )
        {
            Gemv( NORMAL, Real(1), X22, y, t );
            y = t;
        }
        else
        {
            Trsv( LOWER, NORMAL, UNIT, B22, y );
            Trsv( UPPER, NORMAL, NON_UNIT, B22, y );
        }
        Gemv( NORMAL, Real(-1), U12, y, Real(1), xTmp );
        Scale( 1/ctrl.rho, xTmp );

        // xHat := alpha*x + (1-alpha)*zOld
        xHat = xTmp;
        Scale( ctrl.alpha, xHat );
        Axpy( 1-ctrl.alpha, zOld, xHat );

        // z := pos(xHat+u)
        z = xHat;
        Axpy( Real(1), u, z );
        LowerClip( z, Real(0) );

        // u := u + (xHat-z)
        Axpy( Real(1),  xHat, u );
        Axpy( Real(-1), z,    u );

        const Real objective = Dot( c, xTmp );

        // rNorm := || x - z ||_2
        t = xTmp;
        Axpy( Real(-1), z, t );
        const Real rNorm = FrobeniusNorm( t );
        // sNorm := |rho| || z - zOld ||_2
        t = z;
        Axpy( Real(-1), zOld, t );
        const Real sNorm = Abs(ctrl.rho)*FrobeniusNorm( t );

        const Real epsPri = Sqrt(Real(n))*ctrl.absTol +
            ctrl.relTol*Max(FrobeniusNorm(xTmp),FrobeniusNorm(z));
        const Real epsDual = Sqrt(Real(n))*ctrl.absTol +
            ctrl.relTol*Abs(ctrl.rho)*FrobeniusNorm(u);

        if( ctrl.print )
        {
            t = xTmp;
            LowerClip( t, Real(0) );
            Axpy( Real(-1), xTmp, t );
            const Real clipDist = FrobeniusNorm( t );
            if( grid.Rank() == 0 )
                std::cout << numIter << ": "
                  << "||x-z||_2=" << rNorm << ", "
                  << "epsPri=" << epsPri << ", "
                  << "|rho| ||z-zOld||_2=" << sNorm << ", "
                  << "epsDual=" << epsDual << ", "
                  << "||x-Pos(x)||_2=" << clipDist << ", "
                  << "c'x=" << objective << std::endl;
        }
        if( rNorm < epsPri && sNorm < epsDual )
            break;
        ++numIter;
    }
    if( ctrl.maxIter == numIter && grid.Rank() == 0 )
        std::cout << "ADMM failed to converge" << std::endl;
    x = xTmp;
    return numIter;
}

#define PROTO(Real) \
  template Int ADMM \
  ( const Matrix<Real>& A, const Matrix<Real>& b, const Matrix<Real>& c, \
    Matrix<Real>& z, \
    const ADMMCtrl<Real>& ctrl ); \
  template Int ADMM \
  ( const AbstractDistMatrix<Real>& A, const AbstractDistMatrix<Real>& b, \
    const AbstractDistMatrix<Real>& c,       AbstractDistMatrix<Real>& z, \
    const ADMMCtrl<Real>& ctrl );

#define EL_NO_INT_PROTO
#define EL_NO_COMPLEX_PROTO
#include "El/macros/Instantiate.h"

} // namespace lin_prog
} // namespace El
