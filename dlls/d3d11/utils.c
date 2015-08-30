/*
 * Copyright 2008 Henri Verbeet for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

#include "config.h"
#include "wine/port.h"

#include "d3d11_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(d3d11);

#define WINE_D3D10_TO_STR(x) case x: return #x

const char *debug_d3d10_primitive_topology(D3D10_PRIMITIVE_TOPOLOGY topology)
{
    switch (topology)
    {
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
        WINE_D3D10_TO_STR(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ);
        default:
            FIXME("Unrecognized D3D10_PRIMITIVE_TOPOLOGY %#x\n", topology);
            return "unrecognized";
    }
}

const char *debug_dxgi_format(DXGI_FORMAT format)
{
    switch(format)
    {
        WINE_D3D10_TO_STR(DXGI_FORMAT_UNKNOWN);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32A32_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32A32_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32A32_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32A32_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32B32_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16B16A16_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16B16A16_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16B16A16_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16B16A16_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16B16A16_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16B16A16_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G32_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32G8X24_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R10G10B10A2_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R10G10B10A2_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R10G10B10A2_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R11G11B10_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8B8A8_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8B8A8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8B8A8_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8B8A8_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8B8A8_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16G16_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_D32_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R32_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R24G8_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_D24_UNORM_S8_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_X24_TYPELESS_G8_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16_FLOAT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_D16_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R16_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8_UINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8_SINT);
        WINE_D3D10_TO_STR(DXGI_FORMAT_A8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R1_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R9G9B9E5_SHAREDEXP);
        WINE_D3D10_TO_STR(DXGI_FORMAT_R8G8_B8G8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_G8R8_G8B8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC1_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC1_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC1_UNORM_SRGB);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC2_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC2_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC2_UNORM_SRGB);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC3_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC3_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC3_UNORM_SRGB);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC4_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC4_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC4_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC5_TYPELESS);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC5_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_BC5_SNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_B5G6R5_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_B5G5R5A1_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_B8G8R8A8_UNORM);
        WINE_D3D10_TO_STR(DXGI_FORMAT_B8G8R8X8_UNORM);
        default:
            FIXME("Unrecognized DXGI_FORMAT %#x\n", format);
            return "unrecognized";
    }
}

#undef WINE_D3D10_TO_STR

DXGI_FORMAT dxgi_format_from_wined3dformat(enum wined3d_format_id format)
{
    switch(format)
    {
        case WINED3DFMT_UNKNOWN: return DXGI_FORMAT_UNKNOWN;
        case WINED3DFMT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
        case WINED3DFMT_R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case WINED3DFMT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
        case WINED3DFMT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
        case WINED3DFMT_R32G32B32_TYPELESS: return DXGI_FORMAT_R32G32B32_TYPELESS;
        case WINED3DFMT_R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
        case WINED3DFMT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
        case WINED3DFMT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
        case WINED3DFMT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case WINED3DFMT_R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case WINED3DFMT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case WINED3DFMT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
        case WINED3DFMT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
        case WINED3DFMT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
        case WINED3DFMT_R32G32_TYPELESS: return DXGI_FORMAT_R32G32_TYPELESS;
        case WINED3DFMT_R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
        case WINED3DFMT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
        case WINED3DFMT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
        case WINED3DFMT_R32G8X24_TYPELESS: return DXGI_FORMAT_R32G8X24_TYPELESS;
        case WINED3DFMT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case WINED3DFMT_R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        case WINED3DFMT_X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        case WINED3DFMT_R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case WINED3DFMT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case WINED3DFMT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
        case WINED3DFMT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
        case WINED3DFMT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case WINED3DFMT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case WINED3DFMT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case WINED3DFMT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
        case WINED3DFMT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
        case WINED3DFMT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
        case WINED3DFMT_R16G16_TYPELESS: return DXGI_FORMAT_R16G16_TYPELESS;
        case WINED3DFMT_R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
        case WINED3DFMT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
        case WINED3DFMT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
        case WINED3DFMT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
        case WINED3DFMT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
        case WINED3DFMT_R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
        case WINED3DFMT_D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
        case WINED3DFMT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case WINED3DFMT_R32_UINT: return DXGI_FORMAT_R32_UINT;
        case WINED3DFMT_R32_SINT: return DXGI_FORMAT_R32_SINT;
        case WINED3DFMT_R24G8_TYPELESS: return DXGI_FORMAT_R24G8_TYPELESS;
        case WINED3DFMT_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case WINED3DFMT_R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case WINED3DFMT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        case WINED3DFMT_R8G8_TYPELESS: return DXGI_FORMAT_R8G8_TYPELESS;
        case WINED3DFMT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
        case WINED3DFMT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
        case WINED3DFMT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
        case WINED3DFMT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
        case WINED3DFMT_R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
        case WINED3DFMT_R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
        case WINED3DFMT_D16_UNORM: return DXGI_FORMAT_D16_UNORM;
        case WINED3DFMT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
        case WINED3DFMT_R16_UINT: return DXGI_FORMAT_R16_UINT;
        case WINED3DFMT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
        case WINED3DFMT_R16_SINT: return DXGI_FORMAT_R16_SINT;
        case WINED3DFMT_R8_TYPELESS: return DXGI_FORMAT_R8_TYPELESS;
        case WINED3DFMT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
        case WINED3DFMT_R8_UINT: return DXGI_FORMAT_R8_UINT;
        case WINED3DFMT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
        case WINED3DFMT_R8_SINT: return DXGI_FORMAT_R8_SINT;
        case WINED3DFMT_A8_UNORM: return DXGI_FORMAT_A8_UNORM;
        case WINED3DFMT_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
        case WINED3DFMT_R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
        case WINED3DFMT_R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
        case WINED3DFMT_G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
        case WINED3DFMT_BC1_TYPELESS: return DXGI_FORMAT_BC1_TYPELESS;
        case WINED3DFMT_BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
        case WINED3DFMT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
        case WINED3DFMT_BC2_TYPELESS: return DXGI_FORMAT_BC2_TYPELESS;
        case WINED3DFMT_BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
        case WINED3DFMT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
        case WINED3DFMT_BC3_TYPELESS: return DXGI_FORMAT_BC3_TYPELESS;
        case WINED3DFMT_BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
        case WINED3DFMT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
        case WINED3DFMT_BC4_TYPELESS: return DXGI_FORMAT_BC4_TYPELESS;
        case WINED3DFMT_BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
        case WINED3DFMT_BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
        case WINED3DFMT_BC5_TYPELESS: return DXGI_FORMAT_BC5_TYPELESS;
        case WINED3DFMT_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
        case WINED3DFMT_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
        case WINED3DFMT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
        case WINED3DFMT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
        case WINED3DFMT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case WINED3DFMT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
        default:
            FIXME("Unhandled wined3d format %#x.\n", format);
            return DXGI_FORMAT_UNKNOWN;
    }
}

enum wined3d_format_id wined3dformat_from_dxgi_format(DXGI_FORMAT format)
{
    switch(format)
    {
        case DXGI_FORMAT_UNKNOWN: return WINED3DFMT_UNKNOWN;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS: return WINED3DFMT_R32G32B32A32_TYPELESS;
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return WINED3DFMT_R32G32B32A32_FLOAT;
        case DXGI_FORMAT_R32G32B32A32_UINT: return WINED3DFMT_R32G32B32A32_UINT;
        case DXGI_FORMAT_R32G32B32A32_SINT: return WINED3DFMT_R32G32B32A32_SINT;
        case DXGI_FORMAT_R32G32B32_TYPELESS: return WINED3DFMT_R32G32B32_TYPELESS;
        case DXGI_FORMAT_R32G32B32_FLOAT: return WINED3DFMT_R32G32B32_FLOAT;
        case DXGI_FORMAT_R32G32B32_UINT: return WINED3DFMT_R32G32B32_UINT;
        case DXGI_FORMAT_R32G32B32_SINT: return WINED3DFMT_R32G32B32_SINT;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS: return WINED3DFMT_R16G16B16A16_TYPELESS;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return WINED3DFMT_R16G16B16A16_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return WINED3DFMT_R16G16B16A16_UNORM;
        case DXGI_FORMAT_R16G16B16A16_UINT: return WINED3DFMT_R16G16B16A16_UINT;
        case DXGI_FORMAT_R16G16B16A16_SNORM: return WINED3DFMT_R16G16B16A16_SNORM;
        case DXGI_FORMAT_R16G16B16A16_SINT: return WINED3DFMT_R16G16B16A16_SINT;
        case DXGI_FORMAT_R32G32_TYPELESS: return WINED3DFMT_R32G32_TYPELESS;
        case DXGI_FORMAT_R32G32_FLOAT: return WINED3DFMT_R32G32_FLOAT;
        case DXGI_FORMAT_R32G32_UINT: return WINED3DFMT_R32G32_UINT;
        case DXGI_FORMAT_R32G32_SINT: return WINED3DFMT_R32G32_SINT;
        case DXGI_FORMAT_R32G8X24_TYPELESS: return WINED3DFMT_R32G8X24_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return WINED3DFMT_D32_FLOAT_S8X24_UINT;
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return WINED3DFMT_R32_FLOAT_X8X24_TYPELESS;
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return WINED3DFMT_X32_TYPELESS_G8X24_UINT;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS: return WINED3DFMT_R10G10B10A2_TYPELESS;
        case DXGI_FORMAT_R10G10B10A2_UNORM: return WINED3DFMT_R10G10B10A2_UNORM;
        case DXGI_FORMAT_R10G10B10A2_UINT: return WINED3DFMT_R10G10B10A2_UINT;
        case DXGI_FORMAT_R11G11B10_FLOAT: return WINED3DFMT_R11G11B10_FLOAT;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS: return WINED3DFMT_R8G8B8A8_TYPELESS;
        case DXGI_FORMAT_R8G8B8A8_UNORM: return WINED3DFMT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return WINED3DFMT_R8G8B8A8_UNORM_SRGB;
        case DXGI_FORMAT_R8G8B8A8_UINT: return WINED3DFMT_R8G8B8A8_UINT;
        case DXGI_FORMAT_R8G8B8A8_SNORM: return WINED3DFMT_R8G8B8A8_SNORM;
        case DXGI_FORMAT_R8G8B8A8_SINT: return WINED3DFMT_R8G8B8A8_SINT;
        case DXGI_FORMAT_R16G16_TYPELESS: return WINED3DFMT_R16G16_TYPELESS;
        case DXGI_FORMAT_R16G16_FLOAT: return WINED3DFMT_R16G16_FLOAT;
        case DXGI_FORMAT_R16G16_UNORM: return WINED3DFMT_R16G16_UNORM;
        case DXGI_FORMAT_R16G16_UINT: return WINED3DFMT_R16G16_UINT;
        case DXGI_FORMAT_R16G16_SNORM: return WINED3DFMT_R16G16_SNORM;
        case DXGI_FORMAT_R16G16_SINT: return WINED3DFMT_R16G16_SINT;
        case DXGI_FORMAT_R32_TYPELESS: return WINED3DFMT_R32_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT: return WINED3DFMT_D32_FLOAT;
        case DXGI_FORMAT_R32_FLOAT: return WINED3DFMT_R32_FLOAT;
        case DXGI_FORMAT_R32_UINT: return WINED3DFMT_R32_UINT;
        case DXGI_FORMAT_R32_SINT: return WINED3DFMT_R32_SINT;
        case DXGI_FORMAT_R24G8_TYPELESS: return WINED3DFMT_R24G8_TYPELESS;
        case DXGI_FORMAT_D24_UNORM_S8_UINT: return WINED3DFMT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return WINED3DFMT_R24_UNORM_X8_TYPELESS;
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return WINED3DFMT_X24_TYPELESS_G8_UINT;
        case DXGI_FORMAT_R8G8_TYPELESS: return WINED3DFMT_R8G8_TYPELESS;
        case DXGI_FORMAT_R8G8_UNORM: return WINED3DFMT_R8G8_UNORM;
        case DXGI_FORMAT_R8G8_UINT: return WINED3DFMT_R8G8_UINT;
        case DXGI_FORMAT_R8G8_SNORM: return WINED3DFMT_R8G8_SNORM;
        case DXGI_FORMAT_R8G8_SINT: return WINED3DFMT_R8G8_SINT;
        case DXGI_FORMAT_R16_TYPELESS: return WINED3DFMT_R16_TYPELESS;
        case DXGI_FORMAT_R16_FLOAT: return WINED3DFMT_R16_FLOAT;
        case DXGI_FORMAT_D16_UNORM: return WINED3DFMT_D16_UNORM;
        case DXGI_FORMAT_R16_UNORM: return WINED3DFMT_R16_UNORM;
        case DXGI_FORMAT_R16_UINT: return WINED3DFMT_R16_UINT;
        case DXGI_FORMAT_R16_SNORM: return WINED3DFMT_R16_SNORM;
        case DXGI_FORMAT_R16_SINT: return WINED3DFMT_R16_SINT;
        case DXGI_FORMAT_R8_TYPELESS: return WINED3DFMT_R8_TYPELESS;
        case DXGI_FORMAT_R8_UNORM: return WINED3DFMT_R8_UNORM;
        case DXGI_FORMAT_R8_UINT: return WINED3DFMT_R8_UINT;
        case DXGI_FORMAT_R8_SNORM: return WINED3DFMT_R8_SNORM;
        case DXGI_FORMAT_R8_SINT: return WINED3DFMT_R8_SINT;
        case DXGI_FORMAT_A8_UNORM: return WINED3DFMT_A8_UNORM;
        case DXGI_FORMAT_R1_UNORM: return WINED3DFMT_R1_UNORM;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return WINED3DFMT_R9G9B9E5_SHAREDEXP;
        case DXGI_FORMAT_R8G8_B8G8_UNORM: return WINED3DFMT_R8G8_B8G8_UNORM;
        case DXGI_FORMAT_G8R8_G8B8_UNORM: return WINED3DFMT_G8R8_G8B8_UNORM;
        case DXGI_FORMAT_BC1_TYPELESS: return WINED3DFMT_BC1_TYPELESS;
        case DXGI_FORMAT_BC1_UNORM: return WINED3DFMT_BC1_UNORM;
        case DXGI_FORMAT_BC1_UNORM_SRGB: return WINED3DFMT_BC1_UNORM_SRGB;
        case DXGI_FORMAT_BC2_TYPELESS: return WINED3DFMT_BC2_TYPELESS;
        case DXGI_FORMAT_BC2_UNORM: return WINED3DFMT_BC2_UNORM;
        case DXGI_FORMAT_BC2_UNORM_SRGB: return WINED3DFMT_BC2_UNORM_SRGB;
        case DXGI_FORMAT_BC3_TYPELESS: return WINED3DFMT_BC3_TYPELESS;
        case DXGI_FORMAT_BC3_UNORM: return WINED3DFMT_BC3_UNORM;
        case DXGI_FORMAT_BC3_UNORM_SRGB: return WINED3DFMT_BC3_UNORM_SRGB;
        case DXGI_FORMAT_BC4_TYPELESS: return WINED3DFMT_BC4_TYPELESS;
        case DXGI_FORMAT_BC4_UNORM: return WINED3DFMT_BC4_UNORM;
        case DXGI_FORMAT_BC4_SNORM: return WINED3DFMT_BC4_SNORM;
        case DXGI_FORMAT_BC5_TYPELESS: return WINED3DFMT_BC5_TYPELESS;
        case DXGI_FORMAT_BC5_UNORM: return WINED3DFMT_BC5_UNORM;
        case DXGI_FORMAT_BC5_SNORM: return WINED3DFMT_BC5_SNORM;
        case DXGI_FORMAT_B5G6R5_UNORM: return WINED3DFMT_B5G6R5_UNORM;
        case DXGI_FORMAT_B5G5R5A1_UNORM: return WINED3DFMT_B5G5R5A1_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM: return WINED3DFMT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_UNORM: return WINED3DFMT_B8G8R8X8_UNORM;
        default:
            FIXME("Unhandled DXGI_FORMAT %#x\n", format);
            return WINED3DFMT_UNKNOWN;
    }
}

DWORD wined3d_usage_from_d3d11(UINT bind_flags, enum D3D11_USAGE usage)
{
    static const DWORD handled = D3D11_BIND_SHADER_RESOURCE
            | D3D11_BIND_RENDER_TARGET
            | D3D11_BIND_DEPTH_STENCIL;
    DWORD wined3d_usage = 0;

    if (bind_flags & D3D11_BIND_SHADER_RESOURCE)
        wined3d_usage |= WINED3DUSAGE_TEXTURE;
    if (bind_flags & D3D11_BIND_RENDER_TARGET)
        wined3d_usage |= WINED3DUSAGE_RENDERTARGET;
    if (bind_flags & D3D11_BIND_DEPTH_STENCIL)
        wined3d_usage |= WINED3DUSAGE_DEPTHSTENCIL;
    if (bind_flags & ~handled)
        FIXME("Unhandled bind flags %#x.\n", usage & ~handled);

    if (usage == D3D11_USAGE_DYNAMIC)
        wined3d_usage |= WINED3DUSAGE_DYNAMIC;

    return wined3d_usage;
}

DWORD wined3d_usage_from_d3d10core(UINT bind_flags, enum D3D10_USAGE usage)
{
    UINT d3d11_bind_flags = d3d11_bind_flags_from_d3d10_bind_flags(bind_flags);
    enum D3D11_USAGE d3d11_usage = d3d11_usage_from_d3d10_usage(usage);
    return wined3d_usage_from_d3d11(d3d11_bind_flags, d3d11_usage);
}

enum D3D11_USAGE d3d11_usage_from_d3d10_usage(enum D3D10_USAGE usage)
{
    switch (usage)
    {
        case D3D10_USAGE_DEFAULT: return D3D11_USAGE_DEFAULT;
        case D3D10_USAGE_IMMUTABLE: return D3D11_USAGE_IMMUTABLE;
        case D3D10_USAGE_DYNAMIC: return D3D11_USAGE_DYNAMIC;
        case D3D10_USAGE_STAGING: return D3D11_USAGE_STAGING;
        default:
            FIXME("Unhandled usage %#x.\n", usage);
            return D3D11_USAGE_DEFAULT;
    }
}

enum D3D10_USAGE d3d10_usage_from_d3d11_usage(enum D3D11_USAGE usage)
{
    switch (usage)
    {
        case D3D11_USAGE_DEFAULT: return D3D10_USAGE_DEFAULT;
        case D3D11_USAGE_IMMUTABLE: return D3D10_USAGE_IMMUTABLE;
        case D3D11_USAGE_DYNAMIC: return D3D10_USAGE_DYNAMIC;
        case D3D11_USAGE_STAGING: return D3D10_USAGE_STAGING;
        default:
            FIXME("Unhandled usage %#x.\n", usage);
            return D3D10_USAGE_DEFAULT;
    }
}

UINT d3d11_bind_flags_from_d3d10_bind_flags(UINT bind_flags)
{
    static const UINT handled_flags = D3D10_BIND_VERTEX_BUFFER
            | D3D10_BIND_INDEX_BUFFER
            | D3D10_BIND_CONSTANT_BUFFER
            | D3D10_BIND_SHADER_RESOURCE
            | D3D10_BIND_STREAM_OUTPUT
            | D3D10_BIND_RENDER_TARGET
            | D3D10_BIND_DEPTH_STENCIL;
    UINT d3d11_bind_flags = bind_flags & handled_flags;

    if (bind_flags & ~handled_flags)
        FIXME("Unhandled bind flags %#x.\n", bind_flags & ~handled_flags);

    return d3d11_bind_flags;
}

UINT d3d10_bind_flags_from_d3d11_bind_flags(UINT bind_flags)
{
    static const UINT handled_flags = D3D11_BIND_VERTEX_BUFFER
            | D3D11_BIND_INDEX_BUFFER
            | D3D11_BIND_CONSTANT_BUFFER
            | D3D11_BIND_SHADER_RESOURCE
            | D3D11_BIND_STREAM_OUTPUT
            | D3D11_BIND_RENDER_TARGET
            | D3D11_BIND_DEPTH_STENCIL
            | D3D11_BIND_UNORDERED_ACCESS
            | D3D11_BIND_DECODER
            | D3D11_BIND_VIDEO_ENCODER;
    UINT d3d10_bind_flags = bind_flags & handled_flags;

    if (bind_flags & ~handled_flags)
        FIXME("Unhandled bind flags %#x.\n", bind_flags & ~handled_flags);

    return d3d10_bind_flags;
}

UINT d3d11_cpu_access_flags_from_d3d10_cpu_access_flags(UINT cpu_access_flags)
{
    static const UINT handled_flags = D3D10_CPU_ACCESS_WRITE
            | D3D10_CPU_ACCESS_READ;
    UINT d3d11_cpu_access_flags = cpu_access_flags & handled_flags;

    if (cpu_access_flags & ~handled_flags)
        FIXME("Unhandled cpu access flags %#x.\n", cpu_access_flags & ~handled_flags);

    return d3d11_cpu_access_flags;
}

UINT d3d10_cpu_access_flags_from_d3d11_cpu_access_flags(UINT cpu_access_flags)
{
    static const UINT handled_flags = D3D11_CPU_ACCESS_WRITE
            | D3D11_CPU_ACCESS_READ;
    UINT d3d10_cpu_access_flags = cpu_access_flags & handled_flags;

    if (cpu_access_flags & ~handled_flags)
        FIXME("Unhandled cpu access flags %#x.\n", cpu_access_flags & ~handled_flags);

    return d3d10_cpu_access_flags;
}

UINT d3d11_resource_misc_flags_from_d3d10_resource_misc_flags(UINT resource_misc_flags)
{
    static const UINT bitwise_identical_flags = D3D10_RESOURCE_MISC_GENERATE_MIPS
            | D3D10_RESOURCE_MISC_SHARED
            | D3D10_RESOURCE_MISC_TEXTURECUBE;
    const UINT handled_flags = bitwise_identical_flags
            | D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX
            | D3D10_RESOURCE_MISC_GDI_COMPATIBLE;
    UINT d3d11_resource_misc_flags = resource_misc_flags & bitwise_identical_flags;

    if (resource_misc_flags & D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX)
        d3d11_resource_misc_flags |= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    if (resource_misc_flags & D3D10_RESOURCE_MISC_GDI_COMPATIBLE)
        d3d11_resource_misc_flags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    if (resource_misc_flags & ~handled_flags)
        FIXME("Unhandled resource misc flags %#x.\n", resource_misc_flags & ~handled_flags);

    return d3d11_resource_misc_flags;
}

UINT d3d10_resource_misc_flags_from_d3d11_resource_misc_flags(UINT resource_misc_flags)
{
    static const UINT bitwise_identical_flags = D3D11_RESOURCE_MISC_GENERATE_MIPS
            | D3D11_RESOURCE_MISC_SHARED
            | D3D11_RESOURCE_MISC_TEXTURECUBE
            | D3D11_RESOURCE_MISC_BUFFER_STRUCTURED
            | D3D11_RESOURCE_MISC_RESOURCE_CLAMP
            | D3D11_RESOURCE_MISC_SHARED_NTHANDLE
            | D3D11_RESOURCE_MISC_RESTRICTED_CONTENT
            | D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE
            | D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER
            | D3D11_RESOURCE_MISC_GUARDED;
    const UINT handled_flags = bitwise_identical_flags
            | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
            | D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
    UINT d3d10_resource_misc_flags = resource_misc_flags & bitwise_identical_flags;

    if (resource_misc_flags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX)
        d3d10_resource_misc_flags |= D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    if (resource_misc_flags & D3D11_RESOURCE_MISC_GDI_COMPATIBLE)
        d3d10_resource_misc_flags |= D3D10_RESOURCE_MISC_GDI_COMPATIBLE;

    if (resource_misc_flags & ~handled_flags)
        FIXME("Unhandled resource misc flags #%x.\n", resource_misc_flags & ~handled_flags);

    return d3d10_resource_misc_flags;
}

struct wined3d_resource *wined3d_resource_from_resource(ID3D10Resource *resource)
{
    D3D10_RESOURCE_DIMENSION dimension;

    ID3D10Resource_GetType(resource, &dimension);

    switch (dimension)
    {
        case D3D10_RESOURCE_DIMENSION_BUFFER:
            return wined3d_buffer_get_resource(unsafe_impl_from_ID3D10Buffer(
                    (ID3D10Buffer *)resource)->wined3d_buffer);

        case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
            return wined3d_texture_get_resource(unsafe_impl_from_ID3D10Texture2D(
                    (ID3D10Texture2D *)resource)->wined3d_texture);

        default:
            FIXME("Unhandled resource dimension %#x.\n", dimension);
            return NULL;
    }
}

DWORD wined3d_map_flags_from_d3d10_map_type(D3D10_MAP map_type)
{
    switch (map_type)
    {
        case D3D10_MAP_READ_WRITE:
            return 0;

        case D3D10_MAP_READ:
            return WINED3D_MAP_READONLY;

        case D3D10_MAP_WRITE_DISCARD:
            return WINED3D_MAP_DISCARD;

        case D3D10_MAP_WRITE_NO_OVERWRITE:
            return WINED3D_MAP_NOOVERWRITE;

        default:
            FIXME("Unhandled map_type %#x.\n", map_type);
            return 0;
    }
}

HRESULT d3d10_get_private_data(struct wined3d_private_store *store,
        REFGUID guid, UINT *data_size, void *data)
{
    const struct wined3d_private_data *stored_data;
    DWORD size_in;

    if (!data_size)
        return E_INVALIDARG;

    wined3d_mutex_lock();
    if (!(stored_data = wined3d_private_store_get_private_data(store, guid)))
    {
        *data_size = 0;
        wined3d_mutex_unlock();
        return DXGI_ERROR_NOT_FOUND;
    }

    size_in = *data_size;
    *data_size = stored_data->size;
    if (!data)
    {
        wined3d_mutex_unlock();
        return S_OK;
    }
    if (size_in < stored_data->size)
    {
        wined3d_mutex_unlock();
        return DXGI_ERROR_MORE_DATA;
    }

    if (stored_data->flags & WINED3DSPD_IUNKNOWN)
        IUnknown_AddRef(stored_data->content.object);
    memcpy(data, stored_data->content.data, stored_data->size);

    wined3d_mutex_unlock();
    return S_OK;
}

HRESULT d3d10_set_private_data(struct wined3d_private_store *store,
        REFGUID guid, UINT data_size, const void *data)
{
    struct wined3d_private_data *entry;
    HRESULT hr;

    wined3d_mutex_lock();
    if (!data)
    {
        if (!(entry = wined3d_private_store_get_private_data(store, guid)))
        {
            wined3d_mutex_unlock();
            return S_FALSE;
        }
        wined3d_private_store_free_private_data(store, entry);
        wined3d_mutex_unlock();

        return S_OK;
    }

    hr = wined3d_private_store_set_private_data(store, guid, data, data_size, 0);
    wined3d_mutex_unlock();

    return hr;
}

HRESULT d3d10_set_private_data_interface(struct wined3d_private_store *store,
        REFGUID guid, const IUnknown *object)
{
    HRESULT hr;

    if (!object)
        return d3d10_set_private_data(store, guid, sizeof(object), &object);

    wined3d_mutex_lock();
    hr = wined3d_private_store_set_private_data(store,
            guid, object, sizeof(object), WINED3DSPD_IUNKNOWN);
    wined3d_mutex_unlock();

    return hr;
}

void skip_dword_unknown(const char **ptr, unsigned int count)
{
    unsigned int i;
    DWORD d;

    FIXME("Skipping %u unknown DWORDs:\n", count);
    for (i = 0; i < count; ++i)
    {
        read_dword(ptr, &d);
        FIXME("\t0x%08x\n", d);
    }
}

HRESULT parse_dxbc(const char *data, SIZE_T data_size,
        HRESULT (*chunk_handler)(const char *data, DWORD data_size, DWORD tag, void *ctx), void *ctx)
{
    const char *ptr = data;
    HRESULT hr = S_OK;
    DWORD chunk_count;
    DWORD total_size;
    unsigned int i;
    DWORD tag;

    read_dword(&ptr, &tag);
    TRACE("tag: %s.\n", debugstr_an((const char *)&tag, 4));

    if (tag != TAG_DXBC)
    {
        WARN("Wrong tag.\n");
        return E_INVALIDARG;
    }

    /* checksum? */
    skip_dword_unknown(&ptr, 4);

    skip_dword_unknown(&ptr, 1);

    read_dword(&ptr, &total_size);
    TRACE("total size: %#x\n", total_size);

    read_dword(&ptr, &chunk_count);
    TRACE("chunk count: %#x\n", chunk_count);

    for (i = 0; i < chunk_count; ++i)
    {
        DWORD chunk_tag, chunk_size;
        const char *chunk_ptr;
        DWORD chunk_offset;

        read_dword(&ptr, &chunk_offset);
        TRACE("chunk %u at offset %#x\n", i, chunk_offset);

        chunk_ptr = data + chunk_offset;

        read_dword(&chunk_ptr, &chunk_tag);
        read_dword(&chunk_ptr, &chunk_size);

        hr = chunk_handler(chunk_ptr, chunk_size, chunk_tag, ctx);
        if (FAILED(hr)) break;
    }

    return hr;
}
