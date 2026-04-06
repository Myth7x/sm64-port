#if defined(ENABLE_DX11) || defined(ENABLE_DX12)

#include <cstdio>
#include <cmath>

#include "gfx_direct3d_common.h"
#include "gfx_cc.h"

static GfxSSAOParams s_ssao_params = { true, false, 200.0f, 1.5f };

GfxSSAOParams *gfx_ssao_params(void) { return &s_ssao_params; }

bool gfx_d3d_mat4_inverse(const float m[16], float out[16]) {
    float inv[16];
    inv[0]  =  m[5]*m[10]*m[15]-m[5]*m[11]*m[14]-m[9]*m[6]*m[15]+m[9]*m[7]*m[14]+m[13]*m[6]*m[11]-m[13]*m[7]*m[10];
    inv[4]  = -m[4]*m[10]*m[15]+m[4]*m[11]*m[14]+m[8]*m[6]*m[15]-m[8]*m[7]*m[14]-m[12]*m[6]*m[11]+m[12]*m[7]*m[10];
    inv[8]  =  m[4]*m[9]*m[15]-m[4]*m[11]*m[13]-m[8]*m[5]*m[15]+m[8]*m[7]*m[13]+m[12]*m[5]*m[11]-m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14]+m[4]*m[10]*m[13]+m[8]*m[5]*m[14]-m[8]*m[6]*m[13]-m[12]*m[5]*m[10]+m[12]*m[6]*m[9];
    inv[1]  = -m[1]*m[10]*m[15]+m[1]*m[11]*m[14]+m[9]*m[2]*m[15]-m[9]*m[3]*m[14]-m[13]*m[2]*m[11]+m[13]*m[3]*m[10];
    inv[5]  =  m[0]*m[10]*m[15]-m[0]*m[11]*m[14]-m[8]*m[2]*m[15]+m[8]*m[3]*m[14]+m[12]*m[2]*m[11]-m[12]*m[3]*m[10];
    inv[9]  = -m[0]*m[9]*m[15]+m[0]*m[11]*m[13]+m[8]*m[1]*m[15]-m[8]*m[3]*m[13]-m[12]*m[1]*m[11]+m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14]-m[0]*m[10]*m[13]-m[8]*m[1]*m[14]+m[8]*m[2]*m[13]+m[12]*m[1]*m[10]-m[12]*m[2]*m[9];
    inv[2]  =  m[1]*m[6]*m[15]-m[1]*m[7]*m[14]-m[5]*m[2]*m[15]+m[5]*m[3]*m[14]+m[13]*m[2]*m[7]-m[13]*m[3]*m[6];
    inv[6]  = -m[0]*m[6]*m[15]+m[0]*m[7]*m[14]+m[4]*m[2]*m[15]-m[4]*m[3]*m[14]-m[12]*m[2]*m[7]+m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15]-m[0]*m[7]*m[13]-m[4]*m[1]*m[15]+m[4]*m[3]*m[13]+m[12]*m[1]*m[7]-m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14]+m[0]*m[6]*m[13]+m[4]*m[1]*m[14]-m[4]*m[2]*m[13]-m[12]*m[1]*m[6]+m[12]*m[2]*m[5];
    inv[3]  = -m[1]*m[6]*m[11]+m[1]*m[7]*m[10]+m[5]*m[2]*m[11]-m[5]*m[3]*m[10]-m[9]*m[2]*m[7]+m[9]*m[3]*m[6];
    inv[7]  =  m[0]*m[6]*m[11]-m[0]*m[7]*m[10]-m[4]*m[2]*m[11]+m[4]*m[3]*m[10]+m[8]*m[2]*m[7]-m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11]+m[0]*m[7]*m[9]+m[4]*m[1]*m[11]-m[4]*m[3]*m[9]-m[8]*m[1]*m[7]+m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10]-m[0]*m[6]*m[9]-m[4]*m[1]*m[10]+m[4]*m[2]*m[9]+m[8]*m[1]*m[6]-m[8]*m[2]*m[5];
    float det = m[0]*inv[0]+m[1]*inv[4]+m[2]*inv[8]+m[3]*inv[12];
    if (fabsf(det) < 1e-9f) return false;
    det = 1.0f / det;
    for (int i = 0; i < 16; i++) out[i] = inv[i] * det;
    return true;
}

