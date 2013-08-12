/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
// NOTE: It is possible to simply include "elemental.hpp" instead
#include "elemental-lite.hpp"
#include "elemental/matrices/Uniform.hpp"
using namespace elem;

template<typename T, Distribution AColDist, Distribution ARowDist,
                     Distribution BColDist, Distribution BRowDist>
void
Check( DistMatrix<T,AColDist,ARowDist>& A, 
       DistMatrix<T,BColDist,BRowDist>& B )
{
#ifndef RELEASE
    CallStackEntry entry("Check");
#endif
    const Grid& g = A.Grid();

    const Int commRank = g.Rank();
    const Int height = B.Height();
    const Int width = B.Width();
    DistMatrix<T,STAR,STAR> A_STAR_STAR(g);
    DistMatrix<T,STAR,STAR> B_STAR_STAR(g);

    if( commRank == 0 )
    {
        std::cout << "Testing [" << DistToString(AColDist) << ","
                                 << DistToString(ARowDist) << "]"
                  << " <- ["     << DistToString(BColDist) << ","
                                 << DistToString(BRowDist) << "]...";
        std::cout.flush();
    }
    A = B;

    A_STAR_STAR = A;
    B_STAR_STAR = B;

    Int myErrorFlag = 0;
    for( Int j=0; j<width; ++j )
    {
        for( Int i=0; i<height; ++i )
        {
            if( A_STAR_STAR.GetLocal(i,j) != B_STAR_STAR.GetLocal(i,j) )
            {
                myErrorFlag = 1;
                break;
            }
        }
        if( myErrorFlag != 0 )
            break;
    }

    Int summedErrorFlag;
    mpi::AllReduce( &myErrorFlag, &summedErrorFlag, 1, mpi::SUM, g.Comm() );

    if( summedErrorFlag == 0 )
    {
        if( commRank == 0 )
            std::cout << "PASSED" << std::endl;
    }
    else
        LogicError("Redistribution failed");
}

