/*
   This file is part of elemental, a library for distributed-memory dense 
   linear algebra.

   Copyright (C) 2009-2010 Jack Poulson <jack.poulson@gmail.com>

   This program is released under the terms of the license contained in the 
   file LICENSE.
*/
#ifndef ELEMENTAL_BLAS_HPP
#define ELEMENTAL_BLAS_HPP 1

#include "elemental/dist_matrix.hpp"
#include "elemental/partitioning.hpp"
#include "elemental/wrappers/blas.hpp"

namespace elemental {
namespace blas {

//--------------------------------------------------------------------//
// Local BLAS: Level 1                                                //
//--------------------------------------------------------------------//

// AXPY: Y := Alpha X Plus Y 
template<typename T>
void
Axpy( T alpha, const Matrix<T>& X, Matrix<T>& Y );

// COPY: Copy
template<typename T>
void
Copy( const Matrix<T>& X, Matrix<T>& Y );

// DOT: alpha := conj(x)^T * y
// 
// Though the standard BLAS interface only defines DOT for real 
// datatypes, it is naturally generalized to an inner product over the
// complex field. Recall that the conjugate symmetry of inner products 
// requires that (x,y) = conj(y,x), so that (x,x) = conj( (x,x) ) => 
// (x,x) is real. This requires that we choose (x,x) = conj(x)^T * x.
template<typename T>
T
Dot( const Matrix<T>& x, const Matrix<T>& y );

// DOTC: alpha := conj(x)^T * y
//
// This is the sister routine to DOT; while DOT is originally defined 
// only over the reals, DOTC was defined only over the complex field. 
// They are each others' extensions, and so, to us, they are 
// identical.
template<typename T>
T
Dotc( const Matrix<T>& x, const Matrix<T>& y );

// DOTU: alpha := x^T * y
//
// Standard BLAS defines DOTU for complex datatypes, but the operation
// is perfectly valid over the reals (clearly), so we extend it.
template<typename T>
T
Dotu( const Matrix<T>& x, const Matrix<T>& y );

// NRM2: NoRM 2 (Euclidean norm)
template<typename R>
R
Nrm2( const Matrix<R>& x ); 

#ifndef WITHOUT_COMPLEX
template<typename R>
R
Nrm2( const Matrix< std::complex<R> >& x );
#endif

// SCAL: SCALe X by alpha
template<typename T>
void
Scal( T alpha, Matrix<T>& X );
        
//--------------------------------------------------------------------//
// Local BLAS: Level 1 (extensions)                                   //
//--------------------------------------------------------------------//

// CONJ: CONJugate in-place
//
// There are two implementations because, for real datatypes, Conj is
// a no-op. Partial specialization of function templates is not allowed,
// so we must have two declarations.
template<typename R>
void
Conj( Matrix<R>& A );

#ifndef WITHOUT_COMPLEX
template<typename R>
void
Conj( Matrix< std::complex<R> >& A );
#endif

// CONJ: CONJugated copy
template<typename T>
void
Conj( const Matrix<T>& A, Matrix<T>& B );

// CONJTRANS: CONJugated Transposed copy
template<typename T>
void
ConjTrans( const Matrix<T>& A, Matrix<T>& B );

// TRANS: TRANSposed copy
template<typename T>
void
Trans( const Matrix<T>& A, Matrix<T>& B );

//--------------------------------------------------------------------//
// Local BLAS: Level 2                                                //
//--------------------------------------------------------------------//

// GEMV: GEneral Matrix-Vector multiply
template<typename T>
void
Gemv( Orientation orientation, 
      T alpha, const Matrix<T>& A, const Matrix<T>& x, T beta, Matrix<T>& y );

// GER: GEneral Rank-one update
//
// For complex datatypes it routes to Gerc, as x (tensor product) y 
// is x * conj(y)^T. That is, the dual of y is its conjugate transpose
// thanks to the Riesz map. Thus our generalized Ger is equivalent to
// our generalized Gerc.
template<typename T>
void
Ger( T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A );

// GERC: GEneral Rank-one Conjugated update
template<typename T>
void
Gerc( T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A );

// GERU: GEneral Rank-one Unconjugated update
template<typename T>
void
Geru( T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A );

// HEMV: HErmitian Matrix-Vector multiply
template<typename T>
void
Hemv( Shape shape,
      T alpha, const Matrix<T>& A, const Matrix<T>& x, T beta, Matrix<T>& y );

// HER: HErmitian Rank-one update
template<typename T>
void
Her( Shape shape, T alpha, const Matrix<T>& x, Matrix<T>& A );

// HER2: HErmitian Rank-2 update
template<typename T>
void
Her2( Shape shape,
      T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A );

// SYMV: SYmmetric Matrix-Vector multiply
template<typename T>
void
Symv( Shape shape,
      T alpha, const Matrix<T>& A, const Matrix<T>& x, T beta, Matrix<T>& y );

// SYR: SYmmetric Rank-one update
template<typename T>
void
Syr( Shape shape, T alpha, const Matrix<T>& x, Matrix<T>& A );

// SYR2: SYmmetric Rank-2 update
template<typename T>
void
Syr2( Shape shape,
      T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A );

// TRMV: TRiangular Matrix-Vector multiply
template<typename T>
void
Trmv
( Shape shape, Orientation orientation, Diagonal diagonal,
  const Matrix<T>& A, Matrix<T>& x );

// TRSV: TRiangular Solve with a Vector
template<typename T>
void
Trsv
( Shape shape, Orientation orientation, Diagonal diagonal,
  const Matrix<T>& A, Matrix<T>& x );

//--------------------------------------------------------------------//
// Local BLAS: Level 3                                                //
//--------------------------------------------------------------------//

// GEMM: GEneral Matrix-Matrix multiplication
template<typename T>
void
Gemm
( Orientation orientationOfA, Orientation orientationOfB,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C );

// HEMM: HErmitian Matrix-Matrix multiply
template<typename T>
void
Hemm
( Side side, Shape shape,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C );

// HER2K: HErmitian Rank-2K update
template<typename T>
void
Her2k
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C );

// HERK: HErmitian Rank-K update
template<typename T>
void
Herk
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, T beta, Matrix<T>& C );

