using System.Collections;
using System.Collections.Generic;
using UnityEngine;
public unsafe class MeowSkinned : MonoBehaviour
{
    /// <summary>
    /// Mesh网格信息
    /// </summary>
    private Mesh mesh;
    /// <summary>
    /// 每个骨骼预定义bone的 bone空间 到 mesh空间 的变换矩阵的 逆矩阵
    /// 直白一点就是 mesh相对于骨骼的坐标
    /// </summary>
    private Matrix4x4[] bindPoses;
    /// <summary>
    /// 骨骼信息
    /// </summary>
    private Transform[] bones;
    /// <summary>
    /// 骨骼的相对位置
    /// </summary>
    private Matrix4x4[] bonePositions;
    /// <summary>
    /// 每个顶点的骨骼权重信息
    /// </summary>
    private BoneWeight[] boneWeights;

    /// <summary>
    /// 解决改一个mat其它全改的问题
    /// </summary>
    private MaterialPropertyBlock rendererBlock;
    
    /// <summary>
    /// 临时借用的skinnedMeshRenderer数据
    /// </summary>
    private SkinnedMeshRenderer meshRenderer;

    //传入Shader的Buffer
    private ComputeBuffer bonePositionBuffer;
    private ComputeBuffer boneWeightBuffer;

    /// <summary>
    /// 最终要画上去的meshRenderer
    /// </summary>
    private MeshRenderer targetRenderer;
    private MeshFilter filter;

    private void Awake()
    {
        meshRenderer = GetComponent<SkinnedMeshRenderer>();
        rendererBlock = new MaterialPropertyBlock();
        mesh = meshRenderer.sharedMesh;
        bindPoses = mesh.bindposes;
        boneWeights = mesh.boneWeights;
        bones = new Transform[meshRenderer.bones.Length];
        for (uint i = 0; i < bones.Length; ++i)
        {
            bones[i] = meshRenderer.bones[i];
        }

     
        bonePositionBuffer = new ComputeBuffer(
            bones.Length,
            sizeof(Matrix4x4));
        boneWeightBuffer = new ComputeBuffer(
            boneWeights.Length,
            sizeof(BoneWeight));

      
        boneWeightBuffer.SetData(boneWeights);
        bonePositions = new Matrix4x4[bones.Length];
        filter = gameObject.AddComponent<MeshFilter>();
        filter.sharedMesh = mesh;
        targetRenderer = gameObject.AddComponent<MeshRenderer>();
        targetRenderer.sharedMaterial = meshRenderer.sharedMaterial;
        Destroy(meshRenderer);
    }

    private void Update()
    {
        //计算每根骨骼的相对位置
        for (int i = 0; i < bonePositions.Length; ++i)
        {
            bonePositions[i] = bones[i].localToWorldMatrix * bindPoses[i];
        }
        bonePositionBuffer.SetData(bonePositions);
        //向GPU传值
        rendererBlock.Clear();
        rendererBlock.SetBuffer("_BoneWeightsBuffer", boneWeightBuffer);
        rendererBlock.SetBuffer("_BonePositionBuffer", bonePositionBuffer);
        targetRenderer.SetPropertyBlock(rendererBlock);
    }

    private void OnDestroy()
    {
        bonePositionBuffer.Dispose();
        boneWeightBuffer.Dispose();
    }
}
