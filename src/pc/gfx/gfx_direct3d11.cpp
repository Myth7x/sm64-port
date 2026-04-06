#ifdef ENABLE_DX11

#include <cstdio>
#include <vector>
#include <cmath>

#include <windows.h>
#include <versionhelpers.h>
#include <wrl/client.h>

#include <dxgi1_3.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include "gfx_cc.h"
#include "gfx_window_manager_api.h"
#include "gfx_rendering_api.h"
#include "gfx_direct3d_common.h"

#define DECLARE_GFX_DXGI_FUNCTIONS
#include "gfx_dxgi.h"

#include "gfx_screen_config.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_menu/imgui_menu.h"
#include "level_lights.h"
#include "gfx_pc.h"
#include "sm64_udp/actor_manager.h"

#define THREE_POINT_FILTERING 0
#define DEBUG_D3D 0

#define GFX_D3D11_MAX_BUFFERED 8192U
#define GFX_D3D11_MAX_SHADERS  512U
#define GFX_D3D11_VBO_BYTES    (GFX_D3D11_MAX_BUFFERED * 29U * 3U * sizeof(float))

#define GFX_D3D11_SHADER_VS "vs_4_0"
#define GFX_D3D11_SHADER_PS "ps_4_0"

#define GFX_D3D11_SHADOW_VBO_MAX_VERTS 900000U

using namespace Microsoft::WRL; // For ComPtr

namespace {

struct PerFrameCB {
    uint32_t noise_frame;
    float noise_scale_x;
    float noise_scale_y;
    uint32_t padding;
};

struct PerDrawCB {
    struct Texture {
        uint32_t width;
        uint32_t height;
        uint32_t linear_filtering;
        uint32_t padding;
    } textures[2];
};

struct TextureData {
    ComPtr<ID3D11ShaderResourceView> resource_view;
    ComPtr<ID3D11SamplerState> sampler_state;
    uint32_t width;
    uint32_t height;
    bool linear_filtering;
};

struct ShaderProgramD3D11 {
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11BlendState> blend_state;

    uint32_t shader_id;
    uint8_t num_inputs;
    uint8_t num_floats;
    bool used_textures[2];
};

static struct {
    HMODULE d3d11_module;
    PFN_D3D11_CREATE_DEVICE D3D11CreateDevice;
    
    HMODULE d3dcompiler_module;
    pD3DCompile D3DCompile;
    
    D3D_FEATURE_LEVEL feature_level;
    
    ComPtr<ID3D11Device> device;
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11RenderTargetView> backbuffer_view;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    ComPtr<ID3D11Buffer> vertex_buffer;
    ComPtr<ID3D11Buffer> per_frame_cb;
    ComPtr<ID3D11Buffer> per_draw_cb;
    ComPtr<ID3D11Buffer> lights_cb;

    LightsCBData lights_cb_data;

    ComPtr<ID3D11Texture2D>          shadow_tex[LEVEL_SHADOW_MAX];
    ComPtr<ID3D11DepthStencilView>   shadow_dsv[LEVEL_SHADOW_MAX];
    ComPtr<ID3D11ShaderResourceView> shadow_srv[LEVEL_SHADOW_MAX];
    ComPtr<ID3D11SamplerState>       shadow_sampler;
    ComPtr<ID3D11VertexShader>       shadow_vs;
    ComPtr<ID3D11InputLayout>        shadow_il;
    ComPtr<ID3D11Buffer>             shadow_pass_cb;
    ComPtr<ID3D11Buffer>             shadow_vbo;
    float                            shadow_vbo_cpu[GFX_D3D11_SHADOW_VBO_MAX_VERTS * 3];
    uint32_t                         shadow_vbo_cpu_count;
    ComPtr<ID3D11DepthStencilState>  shadow_dss;
    ComPtr<ID3D11RasterizerState>    shadow_raster;

    ComPtr<ID3D11ShaderResourceView> ao_depth_srv;
    ComPtr<ID3D11Texture2D>          ao_out_tex;
    ComPtr<ID3D11RenderTargetView>   ao_out_rtv;
    ComPtr<ID3D11ShaderResourceView> ao_out_srv;
    ComPtr<ID3D11Buffer>             ao_cb;
    ComPtr<ID3D11VertexShader>       ao_fullscreen_vs;
    ComPtr<ID3D11PixelShader>        ao_ssao_ps;
    ComPtr<ID3D11PixelShader>        ao_composite_ps;
    ComPtr<ID3D11SamplerState>       ao_point_sampler;
    ComPtr<ID3D11BlendState>         ao_blend_state;
    ComPtr<ID3D11RasterizerState>    ao_raster;
    ComPtr<ID3D11DepthStencilState>  ao_no_depth_dss;

    ComPtr<ID3D11PixelShader>        fog_ps;
    ComPtr<ID3D11BlendState>         fog_blend_state;
    ComPtr<ID3D11Buffer>             fog_cb;
    FogCBData                        fog_cb_data;

#if DEBUG_D3D
    ComPtr<ID3D11Debug> debug;
#endif

    DXGI_SAMPLE_DESC sample_description;

    PerFrameCB per_frame_cb_data;
    PerDrawCB per_draw_cb_data;

    std::vector<struct ShaderProgramD3D11> shader_program_pool;

    std::vector<struct TextureData> textures;

    uint64_t draw_call_count = 0;
    uint64_t frame_count = 0;
    int current_tile;
    uint32_t current_texture_ids[2];

    // Current state

    struct ShaderProgramD3D11 *shader_program;

    uint32_t current_width, current_height;

    int8_t depth_test;
    int8_t depth_mask;
    int8_t zmode_decal;

    // Previous states (to prevent setting states needlessly)

    struct ShaderProgramD3D11 *last_shader_program = nullptr;
    uint32_t last_vertex_buffer_stride = 0;
    ComPtr<ID3D11BlendState> last_blend_state = nullptr;
    ComPtr<ID3D11ShaderResourceView> last_resource_views[2] = { nullptr, nullptr };
    ComPtr<ID3D11SamplerState> last_sampler_states[2] = { nullptr, nullptr };
    int8_t last_depth_test = -1;
    int8_t last_depth_mask = -1;
    int8_t last_zmode_decal = -1;
    D3D_PRIMITIVE_TOPOLOGY last_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
} d3d;

static LARGE_INTEGER last_time, accumulated_time, frequency;

static void create_render_target_views(bool is_resize) {
    DXGI_SWAP_CHAIN_DESC1 desc1;

    if (is_resize) {
        d3d.backbuffer_view.Reset();
        d3d.depth_stencil_view.Reset();
        d3d.ao_depth_srv.Reset();
        d3d.ao_out_srv.Reset();
        d3d.ao_out_rtv.Reset();
        d3d.ao_out_tex.Reset();

        // Resize swap chain buffers

        ThrowIfFailed(d3d.swap_chain->GetDesc1(&desc1));
        ThrowIfFailed(d3d.swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, desc1.Flags),
                      gfx_dxgi_get_h_wnd(), "Failed to resize IDXGISwapChain buffers.");
    }

    // Get new size

    ThrowIfFailed(d3d.swap_chain->GetDesc1(&desc1));

    // Create back buffer

    ComPtr<ID3D11Texture2D> backbuffer_texture;
    ThrowIfFailed(d3d.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *) backbuffer_texture.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to get backbuffer from IDXGISwapChain.");

