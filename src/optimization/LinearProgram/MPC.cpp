/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "../LinearProgram.hpp"

// TODO: Add warnings if the maximum number of iterations is exceeded

namespace El {
namespace lin_prog {

// TODO: Add std::function<Real(Real)> to generalize sigma = (mu_aff/mu)^3
template<typename Real>
void MPC
( const Matrix<Real>& A, 
  const Matrix<Real>& b,  const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const MPCCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("lin_prog::MPC"))    

    const Int m = A.Height();
    const Int n = A.Width();
    Matrix<Real> J, y, rmu, rb, rc, 
                 dsAff, dxAff, dlAff,
                 ds,    dx,    dl;
    Matrix<Real> dSub;
    Matrix<Int> p;
#ifndef RELEASE
    Matrix<Real> dsError, dxError, dlError;
#endif
    for( Int numIts=0; numIts<ctrl.maxIts; ++numIts )
    {
#ifndef RELEASE
        // Check that no entries of x or s are non-positive
        // ================================================
        Int numNonPos_x = 0;
        for( Int i=0; i<x.Height(); ++i )
            if( x.Get(i,0) <= Real(0) )
                ++numNonPos_x;
        Int numNonPos_s = 0;
        for( Int i=0; i<s.Height(); ++i )
            if( s.Get(i,0) <= Real(0) )
                ++numNonPos_s;
        if( numNonPos_x > 0 || numNonPos_s > 0 )
            std::cout << numNonPos_x << " entries of x were nonzero and "
                      << numNonPos_s << " entries of s were nonzero"
                      << std::endl;
#endif

        // Check for convergence
        // =====================
        // |c^T x - b^T l| / (1 + |c^T x|) <= tol ?
        // ----------------------------------------
        const Real primObj = Dot(c,x);
        const Real dualObj = Dot(b,l); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        const Real bNrm2 = Nrm2( b );
        rb = b;
        Gemv( NORMAL, Real(1), A, x, Real(-1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        const Real cNrm2 = Nrm2( c );
        rc = c;
        Gemv( TRANSPOSE, Real(1), A, l, Real(-1), rc );
        Axpy( Real(1), s, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;
        else if( ctrl.print )
            std::cout << " iter " << numIts << ":\n"
                      << "  |c^T x - b^T l| / (1 + |c^T x|) = "
                      << objConv << "\n"
                      << "  || r_b ||_2 / (1 + || b ||_2)   = "
                      << rbConv << "\n"
                      << "  || r_c ||_2 / (1 + || c ||_2)   = "
                      << rcConv << std::endl;

        // r_mu := X S e
        // =============
        rmu.Resize( n, 1 );
        for( Int i=0; i<n; ++i )
            rmu.Set( i, 0, x.Get(i,0)*s.Get(i,0) );

        // Compute the affine search direction
        // ===================================
        if( ctrl.system == FULL_KKT )
        {
            // Construct the full KKT system
            // -----------------------------
            KKT( A, s, x, J );
            KKTRHS( rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LU( J, p );
            lu::SolveAfter( NORMAL, J, p, y );
            ExpandKKTSolution( m, n, y, dsAff, dxAff, dlAff );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the "augmented" KKT system
            // ------------------------------------
            AugmentedKKT( A, s, x, J );
            AugmentedKKTRHS( x, rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, y, false );
            ExpandAugmentedSolution( s, x, rmu, y, dsAff, dxAff, dlAff );
        }
        else if( ctrl.system == NORMAL_KKT )
        {
            // Construct the "normal" KKT system
            // ---------------------------------
            NormalKKT( A, s, x, J );
            NormalKKTRHS( A, s, x, rmu, rc, rb, dlAff );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, dlAff, false );
            ExpandNormalSolution( A, c, s, x, rmu, rc, dlAff, dsAff, dxAff );
        }

#ifndef RELEASE
        // Sanity checks
        // =============
        Real rmuNrm2 = Nrm2( rmu ); 
        dsError = rmu;
        for( Int i=0; i<n; ++i )
        {
            const Real xi = x.Get(i,0);
            const Real si = s.Get(i,0);
            const Real dxi = dxAff.Get(i,0);
            const Real dsi = dsAff.Get(i,0);
            dsError.Update( i, 0, xi*dsi + si*dxi );
        }
        Real dsErrorNrm2 = Nrm2( dsError );

        dlError = dsAff;
        Gemv( TRANSPOSE, Real(1), A, dlAff, Real(1), dlError );
        Axpy( Real(1), rc, dlError );
        Real dlErrorNrm2 = Nrm2( dlError );

        dxError = rb;
        Gemv( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        if( ctrl.print )
            std::cout << "  || dsAffError ||_2 / (1 + || r_mu ||_2) = " 
                      << dsErrorNrm2/(1+rmuNrm2) << "\n"
                      << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                      << dxErrorNrm2/(1+rbNrm2) << "\n"
                      << "  || dlAffError ||_2 / (1 + || r_c ||_2) = " 
                      << dlErrorNrm2/(1+rcNrm2) << std::endl;
#endif

        // Compute the maximum affine [0,1]-step which preserves positivity
        // ================================================================
        Real alphaAffPri = 1; 
        for( Int i=0; i<n; ++i )
        {
            const Real xi = x.Get(i,0);
            const Real dxAffi = dxAff.Get(i,0);
            if( dxAffi < Real(0) )
                alphaAffPri = Min(alphaAffPri,-xi/dxAffi);
        }
        Real alphaAffDual = 1; 
        for( Int i=0; i<n; ++i )
        {
            const Real si = s.Get(i,0);
            const Real dsAffi = dsAff.Get(i,0);
            if( dsAffi < Real(0) )
                alphaAffDual = Min(alphaAffDual,-si/dsAffi);
        }
        if( ctrl.print )
            std::cout << "  alphaAffPri = " << alphaAffPri 
                      << ", alphaAffDual = " << alphaAffDual << std::endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,s) / n;
        // NOTE: ds and dx are used as temporaries
        dx = x;
        ds = s;
        Axpy( alphaAffPri, dxAff, dx );
        Axpy( alphaAffDual, dsAff, ds );
        const Real muAff = Dot(dx,ds) / n;

        // Compute a centrality parameter using Mehotra's formula
        // ======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print )
            std::cout << "  muAff = " << muAff 
                      << ", mu = " << mu 
                      << ", sigma = " << sigma << std::endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        for( Int i=0; i<n; ++i )
            rmu.Set( i, 0, dxAff.Get(i,0)*dsAff.Get(i,0) - sigma*mu );
        if( ctrl.system == FULL_KKT )
        {
            // Construct the new full KKT RHS
            // ------------------------------
            KKTRHS( rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            lu::SolveAfter( NORMAL, J, p, y );
            ExpandKKTSolution( m, n, y, ds, dx, dl );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the new "augmented" KKT RHS
            // -------------------------------------
            AugmentedKKTRHS( x, rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, y, false );
            ExpandAugmentedSolution( s, x, rmu, y, ds, dx, dl );
        }
        else if( ctrl.system == NORMAL_KKT )
        {
            // Construct the new "normal" KKT RHS
            // ----------------------------------
            NormalKKTRHS( A, s, x, rmu, rc, rb, dl );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, dl, false );
            ExpandNormalSolution( A, c, s, x, rmu, rc, dl, ds, dx );
        }

        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dsAff, ds );
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dlAff, dl );

        // Compute the max positive [0,1/maxStepRatio] step length
        // =======================================================
        Real alphaPri = 1/ctrl.maxStepRatio;
        for( Int i=0; i<n; ++i )
        {
            const Real xi = x.Get(i,0);
            const Real dxi = dx.Get(i,0);
            if( dxi < Real(0) )
                alphaPri = Min(alphaPri,-xi/dxi);
        }
        Real alphaDual = 1/ctrl.maxStepRatio;
        for( Int i=0; i<n; ++i )
        {
            const Real si = s.Get(i,0);
            const Real dsi = ds.Get(i,0);
            if( dsi < Real(0) )
                alphaDual = Min(alphaDual,-si/dsi);
        }
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print )
            std::cout << "  alphaPri = " << alphaPri 
                      << ", alphaDual = " << alphaDual << std::endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, ds, s ); 
        Axpy( alphaDual, dl, l );
    }
}