// SYMM: SYmmetric Matrix-Matrix multiply
template<typename T>
void
Symm
( Side side, Shape shape,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C ); 

// SYR2K: SYmmetric Rank-2K update
template<typename T>
void
Syr2k
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C );

// SYRK: SYmmetric Rank-K update
template<typename T>
void
Syrk
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, T beta, Matrix<T>& C );

// TRMM: TRiangular Matrix-Matrix multiplication
template<typename T>
void
Trmm
( Side side, Shape shape, Orientation orientation, Diagonal diagonal,
  T alpha, const Matrix<T>& A, Matrix<T>& B );

// TRSM: TRiangular Solve with Multiple right-hand sides
template<typename T>
void
Trsm
( Side side, Shape shape, Orientation orientation, Diagonal diagonal,
  T alpha, const Matrix<T>& A, Matrix<T>& B ); 
        
//--------------------------------------------------------------------//
// Distributed BLAS: Level 1                                          //
//--------------------------------------------------------------------//

// AXPY: Y := Alpha X Plus Y 
template<typename T, Distribution U, Distribution V>
void
Axpy( T alpha, const DistMatrix<T,U,V>& X, DistMatrix<T,U,V>& Y );

// COPY: Copy
//
// In our case, it is just a wrapper around the '=' operator for those
// that prefer BLAS/PLAPACK syntax.
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
void
Copy( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B );

// DOT: alpha := conj(x)^T * y
// 
// Though the standard BLAS interface only defines DOT for real 
// datatypes, it is naturally generalized to an inner product over the
// complex field. Recall that the conjugate symmetry of inner products 
// requires that (x,y) = conj(y,x), so that (x,x) = conj( (x,x) ) => 
// (x,x) is real. This requires that we choose (x,x) = conj(x)^T * x.
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
T
Dot( const DistMatrix<T,U,V>& x, const DistMatrix<T,W,Z>& y );

// DOTC: alpha := conj(x)^T * y
//
// This is the sister routine to DOT; while DOT is originally defined 
// only over the reals, DOTC was defined only over the complex field. 
// They are each others' extensions, and so, to us, they are 
// identical.
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
T
Dotc( const DistMatrix<T,U,V>& x, const DistMatrix<T,W,Z>& y );

// DOTU: alpha := x^T * y
//
// Standard BLAS defines DOTU for complex datatypes, but the operation
// is perfectly valid over the reals (clearly), so we extend it.
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
T
Dotu( const DistMatrix<T,U,V>& x, const DistMatrix<T,W,Z>& y );

// NRM2: NoRM 2 (Euclidean norm)
template<typename R>
R
Nrm2( const DistMatrix<R,MC,MR>& x );

#ifndef WITHOUT_COMPLEX
template<typename R>
R
Nrm2( const DistMatrix< std::complex<R>, MC, MR >& x );
#endif

// SCAL: SCALe by a constant
template<typename T, Distribution U, Distribution V>
void
Scal
( T alpha, DistMatrix<T,U,V>& A );

//--------------------------------------------------------------------//
// Distributed BLAS: Level 1 (extensions)                             //
//--------------------------------------------------------------------//

// CONJ: CONJugate in-place
template<typename T, Distribution U, Distribution V>
void
Conj( DistMatrix<T,U,V>& A );

// CONJ: CONJugated copy
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
void
Conj( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B );

// CONJTRANS: CONJugated Transposed copy
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
void
ConjTrans( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B );

// TRANS: TRANSposed copy
template<typename T, Distribution U, Distribution V,
                     Distribution W, Distribution Z >
void
Trans( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B );

//--------------------------------------------------------------------//
// Distributed BLAS: Level 2                                          //
//--------------------------------------------------------------------//

// GEMV: GEneral Matrix-Vector multiplication
template<typename T>
void
Gemv
( Orientation orientationOfA,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& x,
  T beta,        DistMatrix<T,MC,MR>& y );

// GER: GEneral Rank-one update
//
// For complex datatypes it routes to Gerc, as x (tensor product) y 
// is x * conj(y)^T. That is, the dual of y is its conjugate transpose
// thanks to the Riesz map. Thus our generalized Ger is equivalent to
// our generalized Gerc.
template<typename T>
void
Ger
( T alpha, const DistMatrix<T,MC,MR>& x, const DistMatrix<T,MC,MR>& y,
                 DistMatrix<T,MC,MR>& A );
        
// GERC: GEneral Rank-one Conjugated update
//
// Since the extension of Ger to complex datatypes 
template<typename T>
void
Gerc
( T alpha, const DistMatrix<T,MC,MR>& x, const DistMatrix<T,MC,MR>& y,
                 DistMatrix<T,MC,MR>& A );
        
// GERU: GEneral Rank-one Unconjugated update
template<typename T>
void
Geru
( T alpha, const DistMatrix<T,MC,MR>& x, const DistMatrix<T,MC,MR>& y,
                 DistMatrix<T,MC,MR>& A );

// HEMV: HErmitian Matrix-Vector multiplication
template<typename T>
void
Hemv
( Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& x,
  T beta,        DistMatrix<T,MC,MR>& y );

// HER: HErmitian Rank-one update
template<typename T>
void
Her
( Shape shape, 
  T alpha, const DistMatrix<T,MC,MR>& x, DistMatrix<T,MC,MR>& A );

// HER: HErmitian Rank-2 update
template<typename T>
void
Her2
( Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& x, const DistMatrix<T,MC,MR>& y,
                 DistMatrix<T,MC,MR>& A );

