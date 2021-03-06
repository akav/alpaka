/**
 * \file
 * Copyright 2015-2017 Benjamin Worpitz
 *
 * This file is part of alpaka.
 *
 * alpaka is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alpaka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with alpaka.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <alpaka/alpaka.hpp>

#include <alpaka/test/mem/view/Iterator.hpp>

#if BOOST_COMP_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/test/unit_test.hpp>
#if BOOST_COMP_CLANG
    #pragma clang diagnostic pop
#endif

namespace alpaka
{
    //-----------------------------------------------------------------------------
    //! The test specifics.
    namespace test
    {
        //-----------------------------------------------------------------------------
        //! The test mem specifics.
        namespace mem
        {
            //-----------------------------------------------------------------------------
            namespace view
            {
                //-----------------------------------------------------------------------------
                template<
                    typename TElem,
                    typename TDim,
                    typename TIdx,
                    typename TDev,
                    typename TView>
                static auto testViewImmutable(
                    TView const & view,
                    TDev const & dev,
                    alpaka::vec::Vec<TDim, TIdx> const & extent,
                    alpaka::vec::Vec<TDim, TIdx> const & offset)
                -> void
                {
                    //-----------------------------------------------------------------------------
                    // alpaka::dev::traits::DevType
                    {
                        static_assert(
                            std::is_same<alpaka::dev::Dev<TView>, TDev>::value,
                            "The device type of the view has to be equal to the specified one.");
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::dev::traits::GetDev
                    {
                        BOOST_REQUIRE(
                            dev == alpaka::dev::getDev(view));
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::dim::traits::DimType
                    {
                        static_assert(
                            alpaka::dim::Dim<TView>::value == TDim::value,
                            "The dimensionality of the view has to be equal to the specified one.");
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::elem::traits::ElemType
                    {
                        static_assert(
                            std::is_same<alpaka::elem::Elem<TView>, TElem>::value,
                            "The element type of the view has to be equal to the specified one.");
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::extent::traits::GetExtent
                    {
                        BOOST_REQUIRE_EQUAL(
                            extent,
                            alpaka::extent::getExtentVec(view));
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::mem::view::traits::GetPitchBytes
                    {
                        // The pitches have to be at least as large as the values we calculate here.
                        auto pitchMinimum(alpaka::vec::Vec<alpaka::dim::DimInt<TDim::value + 1u>, TIdx>::ones());
                        // Initialize the pitch between two elements of the X dimension ...
                        pitchMinimum[TDim::value] = sizeof(TElem);
                        // ... and fill all the other dimensions.
                        for(TIdx i = TDim::value; i > static_cast<TIdx>(0u); --i)
                        {
                            pitchMinimum[i-1] = extent[i-1] * pitchMinimum[i];
                        }

                        auto const pitchView(alpaka::mem::view::getPitchBytesVec(view));

                        for(TIdx i = TDim::value; i > static_cast<TIdx>(0u); --i)
                        {
                            BOOST_REQUIRE_GE(
                                pitchView[i-1],
                                pitchMinimum[i-1]);
                        }
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::mem::view::traits::GetPtrNative
                    {
                        // The view is a const& so the pointer has to point to a const value.
                        using NativePtr = decltype(alpaka::mem::view::getPtrNative(view));
                        static_assert(
                            std::is_pointer<NativePtr>::value,
                            "The value returned by getPtrNative has to be a pointer.");
                        static_assert(
                            std::is_const<typename std::remove_pointer<NativePtr>::type>::value,
                            "The value returned by getPtrNative has to be const when the view is const.");

                        if(alpaka::extent::getExtentProduct(view) != static_cast<TIdx>(0u))
                        {
                            // The pointer is only required to be non-null when the extent is > 0.
                            TElem const * const invalidPtr(nullptr);
                            BOOST_REQUIRE_NE(
                                invalidPtr,
                                alpaka::mem::view::getPtrNative(view));
                        }
                        else
                        {
                            // When the extent is 0, the pointer is undefined but it should still be possible get it.
                            alpaka::mem::view::getPtrNative(view);
                        }
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::offset::traits::GetOffset
                    {
                        BOOST_REQUIRE_EQUAL(
                            offset,
                            alpaka::offset::getOffsetVec(view));
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::idx::traits::IdxType
                    {
                        static_assert(
                            std::is_same<alpaka::idx::Idx<TView>, TIdx>::value,
                            "The idx type of the view has to be equal to the specified one.");
                    }
                }

                //#############################################################################
                //! Compares element-wise that all bytes are set to the same value.
                struct VerifyBytesSetKernel
                {
                    ALPAKA_NO_HOST_ACC_WARNING
                    template<
                        typename TAcc,
                        typename TIter>
                    ALPAKA_FN_ACC void operator()(
                        TAcc const & acc,
                        TIter const & begin,
                        TIter const & end,
                        std::uint8_t const & byte) const
                    {
                        constexpr auto elemSizeInByte = sizeof(decltype(*begin));
                        (void)acc;
                        for(auto it = begin; it != end; ++it)
                        {
                            auto const& elem = *it;
                            auto const pBytes = reinterpret_cast<std::uint8_t const *>(&elem);
                            for(std::size_t i = 0u; i < elemSizeInByte; ++i)
                            {
                                BOOST_VERIFY(pBytes[i] == byte);
                            }
                        }
                    }
                };
                //-----------------------------------------------------------------------------
                template<
                    typename TAcc,
                    typename TView,
                    typename TQueue>
                static auto verifyBytesSet(
                    TQueue & queue,
                    TView const & view,
                    std::uint8_t const & byte)
                -> void
                {
                    using Dim = alpaka::dim::Dim<TView>;
                    using Idx = alpaka::idx::Idx<TView>;

                    using Vec = alpaka::vec::Vec<Dim, Idx>;
                    auto const elementsPerThread(Vec::ones());
                    auto const threadsPerBlock(Vec::ones());
                    auto const blocksPerGrid(Vec::ones());

                    auto const workdiv(
                        alpaka::workdiv::WorkDivMembers<Dim, Idx>(
                            blocksPerGrid,
                            threadsPerBlock,
                            elementsPerThread));
                    VerifyBytesSetKernel verifyBytesSet;
                    alpaka::kernel::exec<TAcc>(
                        queue,
                        workdiv,
                        verifyBytesSet,
                        alpaka::test::mem::view::begin(view),
                        alpaka::test::mem::view::end(view),
                        byte);
                    alpaka::wait::wait(queue);
                }

                //#############################################################################
                //! Compares iterators element-wise
#if BOOST_COMP_GNUC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"  // "comparing floating point with == or != is unsafe"
#endif
                struct VerifyViewsEqualKernel
                {
                    ALPAKA_NO_HOST_ACC_WARNING
                    template<
                        typename TAcc,
                        typename TIterA,
                        typename TIterB>
                    ALPAKA_FN_ACC void operator()(
                        TAcc const & acc,
                        TIterA beginA,
                        TIterA const & endA,
                        TIterB beginB) const
                    {
                        (void)acc;
                        for(; beginA != endA; ++beginA, ++beginB)
                        {
#if BOOST_COMP_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wfloat-equal" // "comparing floating point with == or != is unsafe"
#endif
                            BOOST_VERIFY(*beginA == *beginB);
#if BOOST_COMP_CLANG
    #pragma clang diagnostic pop
#endif
                        }
                    }
                };
#if BOOST_COMP_GNUC
    #pragma GCC diagnostic pop
#endif

                //-----------------------------------------------------------------------------
                template<
                    typename TAcc,
                    typename TViewB,
                    typename TViewA,
                    typename TQueue>
                static auto verifyViewsEqual(
                    TQueue & queue,
                    TViewA const & viewA,
                    TViewB const & viewB)
                -> void
                {
                    using DimA = alpaka::dim::Dim<TViewA>;
                    using DimB = alpaka::dim::Dim<TViewB>;
                    static_assert(DimA::value == DimB::value, "viewA and viewB are required to have identical Dim");
                    using SizeA = alpaka::idx::Idx<TViewA>;
                    using SizeB = alpaka::idx::Idx<TViewB>;
                    static_assert(std::is_same<SizeA, SizeB>::value, "viewA and viewB are required to have identical Idx");

                    using Vec = alpaka::vec::Vec<DimA, SizeA>;
                    auto const elementsPerThread(Vec::ones());
                    auto const threadsPerBlock(Vec::ones());
                    auto const blocksPerGrid(Vec::ones());

                    auto const workdiv(
                        alpaka::workdiv::WorkDivMembers<DimA, SizeA>(
                            blocksPerGrid,
                            threadsPerBlock,
                            elementsPerThread));
                    VerifyViewsEqualKernel verifyViewsEqualKernel;
                    alpaka::kernel::exec<TAcc>(
                        queue,
                        workdiv,
                        verifyViewsEqualKernel,
                        alpaka::test::mem::view::begin(viewA),
                        alpaka::test::mem::view::end(viewA),
                        alpaka::test::mem::view::begin(viewB));
                    alpaka::wait::wait(queue);
                }

                //-----------------------------------------------------------------------------
                //! Fills the given view with increasing values starting at 0.
                template<
                    typename TView,
                    typename TQueue>
                static auto iotaFillView(
                    TQueue & queue,
                    TView & view)
                -> void
                {
                    using Dim = alpaka::dim::Dim<TView>;
                    using Idx = alpaka::idx::Idx<TView>;

                    using DevHost = alpaka::dev::DevCpu;
                    using PltfHost = alpaka::pltf::Pltf<DevHost>;

                    using Elem = alpaka::elem::Elem<TView>;

                    using ViewPlainPtr = alpaka::mem::view::ViewPlainPtr<DevHost, Elem, Dim, Idx>;

                    DevHost const devHost(alpaka::pltf::getDevByIdx<PltfHost>(0));

                    auto const extent(alpaka::extent::getExtentVec(view));

                    // Init buf with increasing values
                    std::vector<Elem> v(static_cast<std::size_t>(extent.prod()), static_cast<Elem>(0));
                    std::iota(v.begin(), v.end(), static_cast<Elem>(0));
                    ViewPlainPtr plainBuf(v.data(), devHost, extent);

                    // Copy the generated content into the given view.
                    alpaka::mem::view::copy(queue, view, plainBuf, extent);

                    alpaka::wait::wait(queue);
                }

                //-----------------------------------------------------------------------------
                template<
                    typename TAcc,
                    typename TView,
                    typename TQueue>
                static auto testViewMutable(
                    TQueue & queue,
                    TView & view)
                -> void
                {
                    //-----------------------------------------------------------------------------
                    // alpaka::mem::view::traits::GetPtrNative
                    {
                        // The view is a non-const so the pointer has to point to a non-const value.
                        using NativePtr = decltype(alpaka::mem::view::getPtrNative(view));
                        static_assert(
                            std::is_pointer<NativePtr>::value,
                            "The value returned by getPtrNative has to be a pointer.");
                        static_assert(
                            !std::is_const<typename std::remove_pointer<NativePtr>::type>::value,
                            "The value returned by getPtrNative has to be non-const when the view is non-const.");
                    }

                    auto const extent(alpaka::extent::getExtentVec(view));

                    //-----------------------------------------------------------------------------
                    // alpaka::mem::view::set
                    {
                        std::uint8_t const byte(static_cast<uint8_t>(42u));
                        alpaka::mem::view::set(queue, view, byte, extent);
                        verifyBytesSet<TAcc>(queue, view, byte);
                    }

                    //-----------------------------------------------------------------------------
                    // alpaka::mem::view::copy
                    {
                        using Elem = alpaka::elem::Elem<TView>;
                        using Idx = alpaka::idx::Idx<TView>;

                        auto const devAcc = alpaka::dev::getDev(view);

                        //-----------------------------------------------------------------------------
                        // alpaka::mem::view::copy into given view
                        {
                            auto srcBufAcc(alpaka::mem::buf::alloc<Elem, Idx>(devAcc, extent));
                            iotaFillView(queue, srcBufAcc);
                            alpaka::mem::view::copy(queue, view, srcBufAcc, extent);
                            alpaka::test::mem::view::verifyViewsEqual<TAcc>(queue, view, srcBufAcc);
                        }

                        //-----------------------------------------------------------------------------
                        // alpaka::mem::view::copy from given view
                        {
                            auto dstBufAcc(alpaka::mem::buf::alloc<Elem, Idx>(devAcc, extent));
                            alpaka::mem::view::copy(queue, dstBufAcc, view, extent);
                            alpaka::test::mem::view::verifyViewsEqual<TAcc>(queue, dstBufAcc, view);
                        }
                    }
                }
            }
        }
    }
}