void get_cc_features(uint32_t shader_id, CCFeatures *cc_features) {
    for (int i = 0; i < 4; i++) {
        cc_features->c[0][i] = (shader_id >> (i * 3)) & 7;
        cc_features->c[1][i] = (shader_id >> (12 + i * 3)) & 7;
    }

    cc_features->opt_alpha = (shader_id & SHADER_OPT_ALPHA) != 0;
    cc_features->opt_fog = (shader_id & SHADER_OPT_FOG) != 0;
    cc_features->opt_texture_edge = (shader_id & SHADER_OPT_TEXTURE_EDGE) != 0;
    cc_features->opt_noise = (shader_id & SHADER_OPT_NOISE) != 0;

    cc_features->used_textures[0] = false;
    cc_features->used_textures[1] = false;
    cc_features->num_inputs = 0;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (cc_features->c[i][j] >= SHADER_INPUT_1 && cc_features->c[i][j] <= SHADER_INPUT_4) {
                if (cc_features->c[i][j] > cc_features->num_inputs) {
                    cc_features->num_inputs = cc_features->c[i][j];
                }
            }
            if (cc_features->c[i][j] == SHADER_TEXEL0 || cc_features->c[i][j] == SHADER_TEXEL0A) {
                cc_features->used_textures[0] = true;
            }
            if (cc_features->c[i][j] == SHADER_TEXEL1) {
                cc_features->used_textures[1] = true;
            }
        }
    }

    cc_features->do_single[0] = cc_features->c[0][2] == 0;
    cc_features->do_single[1] = cc_features->c[1][2] == 0;
    cc_features->do_multiply[0] = cc_features->c[0][1] == 0 && cc_features->c[0][3] == 0;
    cc_features->do_multiply[1] = cc_features->c[1][1] == 0 && cc_features->c[1][3] == 0;
    cc_features->do_mix[0] = cc_features->c[0][1] == cc_features->c[0][3];
    cc_features->do_mix[1] = cc_features->c[1][1] == cc_features->c[1][3];
    cc_features->color_alpha_same = (shader_id & 0xfff) == ((shader_id >> 12) & 0xfff);
}

static void append_str(char *buf, size_t *len, const char *str) {
    while (*str != '\0') buf[(*len)++] = *str++;
}

static void append_line(char *buf, size_t *len, const char *str) {
    while (*str != '\0') buf[(*len)++] = *str++;
    buf[(*len)++] = '\r';
    buf[(*len)++] = '\n';
}

static const char *shader_item_to_str(uint32_t item, bool with_alpha, bool only_alpha, bool inputs_have_alpha, bool hint_single_element) {
    if (!only_alpha) {
        switch (item) {
            default:
            case SHADER_0:
                return with_alpha ? "float4(0.0, 0.0, 0.0, 0.0)" : "float3(0.0, 0.0, 0.0)";
            case SHADER_INPUT_1:
                return with_alpha || !inputs_have_alpha ? "input.input1" : "input.input1.rgb";
            case SHADER_INPUT_2:
                return with_alpha || !inputs_have_alpha ? "input.input2" : "input.input2.rgb";
            case SHADER_INPUT_3:
                return with_alpha || !inputs_have_alpha ? "input.input3" : "input.input3.rgb";
            case SHADER_INPUT_4:
                return with_alpha || !inputs_have_alpha ? "input.input4" : "input.input4.rgb";
            case SHADER_TEXEL0:
                return with_alpha ? "texVal0" : "texVal0.rgb";
            case SHADER_TEXEL0A:
                return hint_single_element ? "texVal0.a" : (with_alpha ? "float4(texVal0.a, texVal0.a, texVal0.a, texVal0.a)" : "float3(texVal0.a, texVal0.a, texVal0.a)");
            case SHADER_TEXEL1:
                return with_alpha ? "texVal1" : "texVal1.rgb";
        }
    } else {
        switch (item) {
            default:
            case SHADER_0:
                return "0.0";
            case SHADER_INPUT_1:
                return "input.input1.a";
            case SHADER_INPUT_2:
                return "input.input2.a";
            case SHADER_INPUT_3:
                return "input.input3.a";
            case SHADER_INPUT_4:
                return "input.input4.a";
            case SHADER_TEXEL0:
                return "texVal0.a";
            case SHADER_TEXEL0A:
                return "texVal0.a";
            case SHADER_TEXEL1:
                return "texVal1.a";
        }
    }
}

