using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public struct VecMatPair
{
    public Vector3 point;
    public Matrix4x4 matrix;
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
        int kernelHandle = shader.FindKernel("CSMain");
        RenderTexture tex = new RenderTexture(256, 256, 24);
        tex.enableRandomWrite = true;
        tex.Create();

        shader.SetTexture(kernelHandle, "Result", tex);
        shader.Dispatch(kernelHandle, 256 / 8, 256 / 8, 1);
    }
}
