struct VSIn
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 texcoor0 : TEXCOORD0;
    float4 color0 : COLOR0;
};

struct VSOut
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 texcoor0 : TEXCOORD0;
    float4 color0 : COLOR0;
};

struct PSOut
{
    float4 color : SV_Target;
};

VSOut VSMain(VSIn input) {
    VSOut output;
    output.position = input.position;
    output.normal = input.normal;
    output.texcoor0 = input.texcoor0;
    output.color0 = input.color0;
    return output;
}

PSOut PSMain(VSOut input) {
    PSOut output;
    output.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    return output;
}