template<typename T>
void
DistMatrixTest( Int m, Int n, const Grid& g )
{
#ifndef RELEASE
    CallStackEntry entry("DistMatrixTest");
#endif
    DistMatrix<T,MC,  MR  > A_MC_MR(g);
    DistMatrix<T,MC,  STAR> A_MC_STAR(g);
    DistMatrix<T,STAR,MR  > A_STAR_MR(g);
    DistMatrix<T,MR,  MC  > A_MR_MC(g);
    DistMatrix<T,MR,  STAR> A_MR_STAR(g);
    DistMatrix<T,STAR,MC  > A_STAR_MC(g);
    DistMatrix<T,VC,  STAR> A_VC_STAR(g);
    DistMatrix<T,STAR,VC  > A_STAR_VC(g);
    DistMatrix<T,VR,  STAR> A_VR_STAR(g);
    DistMatrix<T,STAR,VR  > A_STAR_VR(g);
    DistMatrix<T,STAR,STAR> A_STAR_STAR(g);

    // Communicate from A[MC,MR] 
    Uniform( A_MC_MR, m, n );
    Check( A_MC_STAR,   A_MC_MR );
    Check( A_STAR_MR,   A_MC_MR );
    Check( A_MR_MC,     A_MC_MR );
    Check( A_MR_STAR,   A_MC_MR );
    Check( A_STAR_MC,   A_MC_MR );
    Check( A_VC_STAR,   A_MC_MR );
    Check( A_STAR_VC,   A_MC_MR );
    Check( A_VR_STAR,   A_MC_MR );
    Check( A_STAR_VR,   A_MC_MR );
    Check( A_STAR_STAR, A_MC_MR );

    // Communicate from A[MC,*]
    Uniform( A_MC_STAR, m, n );
    Check( A_MC_MR,     A_MC_STAR );
    Check( A_STAR_MR,   A_MC_STAR );
    Check( A_MR_MC,     A_MC_STAR );
    Check( A_MR_STAR,   A_MC_STAR );
    Check( A_STAR_MC,   A_MC_STAR );
    Check( A_VC_STAR,   A_MC_STAR );
    Check( A_STAR_VC,   A_MC_STAR );
    Check( A_VR_STAR,   A_MC_STAR );
    Check( A_STAR_VR,   A_MC_STAR );
    Check( A_STAR_STAR, A_MC_STAR );

    // Communicate from A[*,MR]
    Uniform( A_STAR_MR, m, n );
    Check( A_MC_MR,     A_STAR_MR );
    Check( A_MC_STAR,   A_STAR_MR );
    Check( A_MR_MC,     A_STAR_MR );
    Check( A_MR_STAR,   A_STAR_MR );
    Check( A_STAR_MC,   A_STAR_MR );
    Check( A_VC_STAR,   A_STAR_MR );
    Check( A_STAR_VC,   A_STAR_MR );
    Check( A_VR_STAR,   A_STAR_MR );
    Check( A_STAR_VR,   A_STAR_MR );
    Check( A_STAR_STAR, A_STAR_MR );
    
    // Communicate from A[MR,MC]
    Uniform( A_MR_MC, m, n );
    Check( A_MC_MR,     A_MR_MC );
    Check( A_MC_STAR,   A_MR_MC );
    Check( A_STAR_MR,   A_MR_MC );
    Check( A_MR_STAR,   A_MR_MC );
    Check( A_STAR_MC,   A_MR_MC );
    Check( A_VC_STAR,   A_MR_MC );
    Check( A_STAR_VC,   A_MR_MC );
    Check( A_VR_STAR,   A_MR_MC );
    Check( A_STAR_VR,   A_MR_MC );
    Check( A_STAR_STAR, A_MR_MC );

    // Communicate from A[MR,*]
    Uniform( A_MR_STAR, m, n );
    Check( A_MC_MR,     A_MR_STAR );
    Check( A_MC_STAR,   A_MR_STAR );
    Check( A_STAR_MR,   A_MR_STAR );
    Check( A_MR_MC,     A_MR_STAR );
    Check( A_STAR_MC,   A_MR_STAR );
    Check( A_VC_STAR,   A_MR_STAR );
    Check( A_STAR_VC,   A_MR_STAR );
    Check( A_VR_STAR,   A_MR_STAR );
    Check( A_STAR_VR,   A_MR_STAR );
    Check( A_STAR_STAR, A_MR_STAR );

    // Communicate from A[*,MC]
    Uniform( A_STAR_MC, m, n );
    Check( A_MC_MR,     A_STAR_MC );
    Check( A_MC_STAR,   A_STAR_MC );
    Check( A_STAR_MR,   A_STAR_MC );
    Check( A_MR_MC,     A_STAR_MC );
    Check( A_MR_STAR,   A_STAR_MC );
    Check( A_VC_STAR,   A_STAR_MC );
    Check( A_STAR_VC,   A_STAR_MC );
    Check( A_VR_STAR,   A_STAR_MC );
    Check( A_STAR_VR,   A_STAR_MC );
    Check( A_STAR_STAR, A_STAR_MC );
 
    // Communicate from A[VC,*]
    Uniform( A_VC_STAR, m, n );
    Check( A_MC_MR,     A_VC_STAR );
    Check( A_MC_STAR,   A_VC_STAR );
    Check( A_STAR_MR,   A_VC_STAR );
    Check( A_MR_MC,     A_VC_STAR );
    Check( A_MR_STAR,   A_VC_STAR );
    Check( A_STAR_MC,   A_VC_STAR );
    Check( A_STAR_VC,   A_VC_STAR );
    Check( A_VR_STAR,   A_VC_STAR );
    Check( A_STAR_VR,   A_VC_STAR );
    Check( A_STAR_STAR, A_VC_STAR );

    // Communicate from A[*,VC]
    Uniform( A_STAR_VC, m, n );
    Check( A_MC_MR,     A_STAR_VC );
    Check( A_MC_STAR,   A_STAR_VC );
    Check( A_STAR_MR,   A_STAR_VC );
    Check( A_MR_MC,     A_STAR_VC );
    Check( A_MR_STAR,   A_STAR_VC );
    Check( A_STAR_MC,   A_STAR_VC );
    Check( A_VC_STAR,   A_STAR_VC );
    Check( A_VR_STAR,   A_STAR_VC );
    Check( A_STAR_VR,   A_STAR_VC );
    Check( A_STAR_STAR, A_STAR_VC );

    // Communicate from A[VR,*]
    Uniform( A_VR_STAR, m, n );
    Check( A_MC_MR,     A_VR_STAR );
    Check( A_MC_STAR,   A_VR_STAR );
    Check( A_STAR_MR,   A_VR_STAR );
    Check( A_MR_MC,     A_VR_STAR );
    Check( A_MR_STAR,   A_VR_STAR );
    Check( A_STAR_MC,   A_VR_STAR );
    Check( A_VC_STAR,   A_VR_STAR );
    Check( A_STAR_VC,   A_VR_STAR );
    Check( A_STAR_VR,   A_VR_STAR );
    Check( A_STAR_STAR, A_VR_STAR );

    // Communicate from A[*,VR]
    Uniform( A_STAR_VR, m, n );
    Check( A_MC_MR,     A_STAR_VR );
    Check( A_MC_STAR,   A_STAR_VR );
    Check( A_STAR_MR,   A_STAR_VR );
    Check( A_MR_MC,     A_STAR_VR );
    Check( A_MR_STAR,   A_STAR_VR );
    Check( A_STAR_MC,   A_STAR_VR );
    Check( A_VC_STAR,   A_STAR_VR );
    Check( A_STAR_VC,   A_STAR_VR );
    Check( A_VR_STAR,   A_STAR_VR );
    Check( A_STAR_STAR, A_STAR_VR );

    // Communicate from A[*,*]
    Uniform( A_STAR_STAR, m, n );
    Check( A_MC_MR,   A_STAR_STAR );
    Check( A_MC_STAR, A_STAR_STAR );
    Check( A_STAR_MR, A_STAR_STAR );
    Check( A_MR_MC,   A_STAR_STAR );
    Check( A_MR_STAR, A_STAR_STAR );
    Check( A_STAR_MC, A_STAR_STAR );
    Check( A_VC_STAR, A_STAR_STAR );
    Check( A_STAR_VC, A_STAR_STAR );
    Check( A_VR_STAR, A_STAR_STAR );
    Check( A_STAR_VR, A_STAR_STAR );
}