    ThrowIfFailed(d3d.device->CreateRenderTargetView(backbuffer_texture.Get(), nullptr, d3d.backbuffer_view.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to create render target view.");

    // Create depth buffer

    D3D11_TEXTURE2D_DESC depth_stencil_texture_desc;
    ZeroMemory(&depth_stencil_texture_desc, sizeof(D3D11_TEXTURE2D_DESC));

    depth_stencil_texture_desc.Width = desc1.Width;
    depth_stencil_texture_desc.Height = desc1.Height;
    depth_stencil_texture_desc.MipLevels = 1;
    depth_stencil_texture_desc.ArraySize = 1;
    const bool ao_capable = d3d.feature_level >= D3D_FEATURE_LEVEL_10_0;
    depth_stencil_texture_desc.Format = ao_capable ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_texture_desc.SampleDesc = d3d.sample_description;
    depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_stencil_texture_desc.BindFlags = ao_capable
        ? D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
        : D3D11_BIND_DEPTH_STENCIL;
    depth_stencil_texture_desc.CPUAccessFlags = 0;
    depth_stencil_texture_desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> depth_stencil_texture;
    ThrowIfFailed(d3d.device->CreateTexture2D(&depth_stencil_texture_desc, nullptr, depth_stencil_texture.GetAddressOf()));
    if (ao_capable) {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
        dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
        dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        ThrowIfFailed(d3d.device->CreateDepthStencilView(depth_stencil_texture.Get(), &dsv_desc, d3d.depth_stencil_view.GetAddressOf()));
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        ThrowIfFailed(d3d.device->CreateShaderResourceView(depth_stencil_texture.Get(), &srv_desc, d3d.ao_depth_srv.GetAddressOf()));
    } else {
        ThrowIfFailed(d3d.device->CreateDepthStencilView(depth_stencil_texture.Get(), nullptr, d3d.depth_stencil_view.GetAddressOf()));
    }

    if (ao_capable) {
        D3D11_TEXTURE2D_DESC ao_tex_desc = {};
        ao_tex_desc.Width = desc1.Width;
        ao_tex_desc.Height = desc1.Height;
        ao_tex_desc.MipLevels = 1;
        ao_tex_desc.ArraySize = 1;
        ao_tex_desc.Format = DXGI_FORMAT_R8_UNORM;
        ao_tex_desc.SampleDesc.Count = 1;
        ao_tex_desc.Usage = D3D11_USAGE_DEFAULT;
        ao_tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        ThrowIfFailed(d3d.device->CreateTexture2D(&ao_tex_desc, nullptr, d3d.ao_out_tex.GetAddressOf()));
        ThrowIfFailed(d3d.device->CreateRenderTargetView(d3d.ao_out_tex.Get(), nullptr, d3d.ao_out_rtv.GetAddressOf()));
        ThrowIfFailed(d3d.device->CreateShaderResourceView(d3d.ao_out_tex.Get(), nullptr, d3d.ao_out_srv.GetAddressOf()));
    }

    d3d.current_width = desc1.Width;
    d3d.current_height = desc1.Height;
}

static void gfx_d3d11_init(void) {
    // Load d3d11.dll
    d3d.d3d11_module = LoadLibraryW(L"d3d11.dll");
    if (d3d.d3d11_module == nullptr) {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), gfx_dxgi_get_h_wnd(), "d3d11.dll could not be loaded");
    }
    d3d.D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3d.d3d11_module, "D3D11CreateDevice");

    // Load D3DCompiler_47.dll or D3DCompiler_43.dll
    d3d.d3dcompiler_module = LoadLibraryW(L"D3DCompiler_47.dll");
    if (d3d.d3dcompiler_module == nullptr) {
        d3d.d3dcompiler_module = LoadLibraryW(L"D3DCompiler_43.dll");
        if (d3d.d3dcompiler_module == nullptr) {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), gfx_dxgi_get_h_wnd(), "D3DCompiler_47.dll or D3DCompiler_43.dll could not be loaded");
        }
    }
    d3d.D3DCompile = (pD3DCompile)GetProcAddress(d3d.d3dcompiler_module, "D3DCompile");

    // Create D3D11 device

    gfx_dxgi_create_factory_and_device(DEBUG_D3D, 11, [](IDXGIAdapter1 *adapter, bool test_only) {
#if DEBUG_D3D
        UINT device_creation_flags = D3D11_CREATE_DEVICE_DEBUG;
#else
        UINT device_creation_flags = 0;
#endif
        D3D_FEATURE_LEVEL FeatureLevels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        HRESULT res = d3d.D3D11CreateDevice(
            adapter,
            D3D_DRIVER_TYPE_UNKNOWN, // since we use a specific adapter
            nullptr,
            device_creation_flags,
            FeatureLevels,
            ARRAYSIZE(FeatureLevels),
            D3D11_SDK_VERSION,
            test_only ? nullptr : d3d.device.GetAddressOf(),
            &d3d.feature_level,
            test_only ? nullptr : d3d.context.GetAddressOf());

        if (test_only) {
            return SUCCEEDED(res);
        } else {
            ThrowIfFailed(res, gfx_dxgi_get_h_wnd(), "Failed to create D3D11 device.");
            return true;
        }
    });

    // Sample description to be used in back buffer and depth buffer

    d3d.sample_description.Count = 1;
    d3d.sample_description.Quality = 0;

    // Create the swap chain
    d3d.swap_chain = gfx_dxgi_create_swap_chain(d3d.device.Get());

    // Create D3D Debug device if in debug mode

