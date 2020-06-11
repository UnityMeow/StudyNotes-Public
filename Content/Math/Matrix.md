### 矩阵相关

- Unity Matrix4x4 API相关

  - [官方API文档](https://docs.unity3d.com/ScriptReference/Matrix4x4.html)
  - 常用API
    - transpose()  返回这个矩阵的转置矩阵(只读)
    - Matrix4x4.inverse 这个矩阵的逆矩阵
  

transform的right,up,forward,position竖放，尾加0001，则为transform的矩阵

#### 矩阵乘法

横*竖

矩阵a * 单位矩阵 = 矩阵a

矩阵乘法有左右顺序

#### 三维单位矩阵

```
1,0,0,0
0,1,0,0
0,0,1,0
0,0,0,1
```

#### 三维平移矩阵

```
1,0,0,0
0,1,0,0
0,0,1,0
x,y,z,1
```

#### 三维缩放矩阵

```
x,0,0,0
0,y,0,0
0,0,z,0
0,0,0,1
```

#### 三维旋转矩阵

```
// 绕x轴旋转
1,0,0,0
0,c,s,0
0,-s,c,0
0,0,0,1

// 绕y轴旋转
c,0,-s,0
0,1,0,0
s,0,c,0
0,0,0,1

// 绕z轴旋转
c,s,0,0
-s,c,0,0
0,0,1,0
0,0,0,1
```

- 多种变换则将矩阵连乘，注意连乘顺序，顺序不同得到的结果是不同的
  

#### Unity Mathematics

[官方API文档](https://docs.unity3d.com/Packages/com.unity.mathematics@1.1/api/Unity.Mathematics.html)

Unity.Mathematics.math.mul() 返回矩阵相乘结果

```
Matrix4x4 pos = transform.localToWorldMatrix;
float4 pos2 = mul(pos, new float4(-0.5f, 0.5f, 0.5f, 1));
Debug.Log(mul(pos.inverse, pos2));
Debug.Log(mul(pos2, ((Matrix4x4)transpose(pos)).inverse));
```