template<typename Real>
void MPC
( const AbstractDistMatrix<Real>& APre, 
  const AbstractDistMatrix<Real>& b,  const AbstractDistMatrix<Real>& c,
  AbstractDistMatrix<Real>& sPre, AbstractDistMatrix<Real>& xPre, 
  AbstractDistMatrix<Real>& l,
  const MPCCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("lin_prog::MPC"))    

    ProxyCtrl control;
    control.colConstrain = true;
    control.rowConstrain = true;
    control.colAlign = 0;
    control.rowAlign = 0;
    auto APtr = ReadProxy<Real,MC,MR>(&APre,control);      auto& A = *APtr;
    auto sPtr = ReadWriteProxy<Real,MC,MR>(&sPre,control); auto& s = *sPtr;
    auto xPtr = ReadWriteProxy<Real,MC,MR>(&xPre,control); auto& x = *xPtr;

    const Int m = A.Height();
    const Int n = A.Width();
    const Grid& grid = A.Grid();
    const Int commRank = A.Grid().Rank();

    DistMatrix<Real> 
        J(grid), y(grid), rmu(grid), rb(grid), rc(grid), 
        dsAff(grid), dxAff(grid), dlAff(grid),
        ds(grid),    dx(grid),    dl(grid);
    ds.AlignWith( x );
    dx.AlignWith( x );
    dsAff.AlignWith( x );
    dxAff.AlignWith( x );
    rmu.AlignWith( x );
    DistMatrix<Real> dSub(grid);
    DistMatrix<Int> p(grid);