// SYMV: SYmmetric Matrix-Vector multiplication
template<typename T>
void
Symv
( Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& x,
  T beta,        DistMatrix<T,MC,MR>& y );

// SYR: SYmmetric Rank-one update
template<typename T>
void
Syr
( Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& x, DistMatrix<T,MC,MR>& A );

// SYR2: SYmmetric Rank-2 update
template<typename T>
void
Syr2
( Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& x, const DistMatrix<T,MC,MR>& y,
                 DistMatrix<T,MC,MR>& A );

// TRMV: TRiangular Matrix-Vector multiply
template<typename T>
void
Trmv
( Shape shape, Orientation orientation, Diagonal diagonal,
  const DistMatrix<T,MC,MR>& A, DistMatrix<T,MC,MR>& x );

// TRSV: TRiangular Solve with a Vector
template<typename T>
void
Trsv
( Shape shape, Orientation orientation, Diagonal diagonal,
  const DistMatrix<T,MC,MR>& A, DistMatrix<T,MC,MR>& x );

//--------------------------------------------------------------------//
// Distributed BLAS: Level 3                                          //
//--------------------------------------------------------------------//

// GEMM: GEneral Matrix-Matrix multiplication
template<typename T>
void
Gemm
( Orientation orientationOfA, Orientation orientationOfB,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C );

// HEMM: HErmitian Matrix-Matrix multiply
template<typename T>
void
Hemm
( Side side, Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C );

// HER2K: HErmitian Rank-2K Update
template<typename T>
void
Her2k
( Shape shape, Orientation orientation,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C );

// HERK: HErmitian Rank-K Update
template<typename T>
void
Herk
( Shape shape, Orientation orientation,
  T alpha, const DistMatrix<T,MC,MR>& A, T beta, DistMatrix<T,MC,MR>& C );

// SYMM: SYmmetric Matrix-Matrix multiply
template<typename T>
void
Symm
( Side side, Shape shape,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C );

// SYR2K: SYmmetric Rank-2K Update
template<typename T>
void
Syr2k
( Shape shape, Orientation orientation,
  T alpha, const DistMatrix<T,MC,MR>& A, const DistMatrix<T,MC,MR>& B,
  T beta,        DistMatrix<T,MC,MR>& C );

// SYRK: SYmmetric Rank-K Update
template<typename T>
void
Syrk
( Shape shape, Orientation orientation,
  T alpha, const DistMatrix<T,MC,MR>& A, T beta, DistMatrix<T,MC,MR>& C );

// TRMM: TRiangular Matrix-Matrix multiplication
template<typename T>
void
Trmm
( Side side, Shape shape, Orientation orientation, Diagonal diagonal,
  T alpha, const DistMatrix<T,MC,MR>& A, DistMatrix<T,MC,MR>& B );

// TRSM: TRiangular Solve with Multiplie right-hand sides
template<typename T>
void
Trsm
( Side side, Shape shape, Orientation orientation, Diagonal diagonal,
  T alpha, const DistMatrix<T,MC,MR>& A, DistMatrix<T,MC,MR>& B );

} // blas
} // elemental

//----------------------------------------------------------------------------//
// Implementation begins here                                                 //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Local BLAS: Level 1                                                        //
//----------------------------------------------------------------------------//

