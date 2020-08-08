Shader "Unlit/BoneTest"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            // make fog work
            #pragma multi_compile_fog

            #include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;

            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                UNITY_FOG_COORDS(1)
                float4 vertex : SV_POSITION;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;

            struct BoneWeight
            {
                float4 weights;
                int4 indices;
            };

            StructuredBuffer<BoneWeight> _BoneWeightsBuffer;
            StructuredBuffer<float4x4> _BonePositionBuffer;
            v2f vert (appdata v, uint vertexID : SV_VERTEXID)
            {
                v2f o;
                BoneWeight weight = _BoneWeightsBuffer[vertexID];
                float4 worldPoses = 0;
                float sum = 0;
                for (uint i = 0; i < 4; ++i)
                {
                    sum += weight.weights[i];
                    worldPoses += weight.weights[i] * mul(_BonePositionBuffer[weight.indices[i]], v.vertex);
                }
                worldPoses /= sum;
                
                o.vertex = mul(UNITY_MATRIX_VP, worldPoses);
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);
                UNITY_TRANSFER_FOG(o,o.vertex);
                return o;
            }

            fixed4 frag(v2f i) : SV_Target
            {
                // sample the texture
                fixed4 col = tex2D(_MainTex, i.uv);
                // apply fog
                UNITY_APPLY_FOG(i.fogCoord, col);
                return col;
            }
            ENDCG
        }
    }
}