#if DEBUG_D3D
    ThrowIfFailed(d3d.device->QueryInterface(__uuidof(ID3D11Debug), (void **) d3d.debug.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to get ID3D11Debug device.");
#endif

    // Create views

    create_render_target_views(false);

    // Create main vertex buffer

    D3D11_BUFFER_DESC vertex_buffer_desc;
    ZeroMemory(&vertex_buffer_desc, sizeof(D3D11_BUFFER_DESC));

    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = (UINT)GFX_D3D11_VBO_BYTES;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertex_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&vertex_buffer_desc, nullptr, d3d.vertex_buffer.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to create vertex buffer.");
    fprintf(stderr, "[d3d11] vertex buffer: %u bytes (%.1f MB, %u max tris)\n",
            (unsigned)GFX_D3D11_VBO_BYTES, GFX_D3D11_VBO_BYTES / 1048576.0,
            (unsigned)GFX_D3D11_MAX_BUFFERED);

    d3d.shader_program_pool.reserve(GFX_D3D11_MAX_SHADERS);
    fprintf(stderr, "[d3d11] shader pool reserved: %u slots\n", (unsigned)GFX_D3D11_MAX_SHADERS);

    // Create per-frame constant buffer

    D3D11_BUFFER_DESC constant_buffer_desc;
    ZeroMemory(&constant_buffer_desc, sizeof(D3D11_BUFFER_DESC));

    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = sizeof(PerFrameCB);
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&constant_buffer_desc, nullptr, d3d.per_frame_cb.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to create per-frame constant buffer.");

    d3d.context->PSSetConstantBuffers(0, 1, d3d.per_frame_cb.GetAddressOf());

    // Create per-draw constant buffer

    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = sizeof(PerDrawCB);
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&constant_buffer_desc, nullptr, d3d.per_draw_cb.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to create per-draw constant buffer.");

    d3d.context->PSSetConstantBuffers(1, 1, d3d.per_draw_cb.GetAddressOf());

    // Create lights constant buffer

    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = sizeof(LightsCBData);
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&constant_buffer_desc, nullptr, d3d.lights_cb.GetAddressOf()),
                  gfx_dxgi_get_h_wnd(), "Failed to create lights constant buffer.");

    d3d.context->PSSetConstantBuffers(2, 1, d3d.lights_cb.GetAddressOf());

    // Shadow map textures, DSVs, SRVs
    {
        D3D11_TEXTURE2D_DESC td = {};
        td.Width            = SHADOW_MAP_SIZE;
        td.Height           = SHADOW_MAP_SIZE;
        td.MipLevels        = 1;
        td.ArraySize        = 1;
        td.Format           = DXGI_FORMAT_R32_TYPELESS;
        td.SampleDesc.Count = 1;
        td.Usage            = D3D11_USAGE_DEFAULT;
        td.BindFlags        = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
        dsvd.Format        = DXGI_FORMAT_D32_FLOAT;
        dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
        srvd.Format                    = DXGI_FORMAT_R32_FLOAT;
        srvd.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvd.Texture2D.MipLevels       = 1;
        srvd.Texture2D.MostDetailedMip = 0;

        for (int i = 0; i < LEVEL_SHADOW_MAX; i++) {
            ThrowIfFailed(d3d.device->CreateTexture2D(&td, nullptr, d3d.shadow_tex[i].GetAddressOf()),
                          gfx_dxgi_get_h_wnd(), "Failed to create shadow map texture.");
            ThrowIfFailed(d3d.device->CreateDepthStencilView(d3d.shadow_tex[i].Get(), &dsvd, d3d.shadow_dsv[i].GetAddressOf()));
            ThrowIfFailed(d3d.device->CreateShaderResourceView(d3d.shadow_tex[i].Get(), &srvd, d3d.shadow_srv[i].GetAddressOf()));
        }

        ID3D11ShaderResourceView *srvs[LEVEL_SHADOW_MAX];
        for (int i = 0; i < LEVEL_SHADOW_MAX; i++) srvs[i] = d3d.shadow_srv[i].Get();
        d3d.context->PSSetShaderResources(2, LEVEL_SHADOW_MAX, srvs);
    }

    // Shadow comparison sampler (slot s2)
    {
        D3D11_SAMPLER_DESC sd = {};
        sd.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        sd.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
        sd.MaxLOD         = D3D11_FLOAT32_MAX;
        ThrowIfFailed(d3d.device->CreateSamplerState(&sd, d3d.shadow_sampler.GetAddressOf()));
        d3d.context->PSSetSamplers(2, 1, d3d.shadow_sampler.GetAddressOf());
    }

    // Shadow pass vertex shader and input layout
    {
        static const char *s_shadow_vs_hlsl =
            "cbuffer ShadowPassCB : register(b0) { row_major float4x4 shadow_vp; }\r\n"
            "float4 main(float3 wp : WORLDPOS) : SV_POSITION { return mul(shadow_vp, float4(wp,1.0)); }\r\n";

        ComPtr<ID3DBlob> vs_blob, err_blob;
        HRESULT hr = d3d.D3DCompile(s_shadow_vs_hlsl, strlen(s_shadow_vs_hlsl),
                                     nullptr, nullptr, nullptr, "main", "vs_4_0",
                                     D3DCOMPILE_OPTIMIZATION_LEVEL2, 0,
                                     vs_blob.GetAddressOf(), err_blob.GetAddressOf());
        if (FAILED(hr)) {
            MessageBox(gfx_dxgi_get_h_wnd(), (char *)err_blob->GetBufferPointer(), "ShadowVS Error", MB_OK | MB_ICONERROR);
            ThrowIfFailed(hr);
        }
        ThrowIfFailed(d3d.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
                                                      nullptr, d3d.shadow_vs.GetAddressOf()));

        D3D11_INPUT_ELEMENT_DESC shadow_ied[] = {
            { "WORLDPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        ThrowIfFailed(d3d.device->CreateInputLayout(shadow_ied, 1,
                                                     vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
                                                     d3d.shadow_il.GetAddressOf()));
    }

    // Shadow pass per-light constant buffer (float4x4 = 64 bytes, rounded to 256)
    {
        D3D11_BUFFER_DESC cbd = {};
        cbd.Usage          = D3D11_USAGE_DYNAMIC;
        cbd.ByteWidth      = 256;
        cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ThrowIfFailed(d3d.device->CreateBuffer(&cbd, nullptr, d3d.shadow_pass_cb.GetAddressOf()),
                      gfx_dxgi_get_h_wnd(), "Failed to create shadow pass CB.");
    }

    // Shadow VBO (float3 per vertex, dynamic)
    {
        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage          = D3D11_USAGE_DYNAMIC;
        vbd.ByteWidth      = GFX_D3D11_SHADOW_VBO_MAX_VERTS * 3 * sizeof(float);
        vbd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ThrowIfFailed(d3d.device->CreateBuffer(&vbd, nullptr, d3d.shadow_vbo.GetAddressOf()),
                      gfx_dxgi_get_h_wnd(), "Failed to create shadow VBO.");
    }

    // Shadow depth-stencil state
    {
        D3D11_DEPTH_STENCIL_DESC dsd = {};
        dsd.DepthEnable    = TRUE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
        ThrowIfFailed(d3d.device->CreateDepthStencilState(&dsd, d3d.shadow_dss.GetAddressOf()));
    }

    // Shadow rasterizer state (slope-scaled bias to avoid acne)
    {
        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode              = D3D11_FILL_SOLID;
        rd.CullMode              = D3D11_CULL_NONE;
        rd.FrontCounterClockwise = TRUE;
        rd.SlopeScaledDepthBias  = 1.0f;
        rd.DepthBias             = 1000;
        rd.DepthClipEnable       = TRUE;
        ThrowIfFailed(d3d.device->CreateRasterizerState(&rd, d3d.shadow_raster.GetAddressOf()));
    }

    if (d3d.feature_level >= D3D_FEATURE_LEVEL_10_0) {
        static const char ao_vs_src[] = R"(
            struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };
            VSOut main(uint id : SV_VertexID) {
                VSOut o;
                o.pos.x = (id & 1) ? 3.0f : -1.0f;
                o.pos.y = (id & 2) ? -3.0f : 1.0f;
                o.pos.zw = float2(0, 1);
                o.uv = o.pos.xy * float2(0.5f, -0.5f) + 0.5f;
                return o;
            })";
        static const char ao_ssao_ps_src[] = R"(
            Texture2D<float> depth_tex : register(t0);
            SamplerState pt_samp : register(s0);
            cbuffer AoCB : register(b3) {
                row_major float4x4 ao_proj;
                row_major float4x4 ao_inv_proj;
                float2 rcp_size;
                float ao_radius;
                float ao_intensity;
                float4 ao_ambient;
            };
            static const float3 K[12] = {
                float3(0.5381f, 0.1856f, 0.4319f), float3(0.1379f, 0.2418f, 0.4441f),
                float3(0.3371f, 0.5679f, 0.0579f), float3(0.0617f, 0.3930f, 0.3599f),
                float3(0.1340f, 0.4348f, 0.1987f), float3(0.1471f, 0.0769f, 0.4482f),
                float3(0.0973f, 0.6080f, 0.2187f), float3(0.2101f, 0.1804f, 0.5377f),
                float3(0.5536f, 0.3528f, 0.1524f), float3(0.6677f, 0.0865f, 0.3554f),
                float3(0.1701f, 0.0553f, 0.7303f), float3(0.2370f, 0.6495f, 0.2358f)
            };
            float3 vpos(float2 uv) {
                float d = depth_tex.SampleLevel(pt_samp, uv, 0);
                float d_ndc = d * 2.0f - 1.0f;
                float4 v = mul(float4(uv.x*2.0f-1.0f, (1.0f-uv.y)*2.0f-1.0f, d_ndc, 1.0f), ao_inv_proj);
                return v.xyz / v.w;
            }
            float main(float4 sv : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
                float d = depth_tex.SampleLevel(pt_samp, uv, 0);
                if (d >= 1.0f) return 1.0f;
                float3 pos = vpos(uv);
                float3 dx = vpos(uv + float2(rcp_size.x, 0)) - pos;
                float3 dy = vpos(uv + float2(0, rcp_size.y)) - pos;
                float3 N = normalize(cross(dx, dy));
                if (dot(N, -pos) < 0) N = -N;
                float ang = frac(sin(dot(sv.xy, float2(12.9898f, 78.233f))) * 43758.5453f) * 6.28318f;
                float sina = sin(ang), cosa = cos(ang);
                float3 rv = float3(cosa, sina, 0);
                float3 T = normalize(rv - N * dot(N, rv));
                float3 B = cross(N, T);
                float occ = 0;
                [unroll] for (int i = 0; i < 12; i++) {
                    float3 sp = pos + (K[i].x*T + K[i].y*B + K[i].z*N) * ao_radius;
                    float4 sc = mul(float4(sp, 1.0f), ao_proj);
                    if (sc.w <= 0) continue;
                    sc.xyz /= sc.w;
                    float2 suv = float2(sc.x*0.5f+0.5f, -sc.y*0.5f+0.5f);
                    if (any(suv < 0) || any(suv > 1)) continue;
                    float sd = depth_tex.SampleLevel(pt_samp, suv, 0);
                    float4 sv4 = mul(float4(suv.x*2.0f-1.0f, (1.0f-suv.y)*2.0f-1.0f, sd*2.0f-1.0f, 1.0f), ao_inv_proj);
                    float3 sa = sv4.xyz / sv4.w;
                    float bias = abs(pos.z) * 0.001f + 0.5f;
                    float diff = abs(sp.z - sa.z);
                    occ += (sa.z > sp.z + bias) ? smoothstep(0, 1, ao_radius / max(diff, 0.001f)) : 0;
                }
                return saturate(1.0f - (occ / 12.0f) * ao_intensity);
            })";
        static const char ao_composite_ps_src[] = R"(
            Texture2D<float> ao_tex : register(t0);
            SamplerState pt_samp : register(s0);
            cbuffer AoCB : register(b3) {
                float4 _aoc_dummy[9];
                float4 ao_ambient;
            };
            float4 main(float4 sv : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
                float ao = ao_tex.SampleLevel(pt_samp, uv, 0);
                float3 result = lerp(ao_ambient.rgb * 0.15f, float3(1,1,1), ao);
                return float4(result, 1);
            })";

        ComPtr<ID3DBlob> vs_blob, ssao_blob, comp_blob, err_blob;
        UINT cf = D3DCOMPILE_OPTIMIZATION_LEVEL2;
        HRESULT hr;
        hr = d3d.D3DCompile(ao_vs_src, sizeof(ao_vs_src)-1, nullptr, nullptr, nullptr, "main", "vs_4_0", cf, 0, vs_blob.GetAddressOf(), err_blob.GetAddressOf());
        if (FAILED(hr)) { MessageBox(gfx_dxgi_get_h_wnd(), (char*)err_blob->GetBufferPointer(), "AO VS", MB_OK|MB_ICONERROR); ThrowIfFailed(hr); }
        hr = d3d.D3DCompile(ao_ssao_ps_src, sizeof(ao_ssao_ps_src)-1, nullptr, nullptr, nullptr, "main", "ps_4_0", cf, 0, ssao_blob.GetAddressOf(), err_blob.GetAddressOf());
        if (FAILED(hr)) { MessageBox(gfx_dxgi_get_h_wnd(), (char*)err_blob->GetBufferPointer(), "AO SSAO PS", MB_OK|MB_ICONERROR); ThrowIfFailed(hr); }
        hr = d3d.D3DCompile(ao_composite_ps_src, sizeof(ao_composite_ps_src)-1, nullptr, nullptr, nullptr, "main", "ps_4_0", cf, 0, comp_blob.GetAddressOf(), err_blob.GetAddressOf());
        if (FAILED(hr)) { MessageBox(gfx_dxgi_get_h_wnd(), (char*)err_blob->GetBufferPointer(), "AO Comp PS", MB_OK|MB_ICONERROR); ThrowIfFailed(hr); }

        ThrowIfFailed(d3d.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, d3d.ao_fullscreen_vs.GetAddressOf()));
        ThrowIfFailed(d3d.device->CreatePixelShader(ssao_blob->GetBufferPointer(), ssao_blob->GetBufferSize(), nullptr, d3d.ao_ssao_ps.GetAddressOf()));
        ThrowIfFailed(d3d.device->CreatePixelShader(comp_blob->GetBufferPointer(), comp_blob->GetBufferSize(), nullptr, d3d.ao_composite_ps.GetAddressOf()));

        // Fog fullscreen PS (reads depth, reconstructs view-Z, blends fog color)
        static const char fog_ps_src[] = R"(
            Texture2D<float> depth_tex : register(t0);
            SamplerState pt_samp : register(s0);
            cbuffer AoCB : register(b3) {
                row_major float4x4 ao_proj;
                row_major float4x4 ao_inv_proj;
            };
            cbuffer FogCB : register(b4) {
                float4 fog_color;
                float fog_near;
                float fog_far;
                float fog_max_density;
                float fog_enabled;
            };
            float4 main(float4 sv : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
                float d = depth_tex.SampleLevel(pt_samp, uv, 0);
                if (d >= 1.0f || fog_enabled < 0.5f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
                float4 ndc = float4(uv.x*2.0f-1.0f, (1.0f-uv.y)*2.0f-1.0f, d*2.0f-1.0f, 1.0f);
                float4 vp4 = mul(ndc, ao_inv_proj);
                float3 wpos = vp4.xyz / vp4.w;
                float view_z = mul(float4(wpos, 1.0f), ao_proj).w;
                float t = saturate((view_z - fog_near) / max(fog_far - fog_near, 1.0f));
                float f = min(t, fog_max_density);
                return float4(fog_color.rgb, f);
            })";
        ComPtr<ID3DBlob> fog_blob;
        hr = d3d.D3DCompile(fog_ps_src, sizeof(fog_ps_src)-1, nullptr, nullptr, nullptr, "main", "ps_4_0", cf, 0, fog_blob.GetAddressOf(), err_blob.GetAddressOf());
        if (FAILED(hr)) { MessageBox(gfx_dxgi_get_h_wnd(), (char*)err_blob->GetBufferPointer(), "Fog PS", MB_OK|MB_ICONERROR); ThrowIfFailed(hr); }
        ThrowIfFailed(d3d.device->CreatePixelShader(fog_blob->GetBufferPointer(), fog_blob->GetBufferSize(), nullptr, d3d.fog_ps.GetAddressOf()));
        {
            D3D11_BLEND_DESC fbd = {};
            fbd.RenderTarget[0].BlendEnable           = TRUE;
            fbd.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
            fbd.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
            fbd.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
            fbd.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ZERO;
            fbd.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
            fbd.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            fbd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            ThrowIfFailed(d3d.device->CreateBlendState(&fbd, d3d.fog_blend_state.GetAddressOf()));
        }
        {
            D3D11_BUFFER_DESC fog_cbd = {};
            fog_cbd.Usage          = D3D11_USAGE_DYNAMIC;
            fog_cbd.ByteWidth      = 256;
            fog_cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            fog_cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            ThrowIfFailed(d3d.device->CreateBuffer(&fog_cbd, nullptr, d3d.fog_cb.GetAddressOf()));
        }

        {
            D3D11_BUFFER_DESC cbd = {};
            cbd.Usage = D3D11_USAGE_DYNAMIC;
            cbd.ByteWidth = 256;
            cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            ThrowIfFailed(d3d.device->CreateBuffer(&cbd, nullptr, d3d.ao_cb.GetAddressOf()));
        }
        {
            D3D11_SAMPLER_DESC sd = {};
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            sd.MaxLOD = D3D11_FLOAT32_MAX;
            ThrowIfFailed(d3d.device->CreateSamplerState(&sd, d3d.ao_point_sampler.GetAddressOf()));
        }
        {
            D3D11_BLEND_DESC bd = {};
            bd.RenderTarget[0].BlendEnable = TRUE;
            bd.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
            bd.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
            bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
            bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
            bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            ThrowIfFailed(d3d.device->CreateBlendState(&bd, d3d.ao_blend_state.GetAddressOf()));
        }
        {
            D3D11_RASTERIZER_DESC rd2 = {};
            rd2.FillMode = D3D11_FILL_SOLID;
            rd2.CullMode = D3D11_CULL_NONE;
            rd2.ScissorEnable = FALSE;
            ThrowIfFailed(d3d.device->CreateRasterizerState(&rd2, d3d.ao_raster.GetAddressOf()));
        }
        {
            D3D11_DEPTH_STENCIL_DESC dsd = {};
            dsd.DepthEnable = FALSE;
            dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            ThrowIfFailed(d3d.device->CreateDepthStencilState(&dsd, d3d.ao_no_depth_dss.GetAddressOf()));
        }
    }

    imgui_menu_init();
    ImGui_ImplWin32_Init(gfx_dxgi_get_h_wnd());
    ImGui_ImplDX11_Init(d3d.device.Get(), d3d.context.Get());
}


