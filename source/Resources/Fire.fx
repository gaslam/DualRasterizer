//-------------------------------
// Global Variables
//-------------------------------

float4x4 gWorldViewProj: WorldViewProjection;
Texture2D gDiffuseMap: DiffuseMap;

SamplerState gStateToSample
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = wrap;
};

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = false;
    DepthFunc = less;
    StencilEnable = false;
};

//-------------------------------
// Input/Output Structs
//-------------------------------

struct VS_INPUT
{
    float3 Position: POSITION;
    float2 Uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 Uv: TEXCOORD;
};


//-------------------------------
// Vertex Shader
//-------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.Uv = input.Uv;
    return output;
}


float4 DoSomeCoolShading(VS_OUTPUT output)
{
    float4 diffuseSample = gDiffuseMap.Sample(gStateToSample,output.Uv);
    return diffuseSample;
}


//-------------------------------
// Pixel Shader
//-------------------------------

//Filtering method POINT
float4 PS(VS_OUTPUT output) : SV_TARGET
{
    float4 coolColor = DoSomeCoolShading(output);
    return coolColor;
}


//-------------------------------
// Technique
//-------------------------------

technique11 DefaultTechnique
{
    pass P0
    {
        SetDepthStencilState(gDepthStencilState,0);
        SetBlendState(gBlendState, float4(0.0f,0.0f,0.0f,0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0,VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS()));
    }
}