#ifndef RELEASE
    DistMatrix<Real> dsError(grid), dxError(grid), dlError(grid);
    dsError.AlignWith( ds );
#endif
    for( Int numIts=0; numIts<ctrl.maxIts; ++numIts )
    {
#ifndef RELEASE
        // Check that no entries of x or s are non-positive
        // ================================================
        Int numNonPos_x = 0;
        if( x.IsLocalCol(0) )
            for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
                if( x.GetLocal(iLoc,0) <= Real(0) ) 
                    ++numNonPos_x;
        numNonPos_x = mpi::AllReduce( numNonPos_x, x.DistComm() );
        Int numNonPos_s = 0;
        if( s.IsLocalCol(0) )
            for( Int iLoc=0; iLoc<s.LocalHeight(); ++iLoc )
                if( s.GetLocal(iLoc,0) <= Real(0) )
                    ++numNonPos_s;
        numNonPos_s = mpi::AllReduce( numNonPos_s, s.DistComm() );
        if( (numNonPos_x > 0 || numNonPos_s > 0) && commRank == 0 )
            std::cout << numNonPos_x << " entries of x were nonzero and " 
                      << numNonPos_s << " entries of s were nonzero" 
                      << std::endl;
#endif

        // Check for convergence
        // =====================
        // |c^T x - b^T l| / (1 + |c^T x|) <= tol ?
        // ----------------------------------------
        const Real primObj = Dot(c,x);
        const Real dualObj = Dot(b,l); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        const Real bNrm2 = Nrm2( b );
        rb = b;
        Gemv( NORMAL, Real(1), A, x, Real(-1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        const Real cNrm2 = Nrm2( c );
        rc = c;
        Gemv( TRANSPOSE, Real(1), A, l, Real(-1), rc );
        Axpy( Real(1), s, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;
        else if( ctrl.print && commRank == 0 )
            std::cout << " iter " << numIts << ":\n"
                      << "  |c^T x - b^T l| / (1 + |c^T x|) = "
                      << objConv << "\n"
                      << "  || r_b ||_2 / (1 + || b ||_2)   = "
                      << rbConv << "\n"
                      << "  || r_c ||_2 / (1 + || c ||_2)   = "
                      << rcConv << std::endl;

        // r_mu := X S e
        // =============
        // TODO: Find a more convenient syntax for expressing this operation
        rmu.Resize( n, 1 );
        if( rmu.IsLocalCol(0) )
            for( Int iLoc=0; iLoc<rmu.LocalHeight(); ++iLoc )
                rmu.SetLocal( iLoc, 0, x.GetLocal(iLoc,0)*s.GetLocal(iLoc,0) );

        // Compute the affine search direction
        // ===================================
        if( ctrl.system == FULL_KKT )
        {
            // Construct the full KKT system
            // -----------------------------
            KKT( A, s, x, J );
            KKTRHS( rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LU( J, p );
            lu::SolveAfter( NORMAL, J, p, y );
            ExpandKKTSolution( m, n, y, dsAff, dxAff, dlAff );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the "augmented" KKT system
            // ------------------------------------
            AugmentedKKT( A, s, x, J );
            AugmentedKKTRHS( x, rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, y, false );
            ExpandAugmentedSolution( s, x, rmu, y, dsAff, dxAff, dlAff );
        }
        else if( ctrl.system == NORMAL_KKT )
        {
            // Construct the "normal" KKT system
            // ---------------------------------
            NormalKKT( A, s, x, J );
            NormalKKTRHS( A, s, x, rmu, rc, rb, dlAff );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, dlAff, false );
            ExpandNormalSolution( A, c, s, x, rmu, rc, dlAff, dsAff, dxAff );
        }

#ifndef RELEASE
        // Sanity checks
        // =============
        Real rmuNrm2 = Nrm2( rmu ); 
        // TODO: Find a more convenient syntax for expressing this operation
        dsError = rmu;
        if( dsError.IsLocalCol(0) )
        {
            for( Int iLoc=0; iLoc<dsError.LocalHeight(); ++iLoc )
            {
                const Real xi = x.GetLocal(iLoc,0);
                const Real si = s.GetLocal(iLoc,0);
                const Real dxi = dxAff.GetLocal(iLoc,0);
                const Real dsi = dsAff.GetLocal(iLoc,0);
                dsError.UpdateLocal( iLoc, 0, xi*dsi + si*dxi );
            }
        }
        Real dsErrorNrm2 = Nrm2( dsError );

        dlError = dsAff;
        Gemv( TRANSPOSE, Real(1), A, dlAff, Real(1), dlError );
        Axpy( Real(1), rc, dlError );
        Real dlErrorNrm2 = Nrm2( dlError );

        dxError = rb;
        Gemv( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        if( ctrl.print && commRank == 0 )
            std::cout << "  || dsAffError ||_2 / (1 + || r_mu ||_2) = " 
                      << dsErrorNrm2/(1+rmuNrm2) << "\n"
                      << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                      << dxErrorNrm2/(1+rbNrm2) << "\n"
                      << "  || dlAffError ||_2 / (1 + || r_c ||_2) = " 
                      << dlErrorNrm2/(1+rcNrm2) << std::endl;
#endif

        // Compute the maximum affine [0,1]-step which preserves positivity
        // ================================================================
        Real alphaAffPri = 1; 
        // TODO: Find a more convenient way to represent this operation
        if( x.IsLocalCol(0) )
        {
            for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
            {
                const Real xi = x.GetLocal(iLoc,0);
                const Real dxAffi = dxAff.GetLocal(iLoc,0);
                if( dxAffi < Real(0) )
                    alphaAffPri = Min(alphaAffPri,-xi/dxAffi);
            }
        }
        alphaAffPri = mpi::AllReduce( alphaAffPri, mpi::MIN, x.DistComm() );
        Real alphaAffDual = 1; 
        // TODO: Find a more convenient way to represent this operation
        if( s.IsLocalCol(0) )
        {
            for( Int iLoc=0; iLoc<s.LocalHeight(); ++iLoc )
            {
                const Real si = s.GetLocal(iLoc,0);
                const Real dsAffi = dsAff.GetLocal(iLoc,0);
                if( dsAffi < Real(0) )
                    alphaAffDual = Min(alphaAffDual,-si/dsAffi);
            }
        }
        alphaAffDual = mpi::AllReduce( alphaAffDual, mpi::MIN, s.DistComm() );
        if( ctrl.print && commRank == 0 )
            std::cout << "  alphaAffPri = " << alphaAffPri 
                      << ", alphaAffDual = " << alphaAffDual << std::endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,s) / n;
        // NOTE: ds and dx are used as temporaries
        dx = x;
        ds = s;
        Axpy( alphaAffPri, dxAff, dx );
        Axpy( alphaAffDual, dsAff, ds );
        const Real muAff = Dot(dx,ds) / n;

        // Compute a centrality parameter using Mehotra's formula
        // ======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print && commRank == 0 )
            std::cout << "  muAff = " << muAff 
                      << ", mu = " << mu 
                      << ", sigma = " << sigma << std::endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        // TODO: Find a more convenient means of expressing this operation
        if( dxAff.IsLocalCol(0) )
            for( Int iLoc=0; iLoc<dxAff.LocalHeight(); ++iLoc )
                rmu.SetLocal
                ( iLoc, 0, 
                  dxAff.GetLocal(iLoc,0)*dsAff.GetLocal(iLoc,0) - sigma*mu );
        if( ctrl.system == FULL_KKT )
        {
            // Construct the new full KKT RHS
            // ------------------------------
            KKTRHS( rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            lu::SolveAfter( NORMAL, J, p, y );
            ExpandKKTSolution( m, n, y, ds, dx, dl );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the new "augmented" KKT RHS
            // -------------------------------------
            AugmentedKKTRHS( x, rmu, rc, rb, y );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, y, false );
            ExpandAugmentedSolution( s, x, rmu, y, ds, dx, dl );
        }
        else if( ctrl.system == NORMAL_KKT )
        {
            // Construct the new "normal" KKT RHS
            // ----------------------------------
            NormalKKTRHS( A, s, x, rmu, rc, rb, dl );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, dl, false );
            ExpandNormalSolution( A, c, s, x, rmu, rc, dl, ds, dx );
        }

        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dsAff, ds );
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dlAff, dl );

        // Compute the max positive [0,1/maxStepRatio] step length
        // =======================================================
        Real alphaPri = 1/ctrl.maxStepRatio;
        // TODO: Find a more convenient means of expressing this operation
        if( x.IsLocalCol(0) )
        {
            for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
            {
                const Real xi = x.GetLocal(iLoc,0);
                const Real dxi = dx.GetLocal(iLoc,0);
                if( dxi < Real(0) )
                    alphaPri = Min(alphaPri,-xi/dxi);
            }
        }
        alphaPri = mpi::AllReduce( alphaPri, mpi::MIN, x.DistComm() );
        Real alphaDual = 1/ctrl.maxStepRatio;
        // TODO: Find a more convenient means of expressing this operation
        if( s.IsLocalCol(0) )
        {
            for( Int iLoc=0; iLoc<s.LocalHeight(); ++iLoc )
            {
                const Real si = s.GetLocal(iLoc,0);
                const Real dsi = ds.GetLocal(iLoc,0);
                if( dsi < Real(0) )
                    alphaDual = Min(alphaDual,-si/dsi);
            }
        }
        alphaDual = mpi::AllReduce( alphaDual, mpi::MIN, s.DistComm() );
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print && commRank == 0 )
            std::cout << "  alphaPri = " << alphaAffPri 
                      << ", alphaDual = " << alphaAffDual << std::endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, ds, s ); 
        Axpy( alphaDual, dl, l );
    }
}