static bool gfx_d3d11_z_is_from_0_to_1(void) {
    return true;
}

static void gfx_d3d11_unload_shader(struct ShaderProgram *old_prg) {
}

static void gfx_d3d11_load_shader(struct ShaderProgram *new_prg) {
    d3d.shader_program = (struct ShaderProgramD3D11 *)new_prg;
}

static struct ShaderProgram *gfx_d3d11_create_and_load_new_shader(uint32_t shader_id) {
    CCFeatures cc_features;
    gfx_cc_get_features(shader_id, &cc_features);

    char buf[8192];
    size_t len, num_floats;

    gfx_direct3d_common_build_shader(buf, len, num_floats, cc_features, false, THREE_POINT_FILTERING);

    ComPtr<ID3DBlob> vs, ps;
    ComPtr<ID3DBlob> error_blob;

#if DEBUG_D3D
    UINT compile_flags = D3DCOMPILE_DEBUG;
#else
    UINT compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL2;
#endif

    HRESULT hr = d3d.D3DCompile(buf, len, nullptr, nullptr, nullptr, "VSMain", GFX_D3D11_SHADER_VS, compile_flags, 0, vs.GetAddressOf(), error_blob.GetAddressOf());

    if (FAILED(hr)) {
        MessageBox(gfx_dxgi_get_h_wnd(), (char *) error_blob->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
        throw hr;
    }

    hr = d3d.D3DCompile(buf, len, nullptr, nullptr, nullptr, "PSMain", GFX_D3D11_SHADER_PS, compile_flags, 0, ps.GetAddressOf(), error_blob.GetAddressOf());

    if (FAILED(hr)) {
        MessageBox(gfx_dxgi_get_h_wnd(), (char *) error_blob->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
        throw hr;
    }

    d3d.shader_program_pool.emplace_back();
    struct ShaderProgramD3D11 *prg = &d3d.shader_program_pool.back();
    fprintf(stderr, "[d3d11] create_shader id=0x%08x slot=%zu/%u\n",
            shader_id, d3d.shader_program_pool.size() - 1, (unsigned)GFX_D3D11_MAX_SHADERS);

    ThrowIfFailed(d3d.device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, prg->vertex_shader.GetAddressOf()));
    ThrowIfFailed(d3d.device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, prg->pixel_shader.GetAddressOf()));