static void append_formula(char *buf, size_t *len, const uint8_t c[2][4], bool do_single, bool do_multiply, bool do_mix, bool with_alpha, bool only_alpha, bool opt_alpha) {
    if (do_single) {
        append_str(buf, len, shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    } else if (do_multiply) {
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " * ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
    } else if (do_mix) {
        append_str(buf, len, "lerp(");
        append_str(buf, len, shader_item_to_str(c[only_alpha][1], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ", ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ", ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
        append_str(buf, len, ")");
    } else {
        append_str(buf, len, "(");
        append_str(buf, len, shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " - ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][1], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ") * ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
        append_str(buf, len, " + ");
        append_str(buf, len, shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    }
}

void gfx_direct3d_common_build_shader(char buf[16384], size_t& len, size_t& num_floats, const CCFeatures& cc_features, bool include_root_signature, bool three_point_filtering) {
    len = 0;
    num_floats = 4 + 3; // position (4) + worldPos (3)

    // Pixel shader input struct

    if (include_root_signature) {
        append_str(buf, &len, "#define RS \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_VERTEX_SHADER_ROOT_ACCESS)");
        if (cc_features.opt_alpha && cc_features.opt_noise) {
            append_str(buf, &len, ",CBV(b0, visibility = SHADER_VISIBILITY_PIXEL)");
        }
        if (cc_features.used_textures[0]) {
            append_str(buf, &len, ",DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL)");
            append_str(buf, &len, ",DescriptorTable(Sampler(s0), visibility = SHADER_VISIBILITY_PIXEL)");
        }
        if (cc_features.used_textures[1]) {
            append_str(buf, &len, ",DescriptorTable(SRV(t1), visibility = SHADER_VISIBILITY_PIXEL)");
            append_str(buf, &len, ",DescriptorTable(Sampler(s1), visibility = SHADER_VISIBILITY_PIXEL)");
        }
        append_str(buf, &len, ",CBV(b2, visibility = SHADER_VISIBILITY_PIXEL)");
        append_str(buf, &len, ",DescriptorTable(SRV(t2, numDescriptors=4), visibility = SHADER_VISIBILITY_PIXEL)");
        append_str(buf, &len, ",DescriptorTable(Sampler(s2), visibility = SHADER_VISIBILITY_PIXEL)");
        append_line(buf, &len, "\"");
    }

    append_line(buf, &len, "struct PSInput {");
    append_line(buf, &len, "    float4 position : SV_POSITION;");
    append_line(buf, &len, "    float3 worldPos : WORLDPOS;");
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        append_line(buf, &len, "    float2 uv : TEXCOORD;");
        num_floats += 2;
    }
    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "    float4 screenPos : TEXCOORD1;");
    }
    if (cc_features.opt_fog) {
        append_line(buf, &len, "    float4 fog : FOG;");
        num_floats += 4;
    }
    for (int i = 0; i < cc_features.num_inputs; i++) {
        len += sprintf(buf + len, "    float%d input%d : INPUT%d;\r\n", cc_features.opt_alpha ? 4 : 3, i + 1, i);
        num_floats += cc_features.opt_alpha ? 4 : 3;
    }
    append_line(buf, &len, "};");

    // Textures and samplers

    if (cc_features.used_textures[0]) {
        append_line(buf, &len, "Texture2D g_texture0 : register(t0);");
        append_line(buf, &len, "SamplerState g_sampler0 : register(s0);");
    }
    if (cc_features.used_textures[1]) {
        append_line(buf, &len, "Texture2D g_texture1 : register(t1);");
        append_line(buf, &len, "SamplerState g_sampler1 : register(s1);");
    }

    // Constant buffer and random function

    append_line(buf, &len, "cbuffer LightsCB : register(b2) {");
    append_line(buf, &len, "    float4 ambient_color;");
    append_line(buf, &len, "    float4 light_pos[8];");
    append_line(buf, &len, "    float4 light_color[8];");
    append_line(buf, &len, "    row_major float4x4 shadow_vp[4];");
    append_line(buf, &len, "    int shadow_count;");
    append_line(buf, &len, "    float3 _lpad;");
    append_line(buf, &len, "    float4 sun_dir;");
    append_line(buf, &len, "    float4 sun_color;");
    append_line(buf, &len, "}");

    append_line(buf, &len, "Texture2D shadow_map_0 : register(t2);");
    append_line(buf, &len, "Texture2D shadow_map_1 : register(t3);");
    append_line(buf, &len, "Texture2D shadow_map_2 : register(t4);");
    append_line(buf, &len, "Texture2D shadow_map_3 : register(t5);");
    append_line(buf, &len, "SamplerComparisonState shadow_sampler : register(s2);");

    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "cbuffer PerFrameCB : register(b0) {");
        append_line(buf, &len, "    uint noise_frame;");
        append_line(buf, &len, "    float2 noise_scale;");
        append_line(buf, &len, "}");

        append_line(buf, &len, "float random(in float3 value) {");
        append_line(buf, &len, "    float random = dot(value, float3(12.9898, 78.233, 37.719));");
        append_line(buf, &len, "    return frac(sin(random) * 143758.5453);");
        append_line(buf, &len, "}");
    }

    // 3 point texture filtering
    // Original author: ArthurCarvalho
    // Based on GLSL implementation by twinaphex, mupen64plus-libretro project.

    if (three_point_filtering && (cc_features.used_textures[0] || cc_features.used_textures[1])) {
        append_line(buf, &len, "cbuffer PerDrawCB : register(b1) {");
        append_line(buf, &len, "    struct {");
        append_line(buf, &len, "        uint width;");
        append_line(buf, &len, "        uint height;");
        append_line(buf, &len, "        bool linear_filtering;");
        append_line(buf, &len, "    } textures[2];");
        append_line(buf, &len, "}");
        append_line(buf, &len, "#define TEX_OFFSET(tex, tSampler, texCoord, off, texSize) tex.Sample(tSampler, texCoord - off / texSize)");
        append_line(buf, &len, "float4 tex2D3PointFilter(in Texture2D tex, in SamplerState tSampler, in float2 texCoord, in float2 texSize) {");
        append_line(buf, &len, "    float2 offset = frac(texCoord * texSize - float2(0.5, 0.5));");
        append_line(buf, &len, "    offset -= step(1.0, offset.x + offset.y);");
        append_line(buf, &len, "    float4 c0 = TEX_OFFSET(tex, tSampler, texCoord, offset, texSize);");
        append_line(buf, &len, "    float4 c1 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x - sign(offset.x), offset.y), texSize);");
        append_line(buf, &len, "    float4 c2 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x, offset.y - sign(offset.y)), texSize);");
        append_line(buf, &len, "    return c0 + abs(offset.x)*(c1-c0) + abs(offset.y)*(c2-c0);");
        append_line(buf, &len, "}");
    }

    // Vertex shader

    append_str(buf, &len, "PSInput VSMain(float4 position : POSITION");
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        append_str(buf, &len, ", float2 uv : TEXCOORD");
    }
    if (cc_features.opt_fog) {
        append_str(buf, &len, ", float4 fog : FOG");
    }
    for (int i = 0; i < cc_features.num_inputs; i++) {
        len += sprintf(buf + len, ", float%d input%d : INPUT%d", cc_features.opt_alpha ? 4 : 3, i + 1, i);
    }
    append_str(buf, &len, ", float3 worldPos : WORLDPOS");
    append_line(buf, &len, ") {");
    append_line(buf, &len, "    PSInput result;");
    append_line(buf, &len, "    result.position = position;");
    append_line(buf, &len, "    result.worldPos = worldPos;");
    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "    result.screenPos = position;");
    }
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        append_line(buf, &len, "    result.uv = uv;");
    }
    if (cc_features.opt_fog) {
        append_line(buf, &len, "    result.fog = fog;");
    }
    for (int i = 0; i < cc_features.num_inputs; i++) {
        len += sprintf(buf + len, "    result.input%d = input%d;\r\n", i + 1, i + 1);
    }
    append_line(buf, &len, "    return result;");
    append_line(buf, &len, "}");

    // Pixel shader
    if (include_root_signature) {
        append_line(buf, &len, "[RootSignature(RS)]");
    }
    append_line(buf, &len, "float4 PSMain(PSInput input) : SV_TARGET {");
    if (cc_features.used_textures[0]) {
        if (three_point_filtering) {
            append_line(buf, &len, "    float4 texVal0;");
            append_line(buf, &len, "    if (textures[0].linear_filtering)");
            append_line(buf, &len, "        texVal0 = tex2D3PointFilter(g_texture0, g_sampler0, input.uv, float2(textures[0].width, textures[0].height));");
            append_line(buf, &len, "    else");
            append_line(buf, &len, "        texVal0 = g_texture0.Sample(g_sampler0, input.uv);");
        } else {
            append_line(buf, &len, "    float4 texVal0 = g_texture0.Sample(g_sampler0, input.uv);");
        }
    }
    if (cc_features.used_textures[1]) {
        if (three_point_filtering) {
            append_line(buf, &len, "    float4 texVal1;");
            append_line(buf, &len, "    if (textures[1].linear_filtering)");
            append_line(buf, &len, "        texVal1 = tex2D3PointFilter(g_texture1, g_sampler1, input.uv, float2(textures[1].width, textures[1].height));");
            append_line(buf, &len, "    else");
            append_line(buf, &len, "        texVal1 = g_texture1.Sample(g_sampler1, input.uv);");
        } else {
            append_line(buf, &len, "    float4 texVal1 = g_texture1.Sample(g_sampler1, input.uv);");
        }
    }

    append_str(buf, &len, cc_features.opt_alpha ? "    float4 texel = " : "    float3 texel = ");
    if (!cc_features.color_alpha_same && cc_features.opt_alpha) {
        append_str(buf, &len, "float4(");
        append_formula(buf, &len, cc_features.c, cc_features.do_single[0], cc_features.do_multiply[0], cc_features.do_mix[0], false, false, true);
        append_str(buf, &len, ", ");
        append_formula(buf, &len, cc_features.c, cc_features.do_single[1], cc_features.do_multiply[1], cc_features.do_mix[1], true, true, true);
        append_str(buf, &len, ")");
    } else {
        append_formula(buf, &len, cc_features.c, cc_features.do_single[0], cc_features.do_multiply[0], cc_features.do_mix[0], cc_features.opt_alpha, false, cc_features.opt_alpha);
    }
    append_line(buf, &len, ";");

    if (cc_features.opt_texture_edge && cc_features.opt_alpha) {
        append_line(buf, &len, "    if (texel.a > 0.3) texel.a = 1.0; else discard;");
    }
    // TODO discard if alpha is 0?
    if (cc_features.opt_fog) {
        if (cc_features.opt_alpha) {
            append_line(buf, &len, "    texel = float4(lerp(texel.rgb, input.fog.rgb, input.fog.a), texel.a);");
        } else {
            append_line(buf, &len, "    texel = lerp(texel, input.fog.rgb, input.fog.a);");
        }
    }

    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "    float2 coords = (input.screenPos.xy / input.screenPos.w) * noise_scale;");
        append_line(buf, &len, "    texel.a *= round(random(float3(floor(coords), noise_frame)));");
    }

    if (cc_features.opt_alpha) {
        append_line(buf, &len, "    float3 lsum = ambient_color.rgb * ambient_color.a;");
        append_line(buf, &len, "    [unroll] for (int li = 0; li < 8; li++) {");
        append_line(buf, &len, "        float3 delta = light_pos[li].xyz - input.worldPos;");
        append_line(buf, &len, "        float dist2 = dot(delta, delta);");
        append_line(buf, &len, "        float r = light_pos[li].w;");
        append_line(buf, &len, "        float attn = saturate(1.0 - dist2 / (r * r + 0.0001));");
        append_line(buf, &len, "        lsum += light_color[li].rgb * light_color[li].a * attn;");
        append_line(buf, &len, "    }");
        append_line(buf, &len, "    { float3 _dn=cross(ddx(input.worldPos),ddy(input.worldPos)); float _l=dot(_dn,_dn); if(_l>1e-10f){_dn*=rsqrt(_l);if(dot(_dn,sun_dir.xyz)<0.0f)_dn=-_dn;lsum+=sun_color.rgb*sun_color.a*saturate(dot(_dn,sun_dir.xyz));} }");
        append_line(buf, &len, "    if (input.position.z >= 0.9999f) lsum = float3(1.0f, 1.0f, 1.0f);");
        append_line(buf, &len, "    texel.rgb = saturate(texel.rgb * lsum);");

        append_line(buf, &len, "    float shadow_factor = 1.0;");
        append_line(buf, &len, "    if (input.position.z < 0.9999f) {");
        append_line(buf, &len, "    float2 s_ts = float2(1.0/1024.0, 1.0/1024.0);");
        append_line(buf, &len, "    [branch] if (shadow_count > 0) { float4 sc=mul(shadow_vp[0],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_0.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    [branch] if (shadow_count > 1) { float4 sc=mul(shadow_vp[1],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_1.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    [branch] if (shadow_count > 2) { float4 sc=mul(shadow_vp[2],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_2.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    [branch] if (shadow_count > 3) { float4 sc=mul(shadow_vp[3],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_3.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    }");
        append_line(buf, &len, "    texel.rgb *= shadow_factor;");
        append_line(buf, &len, "    return texel;");
    } else {
        append_line(buf, &len, "    float3 lsum = ambient_color.rgb * ambient_color.a;");
        append_line(buf, &len, "    [unroll] for (int li = 0; li < 8; li++) {");
        append_line(buf, &len, "        float3 delta = light_pos[li].xyz - input.worldPos;");
        append_line(buf, &len, "        float dist2 = dot(delta, delta);");
        append_line(buf, &len, "        float r = light_pos[li].w;");
        append_line(buf, &len, "        float attn = saturate(1.0 - dist2 / (r * r + 0.0001));");
        append_line(buf, &len, "        lsum += light_color[li].rgb * light_color[li].a * attn;");
        append_line(buf, &len, "    }");
        append_line(buf, &len, "    { float3 _dn=cross(ddx(input.worldPos),ddy(input.worldPos)); float _l=dot(_dn,_dn); if(_l>1e-10f){_dn*=rsqrt(_l);if(dot(_dn,sun_dir.xyz)<0.0f)_dn=-_dn;lsum+=sun_color.rgb*sun_color.a*saturate(dot(_dn,sun_dir.xyz));} }");
        append_line(buf, &len, "    if (input.position.z >= 0.9999f) lsum = float3(1.0f, 1.0f, 1.0f);");
        append_line(buf, &len, "    texel = saturate(texel * lsum);");

        append_line(buf, &len, "    float shadow_factor = 1.0;");
        append_line(buf, &len, "    if (input.position.z < 0.9999f) {");
        append_line(buf, &len, "    float2 s_ts = float2(1.0/1024.0, 1.0/1024.0);");
        append_line(buf, &len, "    [branch] if (shadow_count > 0) { float4 sc=mul(shadow_vp[0],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_0.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    [branch] if (shadow_count > 1) { float4 sc=mul(shadow_vp[1],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_1.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    [branch] if (shadow_count > 2) { float4 sc=mul(shadow_vp[2],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_2.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    [branch] if (shadow_count > 3) { float4 sc=mul(shadow_vp[3],float4(input.worldPos,1.0)); if(sc.w>0.0){sc.xyz/=sc.w; float2 suv=float2(sc.x*0.5+0.5,1.0-(sc.y*0.5+0.5)); if(suv.x>=0.0&&suv.x<=1.0&&suv.y>=0.0&&suv.y<=1.0&&sc.z>=0.0&&sc.z<=1.0){float sh=0.0; [unroll] for(int ky=-1;ky<=1;ky++){[unroll] for(int kx=-1;kx<=1;kx++) sh+=shadow_map_3.SampleCmpLevelZero(shadow_sampler,suv+float2(kx,ky)*s_ts,sc.z-0.00001);} shadow_factor=min(shadow_factor,lerp(0.35,1.0,sh/9.0));}} }");
        append_line(buf, &len, "    }");
        append_line(buf, &len, "    texel *= shadow_factor;");
        append_line(buf, &len, "    return float4(texel, 1.0);");
    }
    append_line(buf, &len, "}");
}

#endif