template<typename Real>
void MPC
( const SparseMatrix<Real>& A, 
  const Matrix<Real>& b,  const Matrix<Real>& c,
  Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l,
  const MPCCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("lin_prog::MPC"))    

    // TODO: Check that x and s are strictly positive

    const Int m = A.Height();
    const Int n = A.Width();
    SparseMatrix<Real> J;
    Matrix<Real> y, rmu, rb, rc, 
                 dsAff, dxAff, dlAff,
                 ds,    dx,    dl;
    Matrix<Real> dSub;
    Matrix<Int> p;
#ifndef RELEASE
    Matrix<Real> dsError, dxError, dlError;
#endif
    for( Int numIts=0; numIts<ctrl.maxIts; ++numIts )
    {
#ifndef RELEASE
        // Check that no entries of x or s are non-positive
        // ================================================
        Int numNonPos_x = 0;
        for( Int i=0; i<x.Height(); ++i )
            if( x.Get(i,0) <= Real(0) )
                ++numNonPos_x;
        Int numNonPos_s = 0;
        for( Int i=0; i<s.Height(); ++i )
            if( s.Get(i,0) <= Real(0) )
                ++numNonPos_s;
        if( numNonPos_x > 0 || numNonPos_s > 0 )
            std::cout << numNonPos_x << " entries of x were nonzero and "
                      << numNonPos_s << " entries of s were nonzero"
                      << std::endl;
#endif

        // Check for convergence
        // =====================
        // |c^T x - b^T l| / (1 + |c^T x|) <= tol ?
        // ----------------------------------------
        const Real primObj = Dot(c,x);
        const Real dualObj = Dot(b,l); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        const Real bNrm2 = Nrm2( b );
        rb = b;
        Multiply( NORMAL, Real(1), A, x, Real(-1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        const Real cNrm2 = Nrm2( c );
        rc = c;
        Multiply( TRANSPOSE, Real(1), A, l, Real(-1), rc );
        Axpy( Real(1), s, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;
        else if( ctrl.print )
            std::cout << " iter " << numIts << ":\n"
                      << "  |c^T x - b^T l| / (1 + |c^T x|) = "
                      << objConv << "\n"
                      << "  || r_b ||_2 / (1 + || b ||_2)   = "
                      << rbConv << "\n"
                      << "  || r_c ||_2 / (1 + || c ||_2)   = "
                      << rcConv << std::endl;

        // r_mu := X S e
        // =============
        rmu.Resize( n, 1 );
        for( Int i=0; i<n; ++i )
            rmu.Set( i, 0, x.Get(i,0)*s.Get(i,0) );

        // Compute the affine search direction
        // ===================================
        // Construct the "normal" KKT system
        // ---------------------------------
        NormalKKT( A, s, x, J );
        NormalKKTRHS( A, s, x, rmu, rc, rb, dlAff );

        // Compute the proposed step from the KKT system
        // ---------------------------------------------
        //LDL( J, dSub, p, false );
        //ldl::SolveAfter( J, dSub, p, dlAff, false );
        LogicError("Sequential SymmetricSolve not yet supported");
        ExpandNormalSolution( A, c, s, x, rmu, rc, dlAff, dsAff, dxAff );

#ifndef RELEASE
        // Sanity checks
        // =============
        Real rmuNrm2 = Nrm2( rmu ); 
        dsError = rmu;
        for( Int i=0; i<n; ++i )
        {
            const Real xi = x.Get(i,0);
            const Real si = s.Get(i,0);
            const Real dxi = dxAff.Get(i,0);
            const Real dsi = dsAff.Get(i,0);
            dsError.Update( i, 0, xi*dsi + si*dxi );
        }
        Real dsErrorNrm2 = Nrm2( dsError );

        dlError = dsAff;
        Multiply( TRANSPOSE, Real(1), A, dlAff, Real(1), dlError );
        Axpy( Real(1), rc, dlError );
        Real dlErrorNrm2 = Nrm2( dlError );

        dxError = rb;
        Multiply( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        if( ctrl.print )
            std::cout << "  || dsAffError ||_2 / (1 + || r_mu ||_2) = " 
                      << dsErrorNrm2/(1+rmuNrm2) << "\n"
                      << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                      << dxErrorNrm2/(1+rbNrm2) << "\n"
                      << "  || dlAffError ||_2 / (1 + || r_c ||_2) = " 
                      << dlErrorNrm2/(1+rcNrm2) << std::endl;
#endif

        // Compute the maximum affine [0,1]-step which preserves positivity
        // ================================================================
        Real alphaAffPri = 1; 
        for( Int i=0; i<n; ++i )
        {
            const Real xi = x.Get(i,0);
            const Real dxAffi = dxAff.Get(i,0);
            if( dxAffi < Real(0) )
                alphaAffPri = Min(alphaAffPri,-xi/dxAffi);
        }
        Real alphaAffDual = 1; 
        for( Int i=0; i<n; ++i )
        {
            const Real si = s.Get(i,0);
            const Real dsAffi = dsAff.Get(i,0);
            if( dsAffi < Real(0) )
                alphaAffDual = Min(alphaAffDual,-si/dsAffi);
        }
        if( ctrl.print )
            std::cout << "  alphaAffPri = " << alphaAffPri 
                      << ", alphaAffDual = " << alphaAffDual << std::endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,s) / n;
        // NOTE: ds and dx are used as temporaries
        dx = x;
        ds = s;
        Axpy( alphaAffPri, dxAff, dx );
        Axpy( alphaAffDual, dsAff, ds );
        const Real muAff = Dot(dx,ds) / n;

        // Compute a centrality parameter using Mehotra's formula
        // ======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print )
            std::cout << "  muAff = " << muAff 
                      << ", mu = " << mu 
                      << ", sigma = " << sigma << std::endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        for( Int i=0; i<n; ++i )
            rmu.Set( i, 0, dxAff.Get(i,0)*dsAff.Get(i,0) - sigma*mu );
        // Construct the new "normal" KKT RHS
        // ----------------------------------
        NormalKKTRHS( A, s, x, rmu, rc, rb, dl );

        // Compute the proposed step from the KKT system
        // ---------------------------------------------
        //ldl::SolveAfter( J, dSub, p, dl, false );
        LogicError("Sequential SymmetricSolve not yet supported");
        ExpandNormalSolution( A, c, s, x, rmu, rc, dl, ds, dx );

        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dsAff, ds );
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dlAff, dl );

        // Compute the max positive [0,1/maxStepRatio] step length
        // =======================================================
        Real alphaPri = 1/ctrl.maxStepRatio;
        for( Int i=0; i<n; ++i )
        {
            const Real xi = x.Get(i,0);
            const Real dxi = dx.Get(i,0);
            if( dxi < Real(0) )
                alphaPri = Min(alphaPri,-xi/dxi);
        }
        Real alphaDual = 1/ctrl.maxStepRatio;
        for( Int i=0; i<n; ++i )
        {
            const Real si = s.Get(i,0);
            const Real dsi = ds.Get(i,0);
            if( dsi < Real(0) )
                alphaDual = Min(alphaDual,-si/dsi);
        }
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print )
            std::cout << "  alphaPri = " << alphaPri 
                      << ", alphaDual = " << alphaDual << std::endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, ds, s ); 
        Axpy( alphaDual, dl, l );
    }
}

