#include "actor_manager.h"
#include "net.h"

#ifdef ENABLE_DX11

#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>
#include <cmath>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#undef near
#undef far
extern "C" {
#include "types.h"
#include "sm64.h"
#include "game/camera.h"
}

extern "C" void gfx_pc_get_proj_matrix(float out[4][4]);

extern ID3D11Device*           gfx_d3d11_get_device(void);
extern ID3D11DeviceContext*    gfx_d3d11_get_context(void);
extern ID3D11RenderTargetView* gfx_d3d11_get_rtv(void);
extern ID3D11DepthStencilView* gfx_d3d11_get_dsv(void);
extern void                    gfx_d3d11_get_size(float *w, float *h);

using Microsoft::WRL::ComPtr;

struct CubeVert { float x, y, z; };

static const CubeVert k_cube_verts[8] = {
    {-40.f,-40.f,-40.f}, { 40.f,-40.f,-40.f},
    { 40.f,-40.f, 40.f}, {-40.f,-40.f, 40.f},
    {-40.f, 40.f,-40.f}, { 40.f, 40.f,-40.f},
    { 40.f, 40.f, 40.f}, {-40.f, 40.f, 40.f},
};

static const uint16_t k_cube_idx[36] = {
    0,2,1, 0,3,2,  // bottom
    4,5,6, 4,6,7,  // top
    0,1,5, 0,5,4,  // front
    2,3,7, 2,7,6,  // back
    3,0,4, 3,4,7,  // left
    1,2,6, 1,6,5,  // right
};

static const float k_pid_colors[NET_MAX_PLAYERS][4] = {
    {1.f,  0.25f, 0.25f, 0.85f},
    {0.25f,1.f,   0.25f, 0.85f},
    {0.25f,0.5f,  1.f,   0.85f},
    {1.f,  1.f,   0.25f, 0.85f},
    {1.f,  0.25f, 1.f,   0.85f},
    {0.25f,1.f,   1.f,   0.85f},
    {1.f,  0.6f,  0.1f,  0.85f},
    {0.7f, 0.25f, 1.f,   0.85f},
};

struct ActorCB {
    float wvp[4][4];
    float color[4];
};

static const char k_shader[] = R"hlsl(
#pragma pack_matrix(row_major)
cbuffer ActorCB : register(b0) {
    float4x4 wvp;
    float4   color;
};
struct VSIn  { float3 pos : POSITION; };
struct VSOut { float4 pos : SV_POSITION; float4 col : COLOR; };
VSOut VS(VSIn i) {
    VSOut o;
    o.pos = mul(float4(i.pos, 1.0), wvp);
    o.col = color;
    return o;
}
float4 PS(VSOut i) : SV_TARGET { return i.col; }
)hlsl";

static ComPtr<ID3D11Buffer>            g_vbuf;
static ComPtr<ID3D11Buffer>            g_ibuf;
static ComPtr<ID3D11Buffer>            g_cbuf;
static ComPtr<ID3D11VertexShader>      g_vs;
static ComPtr<ID3D11PixelShader>       g_ps;
static ComPtr<ID3D11InputLayout>       g_il;
static ComPtr<ID3D11RasterizerState>   g_raster;
static ComPtr<ID3D11DepthStencilState> g_dss;
static ComPtr<ID3D11BlendState>        g_blend;

static bool      g_ready = false;
static NetPlayer g_actors[NET_MAX_PLAYERS];

static void mat4_identity(float m[4][4]) {
    memset(m, 0, 64);
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f;
}

static void mat4_mul(float out[4][4], const float A[4][4], const float B[4][4]) {
    float tmp[4][4] = {};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                tmp[i][j] += A[i][k] * B[k][j];
    memcpy(out, tmp, 64);
}

