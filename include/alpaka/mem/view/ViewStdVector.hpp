/**
* \file
* Copyright 2014-2015 Benjamin Worpitz
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

#include <alpaka/dev/DevCpu.hpp>
#include <alpaka/mem/view/Traits.hpp>
#include <alpaka/pltf/PltfCpu.hpp>
#include <alpaka/core/Common.hpp>

#include <boost/core/ignore_unused.hpp>

#include <vector>

namespace alpaka
{
    namespace dev
    {
        namespace traits
        {
            //#############################################################################
            //! The std::vector device type trait specialization.
            template<
                typename TElem,
                typename TAllocator>
            struct DevType<
                std::vector<TElem, TAllocator>>
            {
                using type = dev::DevCpu;
            };

            //#############################################################################
            //! The std::vector device get trait specialization.
            template<
                typename TElem,
                typename TAllocator>
            struct GetDev<
                std::vector<TElem, TAllocator>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto getDev(
                    std::vector<TElem, TAllocator> const & view)
                -> dev::DevCpu
                {
                    boost::ignore_unused(view);
                    return pltf::getDevByIdx<pltf::PltfCpu>(0u);
                }
            };
        }
    }
    namespace dim
    {
        namespace traits
        {
            //#############################################################################
            //! The std::vector dimension getter trait specialization.
            template<
                typename TElem,
                typename TAllocator>
            struct DimType<
                std::vector<TElem, TAllocator>>
            {
                using type = dim::DimInt<1u>;
            };
        }
    }
    namespace elem
    {
        namespace traits
        {
            //#############################################################################
            //! The std::vector memory element type get trait specialization.
            template<
                typename TElem,
                typename TAllocator>
            struct ElemType<
                std::vector<TElem, TAllocator>>
            {
                using type = TElem;
            };
        }
    }
    namespace extent
    {
        namespace traits
        {
            //#############################################################################
            //! The std::vector width get trait specialization.
            template<
                typename TElem,
                typename TAllocator>
            struct GetExtent<
                dim::DimInt<0u>,
                std::vector<TElem, TAllocator>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto getExtent(
                    std::vector<TElem, TAllocator> const & extent)
                -> idx::Idx<std::vector<TElem, TAllocator>>
                {
                    return extent.size();
                }
            };
        }
    }
    namespace mem
    {
        namespace view
        {
            namespace traits
            {
                //#############################################################################
                //! The std::vector native pointer get trait specialization.
                template<
                    typename TElem,
                    typename TAllocator>
                struct GetPtrNative<
                    std::vector<TElem, TAllocator>>
                {
                    //-----------------------------------------------------------------------------
                    ALPAKA_FN_HOST static auto getPtrNative(
                        std::vector<TElem, TAllocator> const & view)
                    -> TElem const *
                    {
                        return view.data();
                    }
                    //-----------------------------------------------------------------------------
                    ALPAKA_FN_HOST static auto getPtrNative(
                        std::vector<TElem, TAllocator> & view)
                    -> TElem *
                    {
                        return view.data();
                    }
                };

                //#############################################################################
                //! The std::vector pitch get trait specialization.
                template<
                    typename TElem,
                    typename TAllocator>
                struct GetPitchBytes<
                    dim::DimInt<0u>,
                    std::vector<TElem, TAllocator>>
                {
                    //-----------------------------------------------------------------------------
                    ALPAKA_FN_HOST static auto getPitchBytes(
                        std::vector<TElem, TAllocator> const & pitch)
                    -> idx::Idx<std::vector<TElem, TAllocator>>
                    {
                        return sizeof(TElem) * pitch.size();
                    }
                };
            }
        }
    }
    namespace offset
    {
        namespace traits
        {
            //#############################################################################
            //! The std::vector offset get trait specialization.
            template<
                typename TIdx,
                typename TElem,
                typename TAllocator>
            struct GetOffset<
                TIdx,
                std::vector<TElem, TAllocator>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto getOffset(
                    std::vector<TElem, TAllocator> const &)
                -> idx::Idx<std::vector<TElem, TAllocator>>
                {
                    return 0u;
                }
            };
        }
    }
    namespace idx
    {
        namespace traits
        {
            //#############################################################################
            //! The std::vector idx type trait specialization.
            template<
                typename TElem,
                typename TAllocator>
            struct IdxType<
                std::vector<TElem, TAllocator>>
            {
                using type = std::size_t;
            };
        }
    }
}
