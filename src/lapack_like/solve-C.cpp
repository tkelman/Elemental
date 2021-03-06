/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "El.h"
using namespace El;

extern "C" {

#define C_PROTO_FIELD(SIG,SIGBASE,F) \
  /* Gaussian Elimination 
     -------------------- */ \
  ElError ElGaussianElimination_ ## SIG \
  ( ElMatrix_ ## SIG A, ElMatrix_ ## SIG B ) \
  { EL_TRY( GaussianElimination( *CReflect(A), *CReflect(B) ) ) } \
  ElError ElGaussianEliminationDist_ ## SIG \
  ( ElDistMatrix_ ## SIG A, ElDistMatrix_ ## SIG B ) \
  { EL_TRY( GaussianElimination( *CReflect(A), *CReflect(B) ) ) } \
  /* General Linear Model
     -------------------- */ \
  ElError ElGLM_ ## SIG \
  ( ElMatrix_ ## SIG A, ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG D, ElMatrix_ ## SIG Y ) \
  { EL_TRY( GLM( *CReflect(A), *CReflect(B), *CReflect(D), *CReflect(Y) ) ) } \
  ElError ElGLMDist_ ## SIG \
  ( ElDistMatrix_ ## SIG A, ElDistMatrix_ ## SIG B, \
    ElDistMatrix_ ## SIG D, ElDistMatrix_ ## SIG Y ) \
  { EL_TRY( GLM( *CReflect(A), *CReflect(B), *CReflect(D), *CReflect(Y) ) ) } \
  /* HPD solve
     --------- */ \
  ElError ElHPDSolve_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, \
    ElMatrix_ ## SIG A, ElMatrix_ ## SIG B ) \
  { EL_TRY( HPDSolve( CReflect(uplo), CReflect(orientation), \
                      *CReflect(A), *CReflect(B) ) ) } \
  ElError ElHPDSolveDist_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, \
    ElDistMatrix_ ## SIG A, ElDistMatrix_ ## SIG B ) \
  { EL_TRY( HPDSolve( CReflect(uplo), CReflect(orientation), \
                      *CReflect(A), *CReflect(B) ) ) } \
  /* Least squares
     ------------- */ \
  ElError ElLeastSquares_ ## SIG \
  ( ElOrientation orientation, ElMatrix_ ## SIG A, \
    ElConstMatrix_ ## SIG B, ElMatrix_ ## SIG X ) \
  { EL_TRY( LeastSquares( CReflect(orientation), *CReflect(A), \
                          *CReflect(B), *CReflect(X) ) ) } \
  ElError ElLeastSquaresDist_ ## SIG \
  ( ElOrientation orientation, ElDistMatrix_ ## SIG A, \
    ElConstDistMatrix_ ## SIG B, ElDistMatrix_ ## SIG X ) \
  { EL_TRY( LeastSquares( CReflect(orientation), *CReflect(A), \
                          *CReflect(B), *CReflect(X) ) ) } \
  ElError ElLeastSquaresDistSparse_ ## SIG \
  ( ElOrientation orientation, \
    ElConstDistSparseMatrix_ ## SIG A, ElConstDistMultiVec_ ## SIG X, \
    ElDistMultiVec_ ## SIG Y ) \
  { EL_TRY( LeastSquares( CReflect(orientation), \
      *CReflect(A), *CReflect(X), *CReflect(Y) ) ) } \
  /* Equality-constrained Least Squares
     ---------------------------------- */ \
  ElError ElLSE_ ## SIG \
  ( ElMatrix_ ## SIG A, ElMatrix_ ## SIG B, \
    ElMatrix_ ## SIG C, ElMatrix_ ## SIG D, ElMatrix_ ## SIG X ) \
  { EL_TRY( LSE( *CReflect(A), *CReflect(B), \
                 *CReflect(C), *CReflect(D), *CReflect(X) ) ) } \
  ElError ElLSEDist_ ## SIG \
  ( ElDistMatrix_ ## SIG A, ElDistMatrix_ ## SIG B, \
    ElDistMatrix_ ## SIG C, ElDistMatrix_ ## SIG D, ElDistMatrix_ ## SIG X ) \
  { EL_TRY( LSE( *CReflect(A), *CReflect(B), \
                 *CReflect(C), *CReflect(D), *CReflect(X) ) ) } \
  /* Multi-shift Hessenberg solve
     ---------------------------- */ \
  ElError ElMultiShiftHessSolve_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, CREFLECT(F) alpha, \
    ElConstMatrix_ ## SIG H, ElConstMatrix_ ## SIG shifts, \
    ElMatrix_ ## SIG X ) \
  { EL_TRY( MultiShiftHessSolve( \
      CReflect(uplo), CReflect(orientation), CReflect(alpha), \
      *CReflect(H), *CReflect(shifts), *CReflect(X) ) ) } \
  ElError ElMultiShiftHessSolveDist_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, CREFLECT(F) alpha, \
    ElConstDistMatrix_ ## SIG H, ElConstDistMatrix_ ## SIG shifts, \
    ElDistMatrix_ ## SIG X ) \
  { EL_TRY( MultiShiftHessSolve( \
      CReflect(uplo), CReflect(orientation), CReflect(alpha), \
      *CReflect(H), *CReflect(shifts), *CReflect(X) ) ) } \
  /* Ridge regression
     ---------------- */ \
  ElError ElRidge_ ## SIG \
  ( ElConstMatrix_ ## SIG A, ElConstMatrix_ ## SIG B, \
    Base<F> alpha, ElMatrix_ ## SIG X, ElRidgeAlg alg ) \
  { EL_TRY( Ridge( *CReflect(A), *CReflect(B), \
                   alpha, *CReflect(X), CReflect(alg) ) ) } \
  ElError ElRidgeDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, ElConstDistMatrix_ ## SIG B, \
    Base<F> alpha, ElDistMatrix_ ## SIG X, ElRidgeAlg alg ) \
  { EL_TRY( Ridge( *CReflect(A), *CReflect(B), \
                   alpha, *CReflect(X), CReflect(alg) ) ) } \
  ElError ElRidgeDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, ElConstDistMultiVec_ ## SIG X, \
    Base<F> alpha, ElDistMultiVec_ ## SIG Y ) \
  { EL_TRY( Ridge( *CReflect(A), *CReflect(X), alpha, *CReflect(Y) ) ) } \
  /* Symmetric solve
     --------------- */ \
  ElError ElSymmetricSolve_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, \
    ElMatrix_ ## SIG A, ElMatrix_ ## SIG B ) \
  { EL_TRY( SymmetricSolve( CReflect(uplo), CReflect(orientation), \
                            *CReflect(A), *CReflect(B) ) ) } \
  ElError ElSymmetricSolveDist_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, \
    ElDistMatrix_ ## SIG A, ElDistMatrix_ ## SIG B ) \
  { EL_TRY( SymmetricSolve( CReflect(uplo), CReflect(orientation), \
                            *CReflect(A), *CReflect(B) ) ) } \
  ElError ElSymmetricSolveDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, ElDistMultiVec_ ## SIG X ) \
  { EL_TRY( SymmetricSolve( *CReflect(A), *CReflect(X) ) ) } \
  /* Tikhonov regularization
     ----------------------- */ \
  ElError ElTikhonov_ ## SIG \
  ( ElConstMatrix_ ## SIG A, ElConstMatrix_ ## SIG B, \
    ElConstMatrix_ ## SIG Gamma, ElMatrix_ ## SIG X, \
    ElTikhonovAlg alg ) \
  { EL_TRY( Tikhonov( *CReflect(A), *CReflect(B), \
                      *CReflect(Gamma), *CReflect(X), CReflect(alg) ) ) } \
  ElError ElTikhonovDist_ ## SIG \
  ( ElConstDistMatrix_ ## SIG A, ElConstDistMatrix_ ## SIG B, \
    ElConstDistMatrix_ ## SIG Gamma, ElDistMatrix_ ## SIG X, \
    ElTikhonovAlg alg ) \
  { EL_TRY( Tikhonov( *CReflect(A), *CReflect(B), \
                      *CReflect(Gamma), *CReflect(X), CReflect(alg) ) ) } \
  ElError ElTikhonovDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, ElConstDistMultiVec_ ## SIG X, \
    ElConstDistSparseMatrix_ ## SIG Gamma, ElDistMultiVec_ ## SIG Y ) \
  { EL_TRY( Tikhonov( *CReflect(A), *CReflect(X), *CReflect(Gamma), \
                      *CReflect(Y) ) ) }

#define C_PROTO_REAL(SIG,F) \
  C_PROTO_FIELD(SIG,SIG,F)

#define C_PROTO_COMPLEX(SIG,SIGBASE,F) \
  C_PROTO_FIELD(SIG,SIGBASE,F) \
  /* Hermitian solve
     --------------- */ \
  ElError ElHermitianSolve_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, \
    ElMatrix_ ## SIG A, ElMatrix_ ## SIG B ) \
  { EL_TRY( HermitianSolve( CReflect(uplo), CReflect(orientation), \
                            *CReflect(A), *CReflect(B) ) ) } \
  ElError ElHermitianSolveDist_ ## SIG \
  ( ElUpperOrLower uplo, ElOrientation orientation, \
    ElDistMatrix_ ## SIG A, ElDistMatrix_ ## SIG B ) \
  { EL_TRY( HermitianSolve( CReflect(uplo), CReflect(orientation), \
                            *CReflect(A), *CReflect(B) ) ) } \
  ElError ElHermitianSolveDistSparse_ ## SIG \
  ( ElConstDistSparseMatrix_ ## SIG A, ElDistMultiVec_ ## SIG X ) \
  { EL_TRY( HermitianSolve( *CReflect(A), *CReflect(X) ) ) }

#define EL_NO_INT_PROTO
#include "El/macros/CInstantiate.h"

} // extern "C"
