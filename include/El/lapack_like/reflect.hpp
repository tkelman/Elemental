/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_REFLECT_HPP
#define EL_REFLECT_HPP

namespace El {

// ApplyPackedReflectors
// =====================
template<typename F>
void ApplyPackedReflectors
( LeftOrRight side, UpperOrLower uplo,
  VerticalOrHorizontal dir, ForwardOrBackward order,
  Conjugation conjugation,
  Int offset, const Matrix<F>& H, const Matrix<F>& t, Matrix<F>& A );
template<typename F>
void ApplyPackedReflectors
( LeftOrRight side, UpperOrLower uplo,
  VerticalOrHorizontal dir, ForwardOrBackward order,
  Conjugation conjugation, Int offset,
  const AbstractDistMatrix<F>& H, const AbstractDistMatrix<F>& t, 
        AbstractDistMatrix<F>& A );

// ExpandPackedReflectors
// ======================
template<typename F>
void ExpandPackedReflectors
( UpperOrLower uplo, VerticalOrHorizontal dir, Conjugation conjugation,
  Int offset, Matrix<F>& H, const Matrix<F>& t );
template<typename F>
void ExpandPackedReflectors
( UpperOrLower uplo, VerticalOrHorizontal dir, Conjugation conjugation,
  Int offset, AbstractDistMatrix<F>& H, const AbstractDistMatrix<F>& t );

// HyperbolicReflector
// ===================
template<typename F>
F LeftHyperbolicReflector( F& chi, Matrix<F>& x );
template<typename F>
F LeftHyperbolicReflector( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F LeftHyperbolicReflector( Matrix<F>& chi, Matrix<F>& x );
template<typename F>
F LeftHyperbolicReflector
( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );

template<typename F>
F RightHyperbolicReflector( F& chi, Matrix<F>& x );
template<typename F>
F RightHyperbolicReflector( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F RightHyperbolicReflector( Matrix<F>& chi, Matrix<F>& x );
template<typename F>
F RightHyperbolicReflector
( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );

namespace hyp_reflector {

template<typename F>
F Col( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F Col( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );
template<typename F>
F Row( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F Row( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );

} // namespace reflector

// Reflector
// =========
template<typename F>
F LeftReflector( F& chi, Matrix<F>& x );
template<typename F>
F LeftReflector( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F LeftReflector( Matrix<F>& chi, Matrix<F>& x );
template<typename F>
F LeftReflector( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );

template<typename F>
F RightReflector( F& chi, Matrix<F>& x );
template<typename F>
F RightReflector( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F RightReflector( Matrix<F>& chi, Matrix<F>& x );
template<typename F>
F RightReflector( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );

namespace reflector {

template<typename F>
F Col( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F Col( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );
template<typename F>
F Row( F& chi, AbstractDistMatrix<F>& x );
template<typename F>
F Row( AbstractDistMatrix<F>& chi, AbstractDistMatrix<F>& x );

} // namespace reflector

} // namespace El

#endif // ifndef EL_REFLECT_HPP