    // Input Layout

    D3D11_INPUT_ELEMENT_DESC ied[8];
    uint8_t ied_index = 0;
    ied[ied_index++] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        ied[ied_index++] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }
    if (cc_features.opt_fog) {
        ied[ied_index++] = { "FOG", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }
    for (unsigned int i = 0; i < cc_features.num_inputs; i++) {
        DXGI_FORMAT format = cc_features.opt_alpha ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32G32B32_FLOAT;
        ied[ied_index++] = { "INPUT", i, format, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }
    ied[ied_index++] = { "WORLDPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

    ThrowIfFailed(d3d.device->CreateInputLayout(ied, ied_index, vs->GetBufferPointer(), vs->GetBufferSize(), prg->input_layout.GetAddressOf()));

    // Blend state

    D3D11_BLEND_DESC blend_desc;
    ZeroMemory(&blend_desc, sizeof(D3D11_BLEND_DESC));

    if (cc_features.opt_alpha) {
        blend_desc.RenderTarget[0].BlendEnable = true;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    } else {
        blend_desc.RenderTarget[0].BlendEnable = false;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    ThrowIfFailed(d3d.device->CreateBlendState(&blend_desc, prg->blend_state.GetAddressOf()));

    // Save some values

    prg->shader_id = shader_id;
    prg->num_inputs = cc_features.num_inputs;
    prg->num_floats = num_floats;
    prg->used_textures[0] = cc_features.used_textures[0];
    prg->used_textures[1] = cc_features.used_textures[1];

    return (struct ShaderProgram *)(d3d.shader_program = prg);
}

static struct ShaderProgram *gfx_d3d11_lookup_shader(uint32_t shader_id) {
    for (size_t i = 0; i < d3d.shader_program_pool.size(); i++) {
        if (d3d.shader_program_pool[i].shader_id == shader_id) {
            return (struct ShaderProgram *)&d3d.shader_program_pool[i];
        }
    }
    return nullptr;
}

static void gfx_d3d11_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    struct ShaderProgramD3D11 *p = (struct ShaderProgramD3D11 *)prg;

    *num_inputs = p->num_inputs;
    used_textures[0] = p->used_textures[0];
    used_textures[1] = p->used_textures[1];
}

static uint32_t gfx_d3d11_new_texture(void) {
    uint32_t id = (uint32_t)d3d.textures.size();
    d3d.textures.emplace_back();
    fprintf(stderr, "[d3d11] new_texture id=%u (total=%u)\n", id, (unsigned)d3d.textures.size());
    return id;
}

static void gfx_d3d11_select_texture(int tile, uint32_t texture_id) {
    d3d.current_tile = tile;
    d3d.current_texture_ids[tile] = texture_id;
}

static D3D11_TEXTURE_ADDRESS_MODE gfx_cm_to_d3d11(uint32_t val) {
    if (val & G_TX_CLAMP) {
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    }
    return (val & G_TX_MIRROR) ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
}

static void gfx_d3d11_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    // Create texture

    D3D11_TEXTURE2D_DESC texture_desc;
    ZeroMemory(&texture_desc, sizeof(D3D11_TEXTURE2D_DESC));

    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0; // D3D11_RESOURCE_MISC_GENERATE_MIPS ?
    texture_desc.ArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA resource_data;
    resource_data.pSysMem = rgba32_buf;
    resource_data.SysMemPitch = width * 4;
    resource_data.SysMemSlicePitch = resource_data.SysMemPitch * height;

    ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(d3d.device->CreateTexture2D(&texture_desc, &resource_data, texture.GetAddressOf()));

    // Create shader resource view from texture

    D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc;
    ZeroMemory(&resource_view_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    resource_view_desc.Format = texture_desc.Format;
    resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resource_view_desc.Texture2D.MostDetailedMip = 0;
    resource_view_desc.Texture2D.MipLevels = -1;

    TextureData *texture_data = &d3d.textures[d3d.current_texture_ids[d3d.current_tile]];
    texture_data->width = width;
    texture_data->height = height;

    if (texture_data->resource_view.Get() != nullptr) {
        texture_data->resource_view.Reset();
    }

    ThrowIfFailed(d3d.device->CreateShaderResourceView(texture.Get(), &resource_view_desc, texture_data->resource_view.GetAddressOf()));
    fprintf(stderr, "[d3d11] upload_texture slot=%u %dx%d\n",
            d3d.current_texture_ids[d3d.current_tile], width, height);
}

static void gfx_d3d11_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    D3D11_SAMPLER_DESC sampler_desc;
    ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));

