/*
   Copyright (c) 2009-2011, Jack Poulson
   All rights reserved.

   This file is part of Elemental.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    - Neither the name of the owner nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
#include <ctime>
#include "elemental.hpp"
#include "elemental/blas_internal.hpp"
using namespace std;
using namespace elemental;
using namespace elemental::imports::mpi;

void Usage()
{
    cout << "SYmmetric Matrix Matrix multiplication.\n\n"
         << "  Symm <r> <c> <Side> <Shape> <m> <n> <nb> <print?>\n\n"
         << "  r: number of process rows\n"
         << "  c: number of process cols\n"
         << "  Side: {L,R}\n"
         << "  Shape: {L,U}\n"
         << "  m: height of C\n" 
         << "  n: width  of C\n"
         << "  nb: algorithmic blocksize\n"
         << "  print?: [0/1]\n" << endl;
}

template<typename T> // represents a real or complex ring
void TestSymm
( const Side side, const Shape shape,
  const int m, const int n, const T alpha, const T beta,
  const bool printMatrices, const Grid& g  )
{
    double startTime, endTime, runTime, gFlops;
    DistMatrix<T,MC,MR> A(g);
    DistMatrix<T,MC,MR> B(g);
    DistMatrix<T,MC,MR> C(g);

    if( side == Left )
        A.ResizeTo( m, m );
    else
        A.ResizeTo( n, n );
    B.ResizeTo( m, n );
    C.ResizeTo( m, n );

    // Test Symm
    if( g.VCRank() == 0 )
        cout << "Symm:" << endl;
    A.SetToRandom();
    B.SetToRandom();
    C.SetToRandom();
    if( printMatrices )
    {
        A.Print("A");
        B.Print("B");
        C.Print("C");
    }
    if( g.VCRank() == 0 )
    {
        cout << "  Starting Parallel Symm...";
        cout.flush();
    }
    Barrier( g.VCComm() );
    startTime = Time();
    blas::Symm
    ( side, shape, alpha, A, B, beta, C );
    Barrier( g.VCComm() );
    endTime = Time();
    runTime = endTime - startTime;
    gFlops = blas::internal::SymmGFlops<T>(side,m,n,runTime);
    if( g.VCRank() == 0 )
    {
        cout << "DONE. " << endl
             << "  Time = " << runTime << " seconds. GFlops = " 
             << gFlops << endl;
    }
    if( printMatrices )
    {
        ostringstream msg;
        if( side == Left )
            msg << "C := " << alpha << " Symm(A) B + " << beta << " C";
        else
            msg << "C := " << alpha << " B Symm(A) + " << beta << " C";
        C.Print( msg.str() );
    }
}

int main( int argc, char* argv[] )
{
    int rank;
    Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if( argc != 9 )
    {
        if( rank == 0 )
            Usage();
        Finalize();
        return 0;
    }
    try
    {
        int argNum = 0;
        const int r = atoi(argv[++argNum]);
        const int c = atoi(argv[++argNum]);
        const Side side = CharToSide(*argv[++argNum]);
        const Shape shape = CharToShape(*argv[++argNum]);
        const int m = atoi(argv[++argNum]);
        const int n = atoi(argv[++argNum]);
        const int nb = atoi(argv[++argNum]);
        const bool printMatrices = atoi(argv[++argNum]);
#ifndef RELEASE
        if( rank == 0 )
        {
            cout << "==========================================\n"
                 << " In debug mode! Performance will be poor! \n"
                 << "==========================================" << endl;
        }
#endif
        const Grid g( MPI_COMM_WORLD, r, c );
        SetBlocksize( nb );

        if( rank == 0 )
        {
            cout << "Will test Symm" << SideToChar(side) 
                                     << ShapeToChar(shape) << endl;
        }

        if( rank == 0 )
        {
            cout << "---------------------\n"
                 << "Testing with doubles:\n"
                 << "---------------------" << endl;
        }
        TestSymm<double>
        ( side, shape, m, n, (double)3, (double)4, printMatrices, g ); 

#ifndef WITHOUT_COMPLEX
        if( rank == 0 )
        {
            cout << "--------------------------------------\n"
                 << "Testing with double-precision complex:\n"
                 << "--------------------------------------" << endl;
        }
        TestSymm<dcomplex>
        ( side, shape, m, n, (dcomplex)3, (dcomplex)4, printMatrices, g ); 
#endif
    }
    catch( exception& e )
    {
#ifndef RELEASE
        DumpCallStack();
#endif
        cerr << "Process " << rank << " caught error message:" << endl 
             << e.what() << endl;
    }
    Finalize();
    return 0;
}
