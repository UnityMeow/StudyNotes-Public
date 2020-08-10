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
    /// mesh转换骨骼空间坐标系 
    /// (mesh->bone)
    /// </summary>
    private Matrix4x4[] bindPoses;
    /// <summary>
    /// 骨骼信息
    /// bones.localToWorldMatrix (bone->world)
    /// </summary>
    private Transform[] bones;
    /// <summary>
    /// mesh->world
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
        for (int i = 0; i < bonePositions.Length; ++i)
        {
            // mesh坐标系转世界坐标系
            bonePositions[i] = bones[i].localToWorldMatrix/* 骨骼坐标系转世界坐标系 */ * bindPoses[i]/* mesh坐标系转骨骼坐标系 */;
        }
        bonePositionBuffer.SetData(bonePositions);
        // 向GPU传值
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