#if THREE_POINT_FILTERING
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
#else
    sampler_desc.Filter = linear_filter ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
#endif
    sampler_desc.AddressU = gfx_cm_to_d3d11(cms);
    sampler_desc.AddressV = gfx_cm_to_d3d11(cmt);
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    TextureData *texture_data = &d3d.textures[d3d.current_texture_ids[tile]];
    texture_data->linear_filtering = linear_filter;

    // This function is called twice per texture, the first one only to set default values.
    // Maybe that could be skipped? Anyway, make sure to release the first default sampler
    // state before setting the actual one.
    texture_data->sampler_state.Reset();

    ThrowIfFailed(d3d.device->CreateSamplerState(&sampler_desc, texture_data->sampler_state.GetAddressOf()));
}

static void gfx_d3d11_set_depth_test(bool depth_test) {
    d3d.depth_test = depth_test;
}

static void gfx_d3d11_set_depth_mask(bool depth_mask) {
    d3d.depth_mask = depth_mask;
}

static void gfx_d3d11_set_zmode_decal(bool zmode_decal) {
    d3d.zmode_decal = zmode_decal;
}

static void gfx_d3d11_set_viewport(int x, int y, int width, int height) {
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = x;
    viewport.TopLeftY = d3d.current_height - y - height;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    d3d.context->RSSetViewports(1, &viewport);
}

static void gfx_d3d11_set_scissor(int x, int y, int width, int height) {
    D3D11_RECT rect;
    rect.left = x;
    rect.top = d3d.current_height - y - height;
    rect.right = x + width;
    rect.bottom = d3d.current_height - y;

    d3d.context->RSSetScissorRects(1, &rect);
}

static void gfx_d3d11_set_use_alpha(bool use_alpha) {
    // Already part of the pipeline state from shader info
}

