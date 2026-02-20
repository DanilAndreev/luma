struct VSIn {
    float4 position: POSITION;
    float4 normal: NORMAL;
    float4 texcoor0: TEXCOORD0;
    float4 color0: COLOR0;
}

struct VSOut {
    float4 position: POSITION;
    float4 normal: NORMAL;
    float4 texcoor0: TEXCOORD0;
    float4 color0: COLOR0;
}

struct PSOut {
    float4 color: COLOR0;
}

VSOut VSMain(VSIn input) {
    VSOut output{};
    
    return output;
}

PSOut PSMain(VSOut input) {
    PSOut output{};

    return output;
}