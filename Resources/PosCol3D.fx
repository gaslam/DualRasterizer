//-------------------------------
// Global Variables
//-------------------------------

float4x4 gWorldViewProj: WorldViewProjection;
Texture2D gDiffuseMap: DiffuseMap;
Texture2D gNormalMap: NormalMap;
Texture2D gSpecularMap: SpecularMap;
Texture2D gGlossinesMap: GlossinesMap;
float4x4 gWorldMatrix: WORLD;
float4x4 gInvViewMatrix: VIEWINVERSE;
float gPI = 3.14159265358979323846f;
float gLightIntensity = 7.0f;
float gShininess = 25.0f;

SamplerState gStateToSample{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
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
    DepthWriteMask = true;
    DepthFunc = less;
    StencilEnable = true;

    StencilReadMask = 0x0F;
    StencilWriteMask = 0x0F;
};

//-------------------------------
// Input/Output Structs
//-------------------------------

struct VS_INPUT
{
    float3 Position: POSITION;
    float2 Uv : TEXCOORD;
    float4 Color: COLOR;
    float3 Normal: NORMAL;
    float3 Tangent: TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition: COLOR;
    float3 Normal: NORMAL;
    float3 Tangent: TANGENT;
    float2 Uv: TEXCOORD;
};


//-------------------------------
// Vertex Shader
//-------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);

    output.Tangent = mul(normalize(input.Tangent),(float3x3)gWorldMatrix);
    output.Normal = mul(normalize(input.Normal),(float3x3)gWorldMatrix);
    output.Uv = input.Uv;
    return output;
}

float4 Phong(float ks, float exp, const float3 l, const float3 v, const float3 n)
{
	float3 reflectedVar = float3(reflect(l,n));
    float floatDotReflect = dot(reflectedVar, v);
	float cosReflect = ks * pow(saturate(floatDotReflect),exp);
	return float4(cosReflect, cosReflect, cosReflect,cosReflect);
}

float4 Specular(VS_OUTPUT output, const float3 vectorNormal,const float3 lightDirection)
{
    float4 sampledSpecularColor = gSpecularMap.Sample(gStateToSample,output.Uv);
    float3 phongExponent = gGlossinesMap.Sample(gStateToSample,output.Uv);
    float3 viewDirection = normalize(output.WorldPosition.xyz - gInvViewMatrix[3].xyz);
    float4 phong = Phong(1.f,phongExponent.r * gShininess,lightDirection,-viewDirection,vectorNormal);
    float4 phongMulSampledColor = mul(phong,sampledSpecularColor);
    return phongMulSampledColor;
}

float3 Lambert(const float3 kd, const float3 cd)
{

	return cd * kd / gPI;
}

float4 Diffuse(float2 uv, float observedArea)
{
    const float4 lightColor= gDiffuseMap.Sample(gStateToSample,uv);
    const float3 radiance= mul(lightColor.rgb,gLightIntensity);

    float3 lambertColor = Lambert(observedArea,radiance);
    return float4(lambertColor, 1.f);
}

float4 DoSomeCoolShading(VS_OUTPUT output)
{
    float3 lightDirection = float3(0.577f,-0.577f,0.577f );

        float3 binormal = cross(output.Normal,output.Tangent);

float3x3 fMatrix = { output.Tangent.x, output.Tangent.y,output.Tangent.z, // row 1
                     binormal.x, binormal.y,binormal.z,
                     output.Normal.x,output.Normal.y,output.Normal.z
                   };
        float4 sampledNormalColor = gNormalMap.Sample(gStateToSample,output.Uv);
        float3 sampledNormal = sampledNormalColor.rgb;
        sampledNormal = 2.f * sampledNormal - 1.f;
        float3 vectorNormal = mul(sampledNormal,fMatrix);
        vectorNormal = normalize(vectorNormal);

    float observedArea=saturate(dot(-lightDirection,vectorNormal));

    clamp(observedArea,0, 1);
    
    return Specular(output, vectorNormal, -lightDirection) + Diffuse(output.Uv, observedArea);
}


//-------------------------------
// Pixel Shader
//-------------------------------

//Filtering method POINT
float4 PS(VS_OUTPUT output) : SV_TARGET
{
    float3 coolColor = DoSomeCoolShading(output);
    return float4(coolColor,1.f);
}


//-------------------------------
// Technique
//-------------------------------

technique11 DefaultTechnique
{
    pass P0
    {
        SetDepthStencilState(gDepthStencilState,1);
        SetBlendState(gBlendState, float4(0.0f,0.0f,0.0f,0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0,VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0,PS()));
    }
}