template<typename Real>
void MPC
( const DistSparseMatrix<Real>& A, 
  const DistMultiVec<Real>& b,  const DistMultiVec<Real>& c,
  DistMultiVec<Real>& s, DistMultiVec<Real>& x, DistMultiVec<Real>& l,
  const MPCCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("lin_prog::MPC"))    

    const Int m = A.Height();
    const Int n = A.Width();
    mpi::Comm comm = A.Comm();
    const Int commRank = mpi::Rank(comm);

    DistSymmInfo info;
    DistSeparatorTree sepTree;
    DistMap map, invMap;
    DistSparseMatrix<Real> J(comm);
    DistSymmFrontTree<Real> JFrontTree;
    DistNodalMultiVec<Real> dlNodal;

    DistMultiVec<Real> y(comm), rmu(comm), rb(comm), rc(comm), 
                       dsAff(comm), dxAff(comm), dlAff(comm),
                       ds(comm),    dx(comm),    dl(comm);
#ifndef RELEASE
    DistMultiVec<Real> dsError(comm), dxError(comm), dlError(comm);
#endif
    for( Int numIts=0; numIts<ctrl.maxIts; ++numIts )
    {
#ifndef RELEASE
        // Check that no entries of x or s are non-positive
        // ================================================
        Int numNonPos_x = 0;
        for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
            if( x.GetLocal(iLoc,0) <= Real(0) )
                ++numNonPos_x;
        numNonPos_x = mpi::AllReduce( numNonPos_x, comm );
        Int numNonPos_s = 0;
        for( Int iLoc=0; iLoc<s.LocalHeight(); ++iLoc )
            if( s.GetLocal(iLoc,0) <= Real(0) )
                ++numNonPos_s;
        numNonPos_s = mpi::AllReduce( numNonPos_s, comm );
        if( (numNonPos_x > 0 || numNonPos_s > 0) && commRank == 0 )
            std::cout << numNonPos_x << " entries of x were nonzero and "
                      << numNonPos_s << " entries of s were nonzero"
                      << std::endl;
#endif

        // Check for convergence
        // =====================
        // |c^T x - b^T l| / (1 + |c^T x|) <= tol ?
        // ----------------------------------------
        const Real primObj = Dot(c,x);
        const Real dualObj = Dot(b,l); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        const Real bNrm2 = Nrm2( b );
        rb = b;
        Multiply( NORMAL, Real(1), A, x, Real(-1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        const Real cNrm2 = Nrm2( c );
        rc = c;
        Multiply( TRANSPOSE, Real(1), A, l, Real(-1), rc );
        Axpy( Real(1), s, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;
        else if( ctrl.print && commRank == 0 )
            std::cout << " iter " << numIts << ":\n"
                      << "  |c^T x - b^T l| / (1 + |c^T x|) = "
                      << objConv << "\n"
                      << "  || r_b ||_2 / (1 + || b ||_2)   = "
                      << rbConv << "\n"
                      << "  || r_c ||_2 / (1 + || c ||_2)   = "
                      << rcConv << std::endl;

        // r_mu := X S e
        // =============
        rmu.Resize( n, 1 );
        for( Int iLoc=0; iLoc<rmu.LocalHeight(); ++iLoc )
            rmu.SetLocal( iLoc, 0, x.GetLocal(iLoc,0)*s.GetLocal(iLoc,0) );

        // Compute the affine search direction
        // ===================================
        // Construct the "normal" KKT system
        // ---------------------------------
        NormalKKT( A, s, x, J, false );
        NormalKKTRHS( A, s, x, rmu, rc, rb, dlAff );

        // Compute the proposed step from the KKT system
        // ---------------------------------------------
        if( numIts == 0 )
        {
            NestedDissection( J.LockedDistGraph(), map, sepTree, info );
            map.FormInverse( invMap );
        }
        JFrontTree.Initialize( J, map, sepTree, info );
        LDL( info, JFrontTree, LDL_INTRAPIV_1D );
        dlNodal.Pull( invMap, info, dlAff );
        Solve( info, JFrontTree, dlNodal );
        dlNodal.Push( invMap, info, dlAff );
        ExpandNormalSolution( A, c, s, x, rmu, rc, dlAff, dsAff, dxAff );

#ifndef RELEASE
        // Sanity checks
        // =============
        Real rmuNrm2 = Nrm2( rmu ); 
        dsError = rmu;
        for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
        {
            const Real xi = x.GetLocal(iLoc,0);
            const Real si = s.GetLocal(iLoc,0);
            const Real dxi = dxAff.GetLocal(iLoc,0);
            const Real dsi = dsAff.GetLocal(iLoc,0);
            dsError.UpdateLocal( iLoc, 0, xi*dsi + si*dxi );
        }
        Real dsErrorNrm2 = Nrm2( dsError );

        dlError = dsAff;
        Multiply( TRANSPOSE, Real(1), A, dlAff, Real(1), dlError );
        Axpy( Real(1), rc, dlError );
        Real dlErrorNrm2 = Nrm2( dlError );

        dxError = rb;
        Multiply( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        if( ctrl.print && commRank == 0 )
            std::cout << "  || dsAffError ||_2 / (1 + || r_mu ||_2) = " 
                      << dsErrorNrm2/(1+rmuNrm2) << "\n"
                      << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                      << dxErrorNrm2/(1+rbNrm2) << "\n"
                      << "  || dlAffError ||_2 / (1 + || r_c ||_2) = " 
                      << dlErrorNrm2/(1+rcNrm2) << std::endl;
#endif

        // Compute the maximum affine [0,1]-step which preserves positivity
        // ================================================================
        Real alphaAffPri = 1; 
        for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
        {
            const Real xi = x.GetLocal(iLoc,0);
            const Real dxAffi = dxAff.GetLocal(iLoc,0);
            if( dxAffi < Real(0) )
                alphaAffPri = Min(alphaAffPri,-xi/dxAffi);
        }
        alphaAffPri = mpi::AllReduce( alphaAffPri, mpi::MIN, comm );
        Real alphaAffDual = 1; 
        for( Int iLoc=0; iLoc<s.LocalHeight(); ++iLoc )
        {
            const Real si = s.GetLocal(iLoc,0);
            const Real dsAffi = dsAff.GetLocal(iLoc,0);
            if( dsAffi < Real(0) )
                alphaAffDual = Min(alphaAffDual,-si/dsAffi);
        }
        alphaAffDual = mpi::AllReduce( alphaAffDual, mpi::MIN, comm );
        if( ctrl.print && commRank == 0 )
            std::cout << "  alphaAffPri = " << alphaAffPri 
                      << ", alphaAffDual = " << alphaAffDual << std::endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,s) / n;
        // NOTE: ds and dx are used as temporaries
        dx = x;
        ds = s;
        Axpy( alphaAffPri, dxAff, dx );
        Axpy( alphaAffDual, dsAff, ds );
        const Real muAff = Dot(dx,ds) / n;

        // Compute a centrality parameter using Mehotra's formula
        // ======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print && commRank == 0 )
            std::cout << "  muAff = " << muAff 
                      << ", mu = " << mu 
                      << ", sigma = " << sigma << std::endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        for( Int iLoc=0; iLoc<rmu.LocalHeight(); ++iLoc )
            rmu.SetLocal
            ( iLoc, 0, 
              dxAff.GetLocal(iLoc,0)*dsAff.GetLocal(iLoc,0) - sigma*mu );
        // Construct the new "normal" KKT RHS
        // ----------------------------------
        NormalKKTRHS( A, s, x, rmu, rc, rb, dl );

        // Compute the proposed step from the KKT system
        // ---------------------------------------------
        dlNodal.Pull( invMap, info, dl );
        Solve( info, JFrontTree, dlNodal );
        dlNodal.Push( invMap, info, dl );
        ExpandNormalSolution( A, c, s, x, rmu, rc, dl, ds, dx );

        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dsAff, ds );
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dlAff, dl );

        // Compute the max positive [0,1/maxStepRatio] step length
        // =======================================================
        Real alphaPri = 1/ctrl.maxStepRatio;
        for( Int iLoc=0; iLoc<x.LocalHeight(); ++iLoc )
        {
            const Real xi = x.GetLocal(iLoc,0);
            const Real dxi = dx.GetLocal(iLoc,0);
            if( dxi < Real(0) )
                alphaPri = Min(alphaPri,-xi/dxi);
        }
        alphaPri = mpi::AllReduce( alphaPri, mpi::MIN, comm );
        Real alphaDual = 1/ctrl.maxStepRatio;
        for( Int iLoc=0; iLoc<s.LocalHeight(); ++iLoc )
        {
            const Real si = s.GetLocal(iLoc,0);
            const Real dsi = ds.GetLocal(iLoc,0);
            if( dsi < Real(0) )
                alphaDual = Min(alphaDual,-si/dsi);
        }
        alphaDual = mpi::AllReduce( alphaDual, mpi::MIN, comm );
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print && commRank == 0 )
            std::cout << "  alphaPri = " << alphaPri 
                      << ", alphaDual = " << alphaDual << std::endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, ds, s ); 
        Axpy( alphaDual, dl, l );
    }
}