static void mat4_look_at(float out[4][4], const float eye[3], const float at[3]) {
    float fx = at[0]-eye[0], fy = at[1]-eye[1], fz = at[2]-eye[2];
    float len = sqrtf(fx*fx + fy*fy + fz*fz);
    if (len < 0.001f) { mat4_identity(out); return; }
    fx /= len; fy /= len; fz /= len;

    float rx = fy*0.f - fz*1.f, ry = fz*0.f - fx*0.f, rz = fx*1.f - fy*0.f;
    len = sqrtf(rx*rx + ry*ry + rz*rz);
    if (len < 0.001f) { mat4_identity(out); return; }
    rx /= len; ry /= len; rz /= len;

    float ux = ry*fz - rz*fy, uy = rz*fx - rx*fz, uz = rx*fy - ry*fx;

    float dr = rx*eye[0] + ry*eye[1] + rz*eye[2];
    float du = ux*eye[0] + uy*eye[1] + uz*eye[2];
    float df = fx*eye[0] + fy*eye[1] + fz*eye[2];

    out[0][0]=rx;  out[0][1]=ux;  out[0][2]=fx;  out[0][3]=0.f;
    out[1][0]=ry;  out[1][1]=uy;  out[1][2]=fy;  out[1][3]=0.f;
    out[2][0]=rz;  out[2][1]=uz;  out[2][2]=fz;  out[2][3]=0.f;
    out[3][0]=-dr; out[3][1]=-du; out[3][2]=-df; out[3][3]=1.f;
}

void actor_manager_init(void) {
    ID3D11Device *dev = gfx_d3d11_get_device();
    if (!dev) return;

    D3D11_BUFFER_DESC bd = {};
    D3D11_SUBRESOURCE_DATA sd = {};

    bd.ByteWidth = sizeof(k_cube_verts);
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    sd.pSysMem = k_cube_verts;
    if (FAILED(dev->CreateBuffer(&bd, &sd, &g_vbuf))) return;

    bd = {};
    bd.ByteWidth = sizeof(k_cube_idx);
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem = k_cube_idx;
    if (FAILED(dev->CreateBuffer(&bd, &sd, &g_ibuf))) return;

    bd = {};
    bd.ByteWidth = sizeof(ActorCB);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(dev->CreateBuffer(&bd, nullptr, &g_cbuf))) return;

    ComPtr<ID3DBlob> vs_blob, ps_blob, err;
    UINT cf = D3DCOMPILE_OPTIMIZATION_LEVEL2;

    HRESULT hr = D3DCompile(k_shader, sizeof(k_shader)-1, nullptr, nullptr, nullptr,
                             "VS", "vs_4_0", cf, 0, &vs_blob, &err);
    if (FAILED(hr)) {
        fprintf(stderr, "[actors] VS: %s\n", err ? (char*)err->GetBufferPointer() : "?");
        return;
    }

    hr = D3DCompile(k_shader, sizeof(k_shader)-1, nullptr, nullptr, nullptr,
                    "PS", "ps_4_0", cf, 0, &ps_blob, &err);
    if (FAILED(hr)) {
        fprintf(stderr, "[actors] PS: %s\n", err ? (char*)err->GetBufferPointer() : "?");
        return;
    }

    if (FAILED(dev->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &g_vs))) return;
    if (FAILED(dev->CreatePixelShader (ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &g_ps))) return;

    D3D11_INPUT_ELEMENT_DESC il[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    if (FAILED(dev->CreateInputLayout(il, 1, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &g_il))) return;

    D3D11_RASTERIZER_DESC rsd = {};
    rsd.FillMode = D3D11_FILL_SOLID;
    rsd.CullMode = D3D11_CULL_NONE;
    rsd.DepthClipEnable = TRUE;
    dev->CreateRasterizerState(&rsd, &g_raster);

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable    = TRUE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
    dev->CreateDepthStencilState(&dsd, &g_dss);

    D3D11_BLEND_DESC bld = {};
    bld.RenderTarget[0].BlendEnable           = TRUE;
    bld.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    bld.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    bld.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    bld.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    bld.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    bld.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    bld.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    dev->CreateBlendState(&bld, &g_blend);

    g_ready = true;
}