static void gfx_d3d11_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    d3d.draw_call_count++;
    if ((d3d.draw_call_count & 0x1FFF) == 0) {
        fprintf(stderr, "[d3d11] draw_triangles: call=%llu frame=%llu tris_this_batch=%zu shaders=%zu textures=%zu\n",
                (unsigned long long)d3d.draw_call_count, (unsigned long long)d3d.frame_count,
                buf_vbo_num_tris, d3d.shader_program_pool.size(), d3d.textures.size());
    }

    if (d3d.last_depth_test != d3d.depth_test || d3d.last_depth_mask != d3d.depth_mask) {
        d3d.last_depth_test = d3d.depth_test;
        d3d.last_depth_mask = d3d.depth_mask;

        d3d.depth_stencil_state.Reset();

        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
        ZeroMemory(&depth_stencil_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

        depth_stencil_desc.DepthEnable = d3d.depth_test;
        depth_stencil_desc.DepthWriteMask = d3d.depth_mask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depth_stencil_desc.StencilEnable = false;

        ThrowIfFailed(d3d.device->CreateDepthStencilState(&depth_stencil_desc, d3d.depth_stencil_state.GetAddressOf()));
        d3d.context->OMSetDepthStencilState(d3d.depth_stencil_state.Get(), 0);
    }

    if (d3d.last_zmode_decal != d3d.zmode_decal) {
        d3d.last_zmode_decal = d3d.zmode_decal;

        d3d.rasterizer_state.Reset();

        D3D11_RASTERIZER_DESC rasterizer_desc;
        ZeroMemory(&rasterizer_desc, sizeof(D3D11_RASTERIZER_DESC));

        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = true;
        rasterizer_desc.DepthBias = 0;
        rasterizer_desc.SlopeScaledDepthBias = d3d.zmode_decal ? -2.0f : 0.0f;
        rasterizer_desc.DepthBiasClamp = 0.0f;
        rasterizer_desc.DepthClipEnable = true;
        rasterizer_desc.ScissorEnable = true;
        rasterizer_desc.MultisampleEnable = false;
        rasterizer_desc.AntialiasedLineEnable = false;

        ThrowIfFailed(d3d.device->CreateRasterizerState(&rasterizer_desc, d3d.rasterizer_state.GetAddressOf()));
        d3d.context->RSSetState(d3d.rasterizer_state.Get());
    }

    bool textures_changed = false;

    for (int i = 0; i < 2; i++) {
        if (d3d.shader_program->used_textures[i]) {
            if (d3d.last_resource_views[i].Get() != d3d.textures[d3d.current_texture_ids[i]].resource_view.Get()) {
                d3d.last_resource_views[i] = d3d.textures[d3d.current_texture_ids[i]].resource_view.Get();
                d3d.context->PSSetShaderResources(i, 1, d3d.textures[d3d.current_texture_ids[i]].resource_view.GetAddressOf());

#if THREE_POINT_FILTERING
                d3d.per_draw_cb_data.textures[i].width = d3d.textures[d3d.current_texture_ids[i]].width;
                d3d.per_draw_cb_data.textures[i].height = d3d.textures[d3d.current_texture_ids[i]].height;
                d3d.per_draw_cb_data.textures[i].linear_filtering = d3d.textures[d3d.current_texture_ids[i]].linear_filtering;
                textures_changed = true;
#endif

                if (d3d.last_sampler_states[i].Get() != d3d.textures[d3d.current_texture_ids[i]].sampler_state.Get()) {
                    d3d.last_sampler_states[i] = d3d.textures[d3d.current_texture_ids[i]].sampler_state.Get();
                    d3d.context->PSSetSamplers(i, 1, d3d.textures[d3d.current_texture_ids[i]].sampler_state.GetAddressOf());
                }
            }
        }
    }

    // Set per-draw constant buffer

    if (textures_changed) {
        D3D11_MAPPED_SUBRESOURCE ms;
        ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
        d3d.context->Map(d3d.per_draw_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, &d3d.per_draw_cb_data, sizeof(PerDrawCB));
        d3d.context->Unmap(d3d.per_draw_cb.Get(), 0);
    }

    // Set vertex buffer data

    D3D11_MAPPED_SUBRESOURCE ms;
    ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3d.context->Map(d3d.vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, buf_vbo, buf_vbo_len * sizeof(float));
    d3d.context->Unmap(d3d.vertex_buffer.Get(), 0);

    uint32_t stride = d3d.shader_program->num_floats * sizeof(float);
    uint32_t offset = 0;

    if (d3d.last_vertex_buffer_stride != stride) {
        d3d.last_vertex_buffer_stride = stride;
        d3d.context->IASetVertexBuffers(0, 1, d3d.vertex_buffer.GetAddressOf(), &stride, &offset);
    }

    if (d3d.last_shader_program != d3d.shader_program) {
        d3d.last_shader_program = d3d.shader_program;
        d3d.context->IASetInputLayout(d3d.shader_program->input_layout.Get());
        d3d.context->VSSetShader(d3d.shader_program->vertex_shader.Get(), 0, 0);
        d3d.context->PSSetShader(d3d.shader_program->pixel_shader.Get(), 0, 0);

        if (d3d.last_blend_state.Get() != d3d.shader_program->blend_state.Get()) {
            d3d.last_blend_state = d3d.shader_program->blend_state.Get();
            d3d.context->OMSetBlendState(d3d.shader_program->blend_state.Get(), 0, 0xFFFFFFFF);
        }
    }

    if (d3d.last_primitive_topology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
        d3d.last_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        d3d.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    d3d.context->Draw(buf_vbo_num_tris * 3, 0);

    // Accumulate worldPos for shadow pass next frame (last 3 floats per vertex).
    {
        const uint32_t verts   = (uint32_t)(buf_vbo_num_tris * 3);
        const uint32_t vfloats = d3d.shader_program->num_floats;
        if (d3d.shadow_vbo_cpu_count + verts <= GFX_D3D11_SHADOW_VBO_MAX_VERTS) {
            for (uint32_t vi = 0; vi < verts; vi++) {
                const size_t b = (size_t)vi * vfloats;
                d3d.shadow_vbo_cpu[d3d.shadow_vbo_cpu_count * 3 + 0] = buf_vbo[b + vfloats - 3];
                d3d.shadow_vbo_cpu[d3d.shadow_vbo_cpu_count * 3 + 1] = buf_vbo[b + vfloats - 2];
                d3d.shadow_vbo_cpu[d3d.shadow_vbo_cpu_count * 3 + 2] = buf_vbo[b + vfloats - 1];
                d3d.shadow_vbo_cpu_count++;
            }
        }
    }
}

static void gfx_d3d11_on_resize(void) {
    create_render_target_views(true);
}

static void gfx_d3d11_start_frame(void) {
    d3d.frame_count++;
    if ((d3d.frame_count & 0x3F) == 0) {
        fprintf(stderr, "[d3d11] frame=%llu shaders=%zu textures=%zu draw_calls_total=%llu\n",
                (unsigned long long)d3d.frame_count,
                d3d.shader_program_pool.size(), d3d.textures.size(),
                (unsigned long long)d3d.draw_call_count);
    }

    // Fill lights CB data — reset to defaults, apply level-specific lighting, then fill.
    level_lighting_apply_current();
    level_lights_fill_cb(&d3d.lights_cb_data);

    // Shadow pass: render previous frame's accumulated geometry into shadow map depth textures.
    if (d3d.shadow_vbo_cpu_count > 0 && d3d.lights_cb_data.shadow_count > 0) {
        // Upload shadow VBO
        D3D11_MAPPED_SUBRESOURCE svms = {};
        d3d.context->Map(d3d.shadow_vbo.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &svms);
        memcpy(svms.pData, d3d.shadow_vbo_cpu, d3d.shadow_vbo_cpu_count * 3 * sizeof(float));
        d3d.context->Unmap(d3d.shadow_vbo.Get(), 0);

        d3d.context->VSSetShader(d3d.shadow_vs.Get(), nullptr, 0);
        d3d.context->PSSetShader(nullptr, nullptr, 0);
        d3d.context->IASetInputLayout(d3d.shadow_il.Get());
        d3d.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d.context->OMSetDepthStencilState(d3d.shadow_dss.Get(), 0);
        d3d.context->RSSetState(d3d.shadow_raster.Get());
        d3d.context->VSSetConstantBuffers(0, 1, d3d.shadow_pass_cb.GetAddressOf());

        const UINT sv_stride = 3 * sizeof(float), sv_offset = 0;
        d3d.context->IASetVertexBuffers(0, 1, d3d.shadow_vbo.GetAddressOf(), &sv_stride, &sv_offset);

        const D3D11_VIEWPORT svp = { 0, 0, (float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE, 0.0f, 1.0f };
        d3d.context->RSSetViewports(1, &svp);

        const int scount = d3d.lights_cb_data.shadow_count < LEVEL_SHADOW_MAX
                         ? d3d.lights_cb_data.shadow_count : LEVEL_SHADOW_MAX;
        for (int si = 0; si < scount; si++) {
            // Unbind this SRV before using its texture as DSV
            ID3D11ShaderResourceView *null_srv = nullptr;
            d3d.context->PSSetShaderResources(2 + si, 1, &null_srv);

            d3d.context->ClearDepthStencilView(d3d.shadow_dsv[si].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
            ID3D11RenderTargetView *null_rtv = nullptr;
            d3d.context->OMSetRenderTargets(1, &null_rtv, d3d.shadow_dsv[si].Get());

            D3D11_MAPPED_SUBRESOURCE cbms = {};
            d3d.context->Map(d3d.shadow_pass_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cbms);
            memcpy(cbms.pData, d3d.lights_cb_data.shadow_vp[si], 16 * sizeof(float));
            d3d.context->Unmap(d3d.shadow_pass_cb.Get(), 0);

            d3d.context->Draw(d3d.shadow_vbo_cpu_count, 0);

            // Rebind SRV after shadow rendering
            d3d.context->PSSetShaderResources(2 + si, 1, d3d.shadow_srv[si].GetAddressOf());
        }

        // Clear DSV slots that were not rendered (shadow_count < LEVEL_SHADOW_MAX)
        for (int si = scount; si < LEVEL_SHADOW_MAX; si++) {
            d3d.context->ClearDepthStencilView(d3d.shadow_dsv[si].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        }

        // Restore pipeline state dirtied by shadow pass
        d3d.last_shader_program      = nullptr;
        d3d.last_vertex_buffer_stride = 0;
        d3d.last_depth_test           = -1;
        d3d.last_depth_mask           = -1;
        d3d.last_zmode_decal          = -1;

        // Restore viewport to main framebuffer size
        D3D11_VIEWPORT main_vp = { 0, 0, (float)d3d.current_width, (float)d3d.current_height, 0.0f, 1.0f };
        d3d.context->RSSetViewports(1, &main_vp);
    }
    d3d.shadow_vbo_cpu_count = 0;

    d3d.context->OMSetRenderTargets(1, d3d.backbuffer_view.GetAddressOf(), d3d.depth_stencil_view.Get());

    // Clear render targets

    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    d3d.context->ClearRenderTargetView(d3d.backbuffer_view.Get(), clearColor);
    d3d.context->ClearDepthStencilView(d3d.depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Set per-frame constant buffer

    d3d.per_frame_cb_data.noise_frame++;
    if (d3d.per_frame_cb_data.noise_frame > 150) {
        // No high values, as noise starts to look ugly
        d3d.per_frame_cb_data.noise_frame = 0;
    }
    float aspect_ratio = (float) d3d.current_width / (float) d3d.current_height;
    d3d.per_frame_cb_data.noise_scale_x = 120 * aspect_ratio; // 120 = N64 height resolution (240) / 2
    d3d.per_frame_cb_data.noise_scale_y = 120;

    D3D11_MAPPED_SUBRESOURCE ms;
    ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3d.context->Map(d3d.per_frame_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, &d3d.per_frame_cb_data, sizeof(PerFrameCB));
    d3d.context->Unmap(d3d.per_frame_cb.Get(), 0);

    // Upload lights constant buffer
    ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3d.context->Map(d3d.lights_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, &d3d.lights_cb_data, sizeof(LightsCBData));
    d3d.context->Unmap(d3d.lights_cb.Get(), 0);

    if (d3d.fog_cb) {
        level_lights_fill_fog_cb(&d3d.fog_cb_data);
        D3D11_MAPPED_SUBRESOURCE fog_ms = {};
        d3d.context->Map(d3d.fog_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &fog_ms);
        memcpy(fog_ms.pData, &d3d.fog_cb_data, sizeof(FogCBData));
        d3d.context->Unmap(d3d.fog_cb.Get(), 0);
        d3d.context->PSSetConstantBuffers(4, 1, d3d.fog_cb.GetAddressOf());
    }

    if (d3d.ao_cb) {
        float p[4][4];
        gfx_pc_get_ao_proj(p);
        const GfxSSAOParams *sp = gfx_ssao_params();
        AoCBData ao_data = {};
        memcpy(ao_data.proj, p, 64);
        gfx_d3d_mat4_inverse(&p[0][0], ao_data.inv_proj);
        ao_data.rcp_size[0] = 1.0f / d3d.current_width;
        ao_data.rcp_size[1] = 1.0f / d3d.current_height;
        ao_data.ao_radius    = sp->radius;
        ao_data.ao_intensity = sp->enabled ? sp->intensity : 0.0f;
        ao_data.ao_ambient[0] = d3d.lights_cb_data.ambient_color[0];
        ao_data.ao_ambient[1] = d3d.lights_cb_data.ambient_color[1];
        ao_data.ao_ambient[2] = d3d.lights_cb_data.ambient_color[2];
        ao_data.ao_ambient[3] = 1.0f;
        D3D11_MAPPED_SUBRESOURCE aoms = {};
        d3d.context->Map(d3d.ao_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &aoms);
        memcpy(aoms.pData, &ao_data, sizeof(AoCBData));
        d3d.context->Unmap(d3d.ao_cb.Get(), 0);
    }
}

static void gfx_d3d11_end_frame(void) {
    const GfxSSAOParams *sp = gfx_ssao_params();
    if (d3d.ao_depth_srv && d3d.ao_out_rtv && (sp->enabled || sp->visualize)) {
        D3D11_VIEWPORT vp = { 0, 0, (float)d3d.current_width, (float)d3d.current_height, 0.0f, 1.0f };
        d3d.context->RSSetViewports(1, &vp);
        d3d.context->IASetInputLayout(nullptr);
        d3d.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d.context->OMSetDepthStencilState(d3d.ao_no_depth_dss.Get(), 0);
        d3d.context->RSSetState(d3d.ao_raster.Get());
        d3d.context->VSSetShader(d3d.ao_fullscreen_vs.Get(), nullptr, 0);

        d3d.context->OMSetRenderTargets(1, d3d.ao_out_rtv.GetAddressOf(), nullptr);
        const float white[] = { 1, 1, 1, 1 };
        d3d.context->ClearRenderTargetView(d3d.ao_out_rtv.Get(), white);
        d3d.context->PSSetShader(d3d.ao_ssao_ps.Get(), nullptr, 0);
        d3d.context->PSSetConstantBuffers(3, 1, d3d.ao_cb.GetAddressOf());
        d3d.context->PSSetShaderResources(0, 1, d3d.ao_depth_srv.GetAddressOf());
        d3d.context->PSSetSamplers(0, 1, d3d.ao_point_sampler.GetAddressOf());
        d3d.context->Draw(3, 0);

        d3d.context->OMSetRenderTargets(1, d3d.backbuffer_view.GetAddressOf(), nullptr);
        d3d.context->PSSetShader(d3d.ao_composite_ps.Get(), nullptr, 0);
        d3d.context->PSSetConstantBuffers(3, 1, d3d.ao_cb.GetAddressOf());
        d3d.context->PSSetShaderResources(0, 1, d3d.ao_out_srv.GetAddressOf());
        if (sp->visualize) {
            d3d.context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        } else {
            d3d.context->OMSetBlendState(d3d.ao_blend_state.Get(), nullptr, 0xFFFFFFFF);
        }
        d3d.context->Draw(3, 0);

        d3d.context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        ID3D11ShaderResourceView *null_srvs[2] = { nullptr, nullptr };
        d3d.context->PSSetShaderResources(0, 2, null_srvs);
        d3d.last_shader_program = nullptr;
        d3d.last_vertex_buffer_stride = 0;
        d3d.last_depth_test = -1;
        d3d.last_depth_mask = -1;
        d3d.last_zmode_decal = -1;
        d3d.last_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    if (d3d.fog_ps && d3d.ao_depth_srv && d3d.fog_cb && d3d.fog_cb_data.fog_enabled > 0.5f) {
        D3D11_VIEWPORT vp2 = { 0, 0, (float)d3d.current_width, (float)d3d.current_height, 0.0f, 1.0f };
        d3d.context->RSSetViewports(1, &vp2);
        d3d.context->IASetInputLayout(nullptr);
        d3d.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d.context->OMSetDepthStencilState(d3d.ao_no_depth_dss.Get(), 0);
        d3d.context->RSSetState(d3d.ao_raster.Get());
        d3d.context->VSSetShader(d3d.ao_fullscreen_vs.Get(), nullptr, 0);
        d3d.context->PSSetShader(d3d.fog_ps.Get(), nullptr, 0);
        d3d.context->OMSetRenderTargets(1, d3d.backbuffer_view.GetAddressOf(), nullptr);
        d3d.context->OMSetBlendState(d3d.fog_blend_state.Get(), nullptr, 0xFFFFFFFF);
        d3d.context->PSSetShaderResources(0, 1, d3d.ao_depth_srv.GetAddressOf());
        d3d.context->PSSetSamplers(0, 1, d3d.ao_point_sampler.GetAddressOf());
        d3d.context->PSSetConstantBuffers(3, 1, d3d.ao_cb.GetAddressOf());
        d3d.context->PSSetConstantBuffers(4, 1, d3d.fog_cb.GetAddressOf());
        d3d.context->Draw(3, 0);
        d3d.context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        ID3D11ShaderResourceView *null_srv0 = nullptr;
        d3d.context->PSSetShaderResources(0, 1, &null_srv0);
    }

    actor_manager_render();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    imgui_menu_new_frame();
    imgui_menu_compose_frame();
    imgui_menu_render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

static void gfx_d3d11_finish_render(void) {
}

static uintptr_t gfx_d3d11_get_imgui_tex_id(uint32_t texture_id) {
    if (texture_id >= d3d.textures.size()) return 0;
    return (uintptr_t)d3d.textures[texture_id].resource_view.Get();
}

static void gfx_d3d11_clear_depth_stencil(void) {
    if (d3d.depth_stencil_view) {
        d3d.context->ClearDepthStencilView(
            d3d.depth_stencil_view.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0);
    }
}

} // namespace

ID3D11Device*           gfx_d3d11_get_device(void)  { return d3d.device.Get(); }
ID3D11DeviceContext*    gfx_d3d11_get_context(void) { return d3d.context.Get(); }
ID3D11RenderTargetView* gfx_d3d11_get_rtv(void)     { return d3d.backbuffer_view.Get(); }
ID3D11DepthStencilView* gfx_d3d11_get_dsv(void)     { return d3d.depth_stencil_view.Get(); }
void gfx_d3d11_get_size(float *w, float *h) { *w = (float)d3d.current_width; *h = (float)d3d.current_height; }

struct GfxRenderingAPI gfx_direct3d11_api = {
    gfx_d3d11_z_is_from_0_to_1,
    gfx_d3d11_unload_shader,
    gfx_d3d11_load_shader,
    gfx_d3d11_create_and_load_new_shader,
    gfx_d3d11_lookup_shader,
    gfx_d3d11_shader_get_info,
    gfx_d3d11_new_texture,
    gfx_d3d11_select_texture,
    gfx_d3d11_upload_texture,
    gfx_d3d11_set_sampler_parameters,
    gfx_d3d11_set_depth_test,
    gfx_d3d11_set_depth_mask,
    gfx_d3d11_set_zmode_decal,
    gfx_d3d11_set_viewport,
    gfx_d3d11_set_scissor,
    gfx_d3d11_set_use_alpha,
    gfx_d3d11_draw_triangles,
    gfx_d3d11_init,
    gfx_d3d11_on_resize,
    gfx_d3d11_start_frame,
    gfx_d3d11_end_frame,
    gfx_d3d11_finish_render,
    gfx_d3d11_get_imgui_tex_id,
    gfx_d3d11_clear_depth_stencil,
};

#endif