#define PROTO(Real) \
  template void MPC \
  ( const Matrix<Real>& A, \
    const Matrix<Real>& b,  const Matrix<Real>& c, \
    Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l, \
    const MPCCtrl<Real>& ctrl ); \
  template void MPC \
  ( const AbstractDistMatrix<Real>& A, \
    const AbstractDistMatrix<Real>& b,  const AbstractDistMatrix<Real>& c, \
    AbstractDistMatrix<Real>& s, AbstractDistMatrix<Real>& x, \
    AbstractDistMatrix<Real>& l, \
    const MPCCtrl<Real>& ctrl ); \
  template void MPC \
  ( const SparseMatrix<Real>& A, \
    const Matrix<Real>& b,  const Matrix<Real>& c, \
    Matrix<Real>& s, Matrix<Real>& x, Matrix<Real>& l, \
    const MPCCtrl<Real>& ctrl ); \
  template void MPC \
  ( const DistSparseMatrix<Real>& A, \
    const DistMultiVec<Real>& b,  const DistMultiVec<Real>& c, \
    DistMultiVec<Real>& s, DistMultiVec<Real>& x, DistMultiVec<Real>& l, \
    const MPCCtrl<Real>& ctrl );

#define EL_NO_INT_PROTO
#define EL_NO_COMPLEX_PROTO
#include "El/macros/Instantiate.h"

} // namespace lin_prog
} // namespace El