template<typename T>
inline void
elemental::blas::Axpy
( T alpha, const Matrix<T>& X, Matrix<T>& Y )
{
#ifndef RELEASE
    PushCallStack("blas::Axpy");
#endif
    // If X and Y are vectors, we can allow one to be a column and the other
    // to be a row. Otherwise we force X and Y to be the same dimension.
    if( (X.Height()==1 || X.Width()==1) && (Y.Height()==1 || Y.Width()==1) )
    {
        const unsigned XLength = ( X.Width()==1 ? X.Height() : X.Width() );
#ifndef RELEASE
        const unsigned YLength = ( Y.Width()==1 ? Y.Height() : Y.Width() );
        if( XLength != YLength )
            throw "Nonconformal Axpy.";
#endif
        if( X.Width()==1 && Y.Width()==1 )
        {
            elemental::wrappers::blas::Axpy
            ( XLength, alpha, X.LockedBuffer(0,0), 1, Y.Buffer(0,0), 1 );
        }
        else if( X.Width()==1 )
        {
            elemental::wrappers::blas::Axpy
            ( XLength, alpha, X.LockedBuffer(0,0), 1, Y.Buffer(0,0), Y.LDim() );
        }
        else if( Y.Width()==1 )
        {
            elemental::wrappers::blas::Axpy
            ( XLength, alpha, X.LockedBuffer(0,0), X.LDim(), Y.Buffer(0,0), 1 );
        }
        else
        {
            elemental::wrappers::blas::Axpy
            ( XLength, alpha, 
              X.LockedBuffer(0,0), X.LDim(), Y.Buffer(0,0), Y.LDim() );
        }
    }
    else 
    {
#ifndef RELEASE
        if( X.Height() != Y.Height() || X.Width() != Y.Width() )
            throw "Nonconformal Axpy.";
#endif
        if( X.Width() <= X.Height() )
        {
            for( int j=0; j<X.Width(); ++j )
            {
                elemental::wrappers::blas::Axpy
                ( X.Height(), alpha, X.LockedBuffer(0,j), 1, Y.Buffer(0,j), 1 );
            }
        }
        else
        {
            for( int i=0; i<X.Height(); ++i )
            {
                elemental::wrappers::blas::Axpy
                ( X.Width(), alpha, X.LockedBuffer(i,0), X.LDim(),
                                    Y.Buffer(i,0),       Y.LDim() );
            }
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Copy
( const Matrix<T>& A, Matrix<T>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Copy");
#endif
    B = A;
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline T
elemental::blas::Dot
( const Matrix<T>& x, const Matrix<T>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Dot");
    if( (x.Height() != 1 && x.Width() != 1) ||
        (y.Height() != 1 && y.Width() != 1) )
        throw "Expected vector inputs.";
    int xLength = ( x.Width() == 1 ? x.Height() : x.Width() );
    int yLength = ( y.Width() == 1 ? y.Height() : y.Width() );
    if( xLength != yLength )
        throw "x and y must be the same length.";
#endif
    T dotProduct;
    if( x.Width() == 1 && y.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dot
                     ( x.Height(), x.LockedBuffer(), 1,
                                   y.LockedBuffer(), 1 );
    }
    else if( x.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dot
                     ( x.Height(), x.LockedBuffer(), 1,
                                   y.LockedBuffer(), y.LDim() );
    }
    else if( y.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dot
                     ( x.Width(), x.LockedBuffer(), x.LDim(),
                                  y.LockedBuffer(), 1        );
    }
    else
    {
        dotProduct = wrappers::blas::Dot
                     ( x.Width(), x.LockedBuffer(), x.LDim(),
                                  y.LockedBuffer(), y.LDim() );
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return dotProduct;
}

template<typename T>
inline T
elemental::blas::Dotc
( const Matrix<T>& x, const Matrix<T>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Dotc");
    if( (x.Height() != 1 && x.Width() != 1) ||
        (y.Height() != 1 && y.Width() != 1) )
        throw "Expected vector inputs.";
    int xLength = ( x.Width() == 1 ? x.Height() : x.Width() );
    int yLength = ( y.Width() == 1 ? y.Height() : y.Width() );
    if( xLength != yLength )
        throw "x and y must be the same length.";
#endif
    T dotProduct;
    if( x.Width() == 1 && y.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dotc
                     ( x.Height(), x.LockedBuffer(), 1,
                                   y.LockedBuffer(), 1 );
    }
    else if( x.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dotc
                     ( x.Height(), x.LockedBuffer(), 1,
                                   y.LockedBuffer(), y.LDim() );
    }
    else if( y.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dotc
                     ( x.Width(), x.LockedBuffer(), x.LDim(),
                                  y.LockedBuffer(), 1        );
    }
    else
    {
        dotProduct = wrappers::blas::Dotc
                     ( x.Width(), x.LockedBuffer(), x.LDim(),
                                  y.LockedBuffer(), y.LDim() );
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return dotProduct;
}

template<typename T>
inline T
elemental::blas::Dotu
( const Matrix<T>& x, const Matrix<T>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Dotu");
    if( (x.Height() != 1 && x.Width() != 1) ||
        (y.Height() != 1 && y.Width() != 1) )
        throw "Expected vector inputs.";
    int xLength = ( x.Width() == 1 ? x.Height() : x.Width() );
    int yLength = ( y.Width() == 1 ? y.Height() : y.Width() );
    if( xLength != yLength )
        throw "x and y must be the same length.";
#endif
    T dotProduct;
    if( x.Width() == 1 && y.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dotu
                     ( x.Height(), x.LockedBuffer(), 1,
                                   y.LockedBuffer(), 1 );
    }
    else if( x.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dotu
                     ( x.Height(), x.LockedBuffer(), 1,
                                   y.LockedBuffer(), y.LDim() );
    }
    else if( y.Width() == 1 )
    {
        dotProduct = wrappers::blas::Dotu
                     ( x.Width(), x.LockedBuffer(), x.LDim(),
                                  y.LockedBuffer(), 1        );
    }
    else
    {
        dotProduct = wrappers::blas::Dotu
                     ( x.Width(), x.LockedBuffer(), x.LDim(),
                                  y.LockedBuffer(), y.LDim() );
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return dotProduct;
}

template<typename R>
inline R
elemental::blas::Nrm2
( const Matrix<R>& x )
{
#ifndef RELEASE
    PushCallStack("blas::Nrm2");
    if( x.Height() != 1 && x.Width() != 1 )
        throw "Expected vector input.";
#endif
    R norm;
    if( x.Width() == 1 )
    {
        norm = wrappers::blas::Nrm2
               ( x.Height(), x.LockedBuffer(), 1 );
    }
    else
    {
        norm = wrappers::blas::Nrm2
               ( x.Width(), x.LockedBuffer(), x.LDim() );
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return norm;
}

#ifndef WITHOUT_COMPLEX
template<typename R>
inline R
elemental::blas::Nrm2
( const Matrix< std::complex<R> >& x )
{
#ifndef RELEASE
    PushCallStack("blas::Nrm2");
    if( x.Height() != 1 && x.Width() != 1 )
        throw "Expected vector input.";
#endif
    R norm;
    if( x.Width() == 1 )
    {
        norm = wrappers::blas::Nrm2
               ( x.Height(), x.LockedBuffer(), 1 );
    }
    else
    {
        norm = wrappers::blas::Nrm2
               ( x.Width(), x.LockedBuffer(), x.LDim() );
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return norm;
}
#endif

template<typename T>
inline void
elemental::blas::Scal
( T alpha, Matrix<T>& X )
{
#ifndef RELEASE
    PushCallStack("blas::Scal");
#endif
    if( alpha != (T)1 )
    {
        if( alpha == (T)0 )
        {
            for( int j=0; j<X.Width(); ++j )
                for( int i=0; i<X.Height(); ++i )
                    X(i,j) = 0;
        }
        else
        {
            for( int j=0; j<X.Width(); ++j )
            {
                wrappers::blas::Scal
                ( X.Height(), alpha, X.Buffer(0,j), 1 );
            }
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Local BLAS: Level 1 (extensions)                                           //
//----------------------------------------------------------------------------//

// Default case is for real datatypes
template<typename R>
inline void
elemental::blas::Conj
( Matrix<R>& A )
{ }

#ifndef WITHOUT_COMPLEX
// Specialization is to complex datatypes
template<typename R>
inline void
elemental::blas::Conj
( Matrix< std::complex<R> >& A )
{
#ifndef RELEASE
    PushCallStack("blas::Conj (in-place)");
#endif
    const int m = A.Height();
    const int n = A.Width();
    for( int j=0; j<n; ++j )
        for( int i=0; i<m; ++i )
            A(i,j) = Conj( A(i,j) );
#ifndef RELEASE
    PopCallStack();
#endif
}
#endif

template<typename T>
inline void
elemental::blas::Conj
( const Matrix<T>& A, Matrix<T>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Conj");
#endif
    const int m = A.Height();
    const int n = A.Width();
    B.ResizeTo( m, n );
    for( int j=0; j<n; ++j )
        for( int i=0; i<m; ++i )
            B(i,j) = Conj( A(i,j) );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::ConjTrans
( const Matrix<T>& A, Matrix<T>& B )
{
#ifndef RELEASE
    PushCallStack("blas::ConjTrans");
#endif
    const int m = A.Height();
    const int n = A.Width();
    B.ResizeTo( n, m );
    for( int j=0; j<n; ++j )
        for( int i=0; i<m; ++i )
            B(j,i) = Conj( A(i,j) );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Trans
( const Matrix<T>& A, Matrix<T>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Trans");
#endif
    const int m = A.Height();
    const int n = A.Width();
    B.ResizeTo( n, m );
    for( int j=0; j<n; ++j )
        for( int i=0; i<m; ++i )
            B(j,i) = A(i,j);
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Local BLAS: Level 2                                                        //
//----------------------------------------------------------------------------//

template<typename T>
inline void
elemental::blas::Gemv
( Orientation orientation,
  T alpha, const Matrix<T>& A, const Matrix<T>& x, T beta, Matrix<T>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Gemv");
    if( ( x.Height() != 1 && x.Width() != 1 ) ||
        ( y.Height() != 1 && y.Width() != 1 ) )
        throw "x and y must be vector.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( orientation == Normal )
    {
        if( A.Height() != yLength || A.Width() != xLength )
        {
            std::ostringstream msg;
            msg << "A must conform with x and y:" << std::endl
                << "  A ~ " << A.Height() << " x " << A.Width() << std::endl
                << "  x ~ " << x.Height() << " x " << x.Width() << std::endl
                << "  y ~ " << y.Height() << " x " << y.Width() << std::endl;
            const std::string& s = msg.str();
            throw s.c_str();
        }
    }
    else
    {
        if( A.Width() != yLength || A.Height() != xLength )
        {
            std::ostringstream msg;
            msg << "A must conform with x and y:" << std::endl
                << "  A ~ " << A.Height() << " x " << A.Width() << std::endl
                << "  x ~ " << x.Height() << " x " << x.Width() << std::endl
                << "  y ~ " << y.Height() << " x " << y.Width() << std::endl;
            const std::string& s = msg.str();
            throw s.c_str();
        }
    }
#endif
    const char transChar = OrientationToChar( orientation );
    const int m = A.Height();
    const int n = A.Width();
    const int k = ( transChar == 'N' ? n : m );
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    if( k != 0 )
    {
        wrappers::blas::Gemv
        ( transChar, m, n, 
          alpha, A.LockedBuffer(), A.LDim(), x.LockedBuffer(), incx, 
          beta,  y.Buffer(), incy );
    }
    else
    {
        blas::Scal( beta, y );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Ger
( T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Ger");
    if( ( x.Height() != 1 && x.Width() != 1 ) ||
        ( y.Height() != 1 && y.Width() != 1 ) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( xLength != A.Height() || yLength != A.Width() )
    {
        std::ostringstream msg;
        msg << "Nonconformal Ger: " << std::endl
            << "  x ~ " << x.Height() << " x " << x.Width() << std::endl
            << "  y ~ " << y.Height() << " x " << y.Width() << std::endl
            << "  A ~ " << A.Height() << " x " << A.Width() << std::endl;
        const std::string& s = msg.str();
        throw s.c_str();
    }
#endif
    const int m = A.Height(); 
    const int n = A.Width();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Ger
    ( m, n, alpha, x.LockedBuffer(), incx, y.LockedBuffer(), incy, 
                   A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Gerc
( T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Gerc");
    if( ( x.Height() != 1 && x.Width() != 1 ) ||
        ( y.Height() != 1 && y.Width() != 1 ) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( xLength != A.Height() || yLength != A.Width() )
        throw "Nonconformal Gerc.";
#endif
    const int m = A.Height(); 
    const int n = A.Width();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Gerc
    ( m, n, alpha, x.LockedBuffer(), incx, y.LockedBuffer(), incy, 
                   A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Geru
( T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Geru");
    if( ( x.Height() != 1 && x.Width() != 1 ) ||
        ( y.Height() != 1 && y.Width() != 1 ) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( xLength != A.Height() || yLength != A.Width() )
        throw "Nonconformal Geru.";
#endif
    const int m = A.Height(); 
    const int n = A.Width();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Geru
    ( m, n, alpha, x.LockedBuffer(), incx, y.LockedBuffer(), incy, 
                   A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Hemv
( Shape shape,
  T alpha, const Matrix<T>& A, const Matrix<T>& x, T beta, Matrix<T>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Hemv");
    if( A.Height() != A.Width() )
        throw "A must be square.";
    if( ( x.Height() != 1 && x.Width() != 1 ) ||
        ( y.Height() != 1 && y.Width() != 1 ) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( A.Height() != xLength || A.Height() != yLength )
        throw "A must conform with x and y.";
#endif
    const char uploChar = ShapeToChar( shape );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Hemv
    ( uploChar, m,
      alpha, A.LockedBuffer(), A.LDim(), x.LockedBuffer(), incx,
      beta,  y.Buffer(), incy );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Her
( Shape shape, T alpha, const Matrix<T>& x, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Her");
    if( A.Height() != A.Width() )
        throw "A must be square.";
    if( x.Width() != 1 && x.Height() != 1 )
        throw "x must be a vector.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    if( xLength != A.Height() )
        throw "x must conform with A.";
#endif
    const char uploChar = ShapeToChar( shape );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    wrappers::blas::Her
    ( uploChar, m,
      alpha, x.LockedBuffer(), incx, A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Her2
( Shape shape,
  T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Her2");
    if( A.Height() != A.Width() )
        throw "A must be square.";
    if( (x.Width() != 1 && x.Height() != 1) || 
        (y.Width() != 1 && y.Height() != 1) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( xLength != A.Height() || yLength != A.Height() )
        throw "x and y must conform with A.";
#endif
    const char uploChar = ShapeToChar( shape );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Her2
    ( uploChar, m,
      alpha, x.LockedBuffer(), incx, y.LockedBuffer(), incy,
             A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Symv
( Shape shape,
  T alpha, const Matrix<T>& A, const Matrix<T>& x, T beta, Matrix<T>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Symv");
    if( A.Height() != A.Width() )
        throw "A must be square.";
    if( ( x.Height() != 1 && x.Width() != 1 ) ||
        ( y.Height() != 1 && y.Width() != 1 ) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( A.Height() != xLength || A.Height() != yLength )
        throw "A must conform with x and y.";
#endif
    const char uploChar = ShapeToChar( shape );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Symv
    ( uploChar, m, 
      alpha, A.LockedBuffer(), A.LDim(), x.LockedBuffer(), incx, 
      beta,  y.Buffer(), incy );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Syr
( Shape shape, T alpha, const Matrix<T>& x, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Syr");
    if( A.Height() != A.Width() )
        throw "A must be square.";
    if( x.Width() != 1 && x.Height() != 1 )
        throw "x must be a vector.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    if( xLength != A.Height() )
        throw "x must conform with A.";
#endif
    const char uploChar = ShapeToChar( shape );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    wrappers::blas::Syr
    ( uploChar, m,
      alpha, x.LockedBuffer(), incx, A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Syr2
( Shape shape, T alpha, const Matrix<T>& x, const Matrix<T>& y, Matrix<T>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Syr2");
    if( A.Height() != A.Width() )
        throw "A must be square.";
    if( (x.Width() != 1 && x.Height() != 1) || 
        (y.Width() != 1 && y.Height() != 1) )
        throw "x and y must be vectors.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    const int yLength = ( y.Width()==1 ? y.Height() : y.Width() );
    if( xLength != A.Height() || yLength != A.Height() )
        throw "x and y must conform with A.";
#endif
    const char uploChar = ShapeToChar( shape );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    const int incy = ( y.Width()==1 ? 1 : y.LDim() );
    wrappers::blas::Syr2
    ( uploChar, m,
      alpha, x.LockedBuffer(), incx, y.LockedBuffer(), incy,
             A.Buffer(), A.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Trmv
( Shape shape, Orientation orientation, Diagonal diagonal,
  const Matrix<T>& A, Matrix<T>& x )
{
#ifndef RELEASE
    PushCallStack("blas::Trmv");
    if( x.Height() != 1 && x.Width() != 1 )
        throw "x must be a vector.";
    if( A.Height() != A.Width() )
        throw "A must be square.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    if( xLength != A.Height() )
        throw "x must conform with A.";
#endif
    const char uploChar = ShapeToChar( shape );
    const char transChar = OrientationToChar( orientation );
    const char diagChar = DiagonalToChar( diagonal );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    wrappers::blas::Trmv
    ( uploChar, transChar, diagChar, m,
      A.LockedBuffer(), A.LDim(), x.Buffer(), incx );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Trsv
( Shape shape, Orientation orientation, Diagonal diagonal,
  const Matrix<T>& A, Matrix<T>& x )
{
#ifndef RELEASE
    PushCallStack("blas::Trsv");
    if( x.Height() != 1 && x.Width() != 1 )
        throw "x must be a vector.";
    if( A.Height() != A.Width() )
        throw "A must be square.";
    const int xLength = ( x.Width()==1 ? x.Height() : x.Width() );
    if( xLength != A.Height() )
        throw "x must conform with A.";
#endif
    const char uploChar = ShapeToChar( shape );
    const char transChar = OrientationToChar( orientation );
    const char diagChar = DiagonalToChar( diagonal );
    const int m = A.Height();
    const int incx = ( x.Width()==1 ? 1 : x.LDim() );
    wrappers::blas::Trsv
    ( uploChar, transChar, diagChar, m,
      A.LockedBuffer(), A.LDim(), x.Buffer(), incx );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Local BLAS: Level 3                                                        //
//----------------------------------------------------------------------------//

template<typename T>
inline void
elemental::blas::Gemm
( Orientation orientationOfA, Orientation orientationOfB,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Gemm");
    if( orientationOfA == Normal && orientationOfB == Normal )
    {
        if( A.Height() != C.Height() ||
            B.Width()  != C.Width()  ||
            A.Width()  != B.Height() )
            throw "Nonconformal GemmNN.";
    }
    else if( orientationOfA == Normal )
    {
        if( A.Height() != C.Height() ||
            B.Height() != C.Width()  ||
            A.Width()  != B.Width() )
            throw "Nonconformal GemmN(T/C).";
    }
    else if( orientationOfB == Normal )
    {
        if( A.Width()  != C.Height() ||
            B.Width()  != C.Width()  ||
            A.Height() != B.Height() )
            throw "Nonconformal Gemm(T/C)N.";
    }
    else
    {
        if( A.Width()  != C.Height() ||
            B.Height() != C.Width()  ||
            A.Height() != B.Width() )
            throw "Nonconformal Gemm(T/C)(T/C).";
    }
#endif
    const char transA = OrientationToChar( orientationOfA );
    const char transB = OrientationToChar( orientationOfB );
    const int m = C.Height();
    const int n = C.Width();
    const int k = ( orientationOfA == Normal ? A.Width() : A.Height() );
    if( k != 0 )
    {
        wrappers::blas::Gemm
        ( transA, transB, m, n, k, 
          alpha, A.LockedBuffer(), A.LDim(), B.LockedBuffer(), B.LDim(),
          beta,  C.Buffer(),       C.LDim() );
    }
    else
    {
        blas::Scal( beta, C );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Hemm
( Side side, Shape shape,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Hemm");
#endif
    const char sideChar = SideToChar( side );
    const char shapeChar = ShapeToChar( shape );
    wrappers::blas::Hemm
    ( sideChar, shapeChar, C.Height(), C.Width(),
      alpha, A.LockedBuffer(), A.LDim(), 
             B.LockedBuffer(), B.LDim(),
      beta,  C.Buffer(),       C.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Her2k
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Her2k");
    if( orientation == Normal )
    {
        if( A.Height() != C.Height() || A.Height() != C.Width() ||
            B.Height() != C.Height() ||B.Height() != C.Width() )
            throw "Nonconformal Her2k.";
    }
    else if( orientation == ConjugateTranspose )
    {
        if( A.Width() != C.Height() || A.Width() != C.Width() ||
            B.Width() != C.Height() || B.Width() != C.Width() )
            throw "Nonconformal Her2k.";
    }
    else
        throw "Her2k only accepts Normal and ConjugateTranspose options.";
#endif
    const char uplo = ShapeToChar( shape );
    const char trans = OrientationToChar( orientation );
    const int k = ( orientation == Normal ? A.Width() : A.Height() );
    wrappers::blas::Her2k
    ( uplo, trans, C.Height(), k, 
      alpha, A.LockedBuffer(), A.LDim(), 
             B.LockedBuffer(), B.LDim(),
      beta,  C.Buffer(),       C.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Herk
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Herk");
    if( orientation == Normal )
    {
        if( A.Height() != C.Height() || A.Height() != C.Width() )
            throw "Nonconformal Herk.";
    }
    else if( orientation == ConjugateTranspose )
    {
        if( A.Width() != C.Height() || A.Width() != C.Width() )
            throw "Nonconformal Herk.";
    }
    else
        throw "Herk only accepts Normal and ConjugateTranpose options.";
#endif
    const char uplo = ShapeToChar( shape );
    const char trans = OrientationToChar( orientation );
    const int k = ( orientation == Normal ? A.Width() : A.Height() );
    wrappers::blas::Herk
    ( uplo, trans, C.Height(), k, 
      alpha, A.LockedBuffer(), A.LDim(), 
      beta,  C.Buffer(),       C.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Symm
( Side side, Shape shape,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Symm");
#endif
    const char sideChar = SideToChar( side );
    const char shapeChar = ShapeToChar( shape );
    wrappers::blas::Symm
    ( sideChar, shapeChar, C.Height(), C.Width(),
      alpha, A.LockedBuffer(), A.LDim(), 
             B.LockedBuffer(), B.LDim(),
      beta,  C.Buffer(),       C.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Syr2k
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, const Matrix<T>& B, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Syr2k");
    if( orientation == Normal )
    {
        if( A.Height() != C.Height() || A.Height() != C.Width() ||
            B.Height() != C.Height() ||B.Height() != C.Width()    )
            throw "Nonconformal Syr2k.";
    }
    else if( orientation == Transpose )
    {
        if( A.Width() != C.Height() || A.Width() != C.Width() ||
            B.Width() != C.Height() || B.Width() != C.Width()   )
            throw "Nonconformal Syr2k.";
    }
    else
        throw "Syr2k only accepts Normal and Tranpose options.";
#endif
    const char uplo = ShapeToChar( shape );
    const char trans = OrientationToChar( orientation );
    const int k = ( orientation == Normal ? A.Width() : A.Height() );
    wrappers::blas::Syr2k
    ( uplo, trans, C.Height(), k, 
      alpha, A.LockedBuffer(), A.LDim(), 
             B.LockedBuffer(), B.LDim(),
      beta,  C.Buffer(),       C.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Syrk
( Shape shape, Orientation orientation,
  T alpha, const Matrix<T>& A, T beta, Matrix<T>& C )
{
#ifndef RELEASE
    PushCallStack("blas::Syrk");
    if( orientation == Normal )
    {
        if( A.Height() != C.Height() || A.Height() != C.Width() )
            throw "Nonconformal Syrk.";
    }
    else if( orientation == Transpose )
    {
        if( A.Width() != C.Height() || A.Width() != C.Width() )
            throw "Nonconformal Syrk.";
    }
    else
        throw "Syrk only accepts Normal and Tranpose options.";
#endif
    const char uplo = ShapeToChar( shape );
    const char trans = OrientationToChar( orientation );
    const int k = ( orientation == Normal ? A.Width() : A.Height() );
    wrappers::blas::Syrk
    ( uplo, trans, C.Height(), k, 
      alpha, A.LockedBuffer(), A.LDim(), 
      beta,  C.Buffer(),       C.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Trmm
( Side side, Shape shape, 
  Orientation orientation, Diagonal diagonal,
  T alpha, const Matrix<T>& A, Matrix<T>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Trmm");
    if( A.Height() != A.Width() )
        throw "Triangular matrix must be square.";
    if( side == Left )
    {
        if( A.Height() != B.Height() )
            throw "Nonconformal Trmm.";
    }
    else
    {
        if( A.Height() != B.Width() )
            throw "Nonconformal Trmm.";
    }
#endif
    const char sideChar = SideToChar( side );
    const char uploChar = ShapeToChar( shape );
    const char transChar = OrientationToChar( orientation );
    const char diagChar = DiagonalToChar( diagonal );
    wrappers::blas::Trmm
    ( sideChar, uploChar, transChar, diagChar, B.Height(), B.Width(),
      alpha, A.LockedBuffer(), A.LDim(), B.Buffer(), B.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
elemental::blas::Trsm
( Side side, Shape shape,
  Orientation orientation,Diagonal diagonal,
  T alpha, const Matrix<T>& A, Matrix<T>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Trsm");
    if( A.Height() != A.Width() )
        throw "Triangular matrix must be square.";
    if( side == Left )
    {
        if( A.Height() != B.Height() )
            throw "Nonconformal Trsm.";
    }
    else
    {
        if( A.Height() != B.Width() )
            throw "Nonconformal Trsm.";
    }
#endif
    const char sideChar = SideToChar( side );
    const char uploChar = ShapeToChar( shape );
    const char transChar = OrientationToChar( orientation );
    const char diagChar = DiagonalToChar( diagonal );
    wrappers::blas::Trsm
    ( sideChar, uploChar, transChar, diagChar, B.Height(), B.Width(),
      alpha, A.LockedBuffer(), A.LDim(), B.Buffer(), B.LDim() );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Distributed BLAS: Level 1                                                  //
//----------------------------------------------------------------------------//

template<typename T, elemental::Distribution U, elemental::Distribution V>
inline void
elemental::blas::Axpy
( T alpha, const DistMatrix<T,U,V>& X, DistMatrix<T,U,V>& Y )
{
#ifndef RELEASE
    PushCallStack("blas::Axpy");
    if( X.GetGrid() != Y.GetGrid() )
        throw "X and Y must be distributed over the same grid.";
    if( X.ColAlignment() != Y.ColAlignment() ||
        X.RowAlignment() != Y.RowAlignment() )
        throw "Axpy requires X and Y be aligned.";
#endif
    blas::Axpy( alpha, X.LockedLocalMatrix(), Y.LocalMatrix() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T, elemental::Distribution U, elemental::Distribution V,
                     elemental::Distribution W, elemental::Distribution Z >
inline void
elemental::blas::Copy
( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Copy");
#endif
    B = A;
#ifndef RELEASE
    PopCallStack();
#endif
}

// Our extended Dotc is equivalent to our extended Dot, 
// but we are burdened with consistency
template<typename T, elemental::Distribution U, elemental::Distribution V,
                     elemental::Distribution W, elemental::Distribution Z >
inline T
elemental::blas::Dotc
( const DistMatrix<T,U,V>& x, const DistMatrix<T,W,Z>& y )
{
#ifndef RELEASE
    PushCallStack("blas::Dotc");
#endif
    T globalDot = blas::Dot( x, y );
#ifndef RELEASE
    PopCallStack();
#endif
    return globalDot;
}

template<typename T, elemental::Distribution U, elemental::Distribution V>
inline void
elemental::blas::Scal
( T alpha, DistMatrix<T,U,V>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Scal");
#endif
    blas::Scal( alpha, A.LocalMatrix() );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Distributed BLAS: Level 1 (extensions)                                     //
//----------------------------------------------------------------------------//

template<typename T, elemental::Distribution U, elemental::Distribution V>
inline void
elemental::blas::Conj
( DistMatrix<T,U,V>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Conj (in-place)");
#endif
    blas::Conj( A.LocalMatrix() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T, elemental::Distribution U, elemental::Distribution V,
                     elemental::Distribution W, elemental::Distribution Z >
inline void
elemental::blas::Conj
( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Conj");
#endif
    B = A;
    blas::Conj( B ); 
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T, elemental::Distribution U, elemental::Distribution V,
                     elemental::Distribution W, elemental::Distribution Z >
inline void
elemental::blas::ConjTrans
( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B )
{
#ifndef RELEASE
    PushCallStack("blas::ConjTrans");
#endif
    DistMatrix<T,Z,W> C( B.GetGrid() );
    if( B.ConstrainedColAlignment() )
        C.AlignRowsWith( B );
    if( B.ConstrainedRowAlignment() )
        C.AlignColsWith( B );

    C = A;

    if( !B.ConstrainedColAlignment() )
        B.AlignColsWith( C );
    if( !B.ConstrainedRowAlignment() )
        B.AlignRowsWith( C );

    B.ResizeTo( A.Width(), A.Height() );
    blas::ConjTrans( C.LockedLocalMatrix(), B.LocalMatrix() );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T, elemental::Distribution U, elemental::Distribution V,
                     elemental::Distribution W, elemental::Distribution Z >
inline void
elemental::blas::Trans
( const DistMatrix<T,U,V>& A, DistMatrix<T,W,Z>& B )
{
#ifndef RELEASE
    PushCallStack("blas::Trans");
#endif
    DistMatrix<T,Z,W> C( B.GetGrid() );
    if( B.ConstrainedColAlignment() )
        C.AlignRowsWith( B );
    if( B.ConstrainedRowAlignment() )
        C.AlignColsWith( B );

    C = A;

    if( !B.ConstrainedColAlignment() )
        B.AlignColsWith( C );
    if( !B.ConstrainedRowAlignment() )
        B.AlignRowsWith( C );

    B.ResizeTo( A.Width(), A.Height() );
    blas::Trans( C.LockedLocalMatrix(), B.LocalMatrix() );
#ifndef RELEASE
    PopCallStack();
#endif
}

//----------------------------------------------------------------------------//
// Distributed BLAS: Level 2                                                  //
//----------------------------------------------------------------------------//

// Our extended Ger and Gerc are equivalent
template<typename T>
inline void
elemental::blas::Gerc
( T alpha, const DistMatrix<T,MC,MR>& x,
           const DistMatrix<T,MC,MR>& y,
                 DistMatrix<T,MC,MR>& A )
{
#ifndef RELEASE
    PushCallStack("blas::Gerc");
#endif
    blas::Ger( alpha, x, y, A );
#ifndef RELEASE
    PopCallStack();
#endif
}

#endif /* ELEMENTAL_BLAS_HPP */