void actor_manager_update(void) {
    memcpy(g_actors, net_get_players(), sizeof(g_actors));
}

void actor_manager_render(void) {
    if (!g_ready) return;

    ID3D11DeviceContext    *ctx = gfx_d3d11_get_context();
    ID3D11RenderTargetView *rtv = gfx_d3d11_get_rtv();
    ID3D11DepthStencilView *dsv = gfx_d3d11_get_dsv();
    if (!ctx || !rtv) return;

    float w = 1.f, h = 1.f;
    gfx_d3d11_get_size(&w, &h);

    const float *eye_f = gLakituState.pos;
    const float *at_f  = gLakituState.focus;
    if (eye_f[0] == 0.f && eye_f[1] == 0.f && eye_f[2] == 0.f &&
        at_f[0]  == 0.f && at_f[1]  == 0.f && at_f[2]  == 0.f) return;

    float proj[4][4], view[4][4], vp[4][4];
    gfx_pc_get_proj_matrix(proj);
    mat4_look_at(view, eye_f, at_f);
    mat4_mul(vp, view, proj);

    ctx->OMSetRenderTargets(1, &rtv, dsv);
    D3D11_VIEWPORT vp_desc = {0.f, 0.f, w, h, 0.f, 1.f};
    ctx->RSSetViewports(1, &vp_desc);

    ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(g_il.Get());
    const UINT stride = sizeof(CubeVert), offset = 0;
    ctx->IASetVertexBuffers(0, 1, g_vbuf.GetAddressOf(), &stride, &offset);
    ctx->IASetIndexBuffer(g_ibuf.Get(), DXGI_FORMAT_R16_UINT, 0);
    ctx->VSSetShader(g_vs.Get(), nullptr, 0);
    ctx->PSSetShader(g_ps.Get(), nullptr, 0);
    ctx->VSSetConstantBuffers(0, 1, g_cbuf.GetAddressOf());
    ctx->PSSetConstantBuffers(0, 1, g_cbuf.GetAddressOf());
    ctx->RSSetState(g_raster.Get());
    ctx->OMSetDepthStencilState(g_dss.Get(), 0);
    const float blend_factor[4] = {1.f,1.f,1.f,1.f};
    ctx->OMSetBlendState(g_blend.Get(), blend_factor, 0xFFFFFFFF);

    for (int i = 0; i < NET_MAX_PLAYERS; i++) {
        const NetPlayer *p = &g_actors[i];
        if (!p->active) continue;

        float T[4][4], wvp_mat[4][4];
        mat4_identity(T);
        T[3][0] = p->x;
        T[3][1] = p->y + 40.f;
        T[3][2] = p->z;
        mat4_mul(wvp_mat, T, vp);

        ActorCB cb;
        memcpy(cb.wvp, wvp_mat, 64);
        const float *col = k_pid_colors[i & (NET_MAX_PLAYERS - 1)];
        cb.color[0] = col[0]; cb.color[1] = col[1];
        cb.color[2] = col[2]; cb.color[3] = col[3];

        D3D11_MAPPED_SUBRESOURCE ms = {};
        ctx->Map(g_cbuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, &cb, sizeof(cb));
        ctx->Unmap(g_cbuf.Get(), 0);

        ctx->DrawIndexed(36, 0, 0);
    }

    ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void actor_manager_shutdown(void) {
    g_blend.Reset();
    g_dss.Reset();
    g_raster.Reset();
    g_il.Reset();
    g_ps.Reset();
    g_vs.Reset();
    g_cbuf.Reset();
    g_ibuf.Reset();
    g_vbuf.Reset();
    g_ready = false;
}

#else

void actor_manager_init(void)     {}
void actor_manager_update(void)   {}
void actor_manager_render(void)   {}
void actor_manager_shutdown(void) {}

#endif