int 
main( int argc, char* argv[] )
{
    Initialize( argc, argv );
    mpi::Comm comm = mpi::COMM_WORLD;
    const Int commRank = mpi::CommRank( comm );
    const Int commSize = mpi::CommSize( comm );

    try
    {
        Int r = Input("--gridHeight","height of process grid",0);
        const Int m = Input("--height","height of matrix",100);
        const Int n = Input("--width","width of matrix",100);
        ProcessInput();
        PrintInputReport();

        if( r == 0 )
            r = Grid::FindFactor( commSize );
        const Grid g( comm, r );

        if( commRank == 0 )
        {
            std::cout << "--------------------\n"
                      << "Testing with floats:\n"
                      << "--------------------" << std::endl;
        }
        DistMatrixTest<float>( m, n, g );

        if( commRank == 0 )
        {
            std::cout << "---------------------\n"
                      << "Testing with doubles:\n"
                      << "---------------------" << std::endl;
        }
        DistMatrixTest<double>( m, n, g );

        if( commRank == 0 )
        {
            std::cout << "--------------------------------------\n"
                      << "Testing with single-precision complex:\n"
                      << "--------------------------------------" << std::endl;
        }
        DistMatrixTest<Complex<float> >( m, n, g );
        
        if( commRank == 0 )
        {
            std::cout << "--------------------------------------\n"
                      << "Testing with double-precision complex:\n"
                      << "--------------------------------------" << std::endl;
        }
        DistMatrixTest<Complex<double> >( m, n, g );
    }
    catch( std::exception& e ) { ReportException(e); }

    Finalize();
    return 0;
}
