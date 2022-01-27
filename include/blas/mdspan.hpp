// Copyright (c) 2021, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef __TBLAS_MDSPAN_HH__
#define __TBLAS_MDSPAN_HH__

#include <cassert>              // For the assert macro
#include <experimental/mdspan>  // Use mdspan for multidimensional arrays

namespace blas {

using std::experimental::mdspan;
using std::experimental::extents;
using std::experimental::full_extent_t;
constexpr auto full_extent = full_extent_t{ };

#define isSlice(SliceSpec) \
(   is_convertible_v< SliceSpec, full_extent_t > || \
    is_convertible_v< SliceSpec, std::tuple<size_t, size_t> > )

// -----------------------------------------------------------------------------
// Data traits for mdspan

// Data type
template< class ET, class Exts, class LP, class AP >
struct type_trait< mdspan<ET,Exts,LP,AP> > {
    using type = ET;
};
// Size type
template< class ET, class Exts, class LP, class AP >
struct sizet_trait< mdspan<ET,Exts,LP,AP> > {
    using type = typename mdspan<ET,Exts,LP,AP>::size_type;
};

// Size
template< class ET, class Exts, class LP, class AP >
inline constexpr auto
size( const mdspan<ET,Exts,LP,AP>& x ) {
    return x.size();
}
// Number of rows
template< class ET, class Exts, class LP, class AP >
inline constexpr auto
nrows( const mdspan<ET,Exts,LP,AP>& x ) {
    return x.extent(0);
}
// Number of columns
template< class ET, class Exts, class LP, class AP >
inline constexpr auto
ncols( const mdspan<ET,Exts,LP,AP>& x ) {
    return x.extent(1);
}

// Submatrix
template< class ET, class Exts, class LP, class AP,
    class SliceSpecRow, class SliceSpecCol,
    enable_if_t< isSlice(SliceSpecRow) && isSlice(SliceSpecCol), int > = 0
>
inline constexpr auto submatrix(
    const mdspan<ET,Exts,LP,AP>& A,
    const SliceSpecRow& rows,
    const SliceSpecCol& cols ) noexcept
{
    return std::experimental::submdspan( A, rows, cols );
}

// Extract row from matrix
template<
    class ET, class Exts, class LP, class AP,
    class SliceSpec, enable_if_t< isSlice(SliceSpec), int > = 0
>
inline constexpr auto row(
    const mdspan<ET,Exts,LP,AP>& A,
    size_t rowIdx,
    const SliceSpec& cols = full_extent ) noexcept
{
    return std::experimental::submdspan( A, rowIdx, cols );
}

// Extract column from matrix
template<
    class ET, class Exts, class LP, class AP,
    class SliceSpec, enable_if_t< isSlice(SliceSpec), int > = 0
>
inline constexpr auto col(
    const mdspan<ET,Exts,LP,AP>& A,
    size_t colIdx,
    const SliceSpec& rows = full_extent ) noexcept
{
    return std::experimental::submdspan( A, rows, colIdx );
}

// Subvector
template< class ET, class Exts, class LP, class AP,
    class SliceSpec, enable_if_t< isSlice(SliceSpec), int > = 0
>
inline constexpr auto subvector(
    mdspan<ET,Exts,LP,AP>& v,
    const SliceSpec& rows ) noexcept
{
    return std::experimental::submdspan( v, rows );
}

// Extract a diagonal from a matrix
template< class ET, class Exts, class LP, class AP,
    enable_if_t<
    /* Requires: */
        LP::template mapping<Exts>::is_always_strided()
    , bool > = true
>
inline constexpr auto diag(
    const mdspan<ET, Exts, LP, AP>& A,
    int diagIdx = 0 ) noexcept
{
    using std::abs;
    using std::min;
    using std::array;
    using std::experimental::dextents;
    using std::experimental::layout_stride;
    using extents_t = dextents<1>;
    using mapping = typename layout_stride::template mapping< extents_t >;

    // constants
    const auto n = min( A.extent(0), A.extent(1) );
    const auto s = A.stride(0) + A.stride(1);
    const array<typename extents_t::size_type, 1> stride = {s};
    
    return mdspan< ET, extents_t, layout_stride, AP > (
        (diagIdx >= 0)
            ? A.data() + A.mapping()(diagIdx,0)
            : A.data() + A.mapping()(0,-diagIdx),
        mapping( extents_t( n - abs(diagIdx) ), stride )
    );
}

namespace internal {

using std::experimental::dextents;
using std::experimental::layout_stride;
using std::array;

// // -----------------------------------------------------------------------------
// /// layout_colmajor Column Major layout for mdspan
// struct layout_colmajor {
//     template <class Extents>
//     struct mapping {
//         static_assert(Extents::rank() == 2, "layout_colmajor is a 2D layout");

//         // for convenience
//         using idx_t = typename Extents::size_type;

//         // constructor
//         mapping( const Extents& exts, idx_t ldim ) noexcept
//         : extents_(exts), ldim_(ldim) {}

//         // Default constructors
//         mapping() noexcept = default;
//         mapping(const mapping&) noexcept = default;
//         mapping(mapping&&) noexcept = default;
//         mapping& operator=(mapping const&) noexcept = default;
//         mapping& operator=(mapping&&) noexcept = default;
//         ~mapping() noexcept = default;

//         //------------------------------------------------------------
//         // Required members

//         constexpr idx_t
//         operator()(idx_t row, idx_t col) const noexcept {
//             return row + col * ldim_;
//         }

//         constexpr idx_t
//         required_span_size() const noexcept {
//             return extents_.extent(1) * ldim_;
//         }

//         // Mapping is always unique
//         static constexpr bool is_always_unique() noexcept { return true; }
//         constexpr bool is_unique() const noexcept { return true; }

//         // Only contiguous if extents_.extent(0) == ldim_
//         static constexpr bool is_always_contiguous() noexcept { return false; }
//         constexpr bool is_contiguous() const noexcept { return (extents_.extent(0) == ldim_); }

//         // Mapping is always strided: strides: (1,ldim_)
//         static constexpr bool is_always_strided() noexcept { return true; }
//         constexpr bool is_strided() const noexcept { return true; }

//         // Get the extents
//         inline constexpr Extents extents() const noexcept { return extents_; };

//         // Get the leading dimension
//         inline constexpr idx_t ldim() const noexcept { return ldim_; };

//         private:
//             Extents extents_;
//             idx_t ldim_; // leading dimension
//     };
// };

// -----------------------------------------------------------------------------
/** Returns a Matrix object representing a column major matrix
 * 
 * @param A                 serial data
 * @param m                 number of rows
 * @param n                 number of columns
 * @param lda               leading dimension 
 * 
 * @return mdspan< T, dextents<2>, layout_stride > 
 *      matrix object using the abstraction A(i,j) = i + j * lda
 */
template< typename T, typename integral_type >
inline constexpr auto colmajor_matrix(
    T* A, 
    dextents<2>::size_type m, 
    dextents<2>::size_type n, 
    integral_type lda )
{
    using extents_t = dextents<2>;
    using mapping = typename layout_stride::template mapping< extents_t >;

    const array<integral_type, 2> strides = {1,lda};

    return mdspan< T, extents_t, layout_stride > (
        A, mapping( extents_t(m,n), strides )
    );
}
template< typename T >
inline constexpr auto colmajor_matrix(
    T* A, 
    dextents<2>::size_type m, 
    dextents<2>::size_type n )
{
    return colmajor_matrix<T>( A, m, n, m );
}

template< typename T, typename integral_type >
inline constexpr auto vector(
    T* x,
    dextents<1>::size_type n,
    integral_type ldim )
{
    using extents_t = dextents<1>;
    using mapping = typename layout_stride::template mapping< extents_t >;

    const array<integral_type, 1> strides = {ldim};

    return mdspan< T, extents_t, layout_stride > (
        x, mapping( extents_t(n), strides )
    );
}
template< typename T >
inline constexpr auto vector(
    T* x,
    dextents<1>::size_type n )
{
    return vector<T>( x, n, 1 );
}

// Transpose
template< class ET, class Exts, class AP >
inline constexpr auto transpose(
    const mdspan<ET,Exts,layout_stride,AP>& A ) noexcept
{    
    using mapping = typename layout_stride::template mapping< Exts >;
    
    const auto m  = A.extent(0);
    const auto n  = A.extent(1);
    const auto s0 = A.stride(0);
    const auto s1 = A.stride(1);
    const array<typename Exts::size_type, 2> strides = {s1,s0};

    return mdspan<ET,Exts,layout_stride,AP>(
        A.data(), mapping( Exts(n,m), strides )
    );
}

} // namespace internal

#undef isSlice

} // namespace blas

#endif // __TBLAS_MDSPAN_HH__
