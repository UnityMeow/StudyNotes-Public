using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public struct VecMatPair
{
    public Vector4 a;
    public Vector2 b;
}

public class TestMain : MonoBehaviour
{
    public ComputeShader shader;
    private void Start()
    {
        RunShader();
    }
    private void RunShader()
    {
        // 要传入的数据
        VecMatPair[] data = new VecMatPair[128];
        // 创建ComputeBuffer(数据个数，单个数据大小)
        ComputeBuffer buffer = new ComputeBuffer(data.Length, 24);
        // 设置数据
        for(uint i = 0; i < data.Length; ++i)
        {
            data[i].a = Vector4.zero;
            data[i].b = Vector2.one;
        }
        // 数据塞入ComputeBuffer
        buffer.SetData(data);
        // 设置到相应的buffer(数字0与shader.FindKernel("CSMain")同等效果  与CS中的#pra gma kernel CSMain排列顺序有关, 
                               // CS中StructuredBuffer名字符串，
                               // 要传入的buffer)
        shader.SetBuffer(0, "_TestStruct", buffer);
        // 设置发送数组长度 吧~
        shader.Dispatch(0, data.Length, 1, 1);
        // 数据回读
        buffer.GetData(data);
        for (uint i = 0; i < data.Length; ++i)
        {
            Debug.Log(data[i].a + "  " + data[i].b);
        }
    }
}
