// Copyright (c) 2017-2021, University of Tennessee. All rights reserved.
// Copyright (c) 2021-2022, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef TLAPACK_BLAS_SYMV_HH
#define TLAPACK_BLAS_SYMV_HH

#include "tlapack/base/utils.hpp"

namespace tlapack {

/**
 * Symmetric matrix-vector multiply:
 * \[
 *     y := \alpha A x + \beta y,
 * \]
 * where alpha and beta are scalars, x and y are vectors,
 * and A is an n-by-n symmetric matrix.
 *
 * @param[in] uplo
 *     What part of the matrix A is referenced,
 *     the opposite triangle being assumed from symmetry.
 *     - Uplo::Lower: only the lower triangular part of A is referenced.
 *     - Uplo::Upper: only the upper triangular part of A is referenced.
 *
 * @param[in] alpha Scalar.
 * @param[in] A A n-by-n symmetric matrix.
 * @param[in] x A n-element vector.
 * @param[in] beta Scalar.
 * @param[in,out] y A n-element vector.
 *
 * @ingroup symv
 */
template<
    class matrixA_t,
    class vectorX_t, class vectorY_t, 
    class alpha_t, class beta_t,
    class T = type_t<vectorY_t>,
    disable_if_allow_optblas_t<
        pair< matrixA_t, T >,
        pair< vectorX_t, T >,
        pair< vectorY_t, T >,
        pair< beta_t,    T >
    > = 0
>
void symv(
    Uplo uplo,
    const alpha_t& alpha, const matrixA_t& A, const vectorX_t& x,
    const beta_t& beta, vectorY_t& y )
{
    // data traits
    using TA    = type_t< matrixA_t >;
    using TX    = type_t< vectorX_t >;
    using idx_t = size_type< matrixA_t >;

    // using
    using scalar_t = scalar_type<TA,TX>;

    // constants
    const idx_t n = nrows(A);

    // check arguments
    tlapack_check_false( uplo != Uplo::Lower &&
                   uplo != Uplo::Upper );
    tlapack_check_false( ncols(A) != n );
    tlapack_check_false( size(x)  != n );
    tlapack_check_false( size(y)  != n );

    tlapack_check_false( access_denied( uplo, read_policy(A) ) );

    // form y = beta*y
    for (idx_t i = 0; i < n; ++i)
        y[i] *= beta;

    if (uplo == Uplo::Upper) {
        // A is stored in upper triangle
        // form y += alpha * A * x
        for (idx_t j = 0; j < n; ++j) {
            auto tmp1 = alpha*x[j];
            auto tmp2 = scalar_t(0);
            for (idx_t i = 0; i < j; ++i) {
                y[i] += tmp1 * A(i,j);
                tmp2 += A(i,j) * x[i];
            }
            y[j] += tmp1 * A(j,j) + alpha * tmp2;
        }
    }
    else {
        // A is stored in lower triangle
        // form y += alpha * A * x
        for (idx_t j = 0; j < n; ++j) {
            auto tmp1 = alpha*x[j];
            auto tmp2 = scalar_t(0);
            for (idx_t i = j+1; i < n; ++i) {
                y[i] += tmp1 * A(i,j);
                tmp2 += A(i,j) * x[i];
            }
            y[j] += tmp1 * A(j,j) + alpha * tmp2;
        }
    }
}

/**
 * Symmetric matrix-vector multiply:
 * \[
 *     y := \alpha A x,
 * \]
 * where alpha and beta are scalars, x and y are vectors,
 * and A is an n-by-n symmetric matrix.
 *
 * @param[in] uplo
 *     What part of the matrix A is referenced,
 *     the opposite triangle being assumed from symmetry.
 *     - Uplo::Lower: only the lower triangular part of A is referenced.
 *     - Uplo::Upper: only the upper triangular part of A is referenced.
 *
 * @param[in] alpha Scalar.
 * @param[in] A A n-by-n symmetric matrix.
 * @param[in] x A n-element vector.
 * @param[in,out] y A n-element vector.
 *
 * @ingroup symv
 */
template<
    class matrixA_t,
    class vectorX_t, class vectorY_t, 
    class alpha_t,
    class T = type_t<vectorY_t>,
    disable_if_allow_optblas_t<
        pair< matrixA_t, T >,
        pair< vectorX_t, T >,
        pair< vectorY_t, T >
    > = 0
>
inline
void symv(
    Uplo uplo,
    const alpha_t& alpha, const matrixA_t& A, const vectorX_t& x,
    vectorY_t& y )
{
    return symv( uplo, alpha, A, x, internal::StrongZero(), y );
}

}  // namespace tlapack

#endif        //  #ifndef TLAPACK_BLAS_SYMV_HH
