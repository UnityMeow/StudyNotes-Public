# Unity学习笔记

## DLL相关

- 更新Unity中的DLL时必须关闭Unity后导入新DLL

- 声明DLL内的C++函数

  ```C++
  public unsafe static class TestDLL
  {
      [DllImport("DLLName")] public static extern void XXX(XXXXX);
      [DllImport("DLLName")] public static extern void XXX(XXXXX);
  }
  ```

## 字符串相关

```c#
// string转ASCII码单字节数组
byte[] a = Encoding.Default.GetBytes(str);
```

[Encoding官方文档](https://docs.microsoft.com/zh-cn/dotnet/standard/base-types/character-encoding)

## NativeArray

  ```c++
// 开辟N个T类型的内存空间 Allocator.Temp 释放类型 临时数据用temp即可
// NativeArrayOptions.UninitializedMemory不会将内存中数据置零，效率略高
NativeArray<T> test = new NativeArray<T>(count,Allocator.Temp,NativeArrayOptions.UninitializedMemory);
  ```

## Matrix4x4

[官方API文档](https://docs.unity3d.com/ScriptReference/Matrix4x4.html)

- transpose()  返回这个矩阵的转置矩阵(只读)
- Matrix4x4.inverse 这个矩阵的逆矩阵
- transform的right,up,forward,position尾+1竖放，则为transform的世界空间矩阵

## Mathematics

[官方API文档](https://docs.unity3d.com/Packages/com.unity.mathematics@1.1/api/Unity.Mathematics.html)

Unity.Mathematics.math.mul() 返回矩阵相乘结果

```c++
Matrix4x4 pos = transform.localToWorldMatrix;
float4 pos2 = mul(pos, new float4(-0.5f, 0.5f, 0.5f, 1));
Debug.Log(mul(pos.inverse, pos2));
Debug.Log(mul(pos2, ((Matrix4x4)transpose(pos)).inverse));
```

## Transform

transform.worldToLocalMatrix     将一个点从世界空间转换为局部空间的矩阵(只读)。

transform.localToWorldMatrix      将点从局部空间转换到世界空间的矩阵(只读)。

transform.forward 在世界坐标系中表示物体正前方
transform.right 在世界坐标系中表示物体正右方
transform.up 在世界坐标系中表示物体正上方

## UnsafeUtility

[UnsafeUtility官方API文档](https://docs.unity3d.com/ScriptReference/Unity.Collections.LowLevel.Unsafe.UnsafeUtility.html)

- unsafe关键字表示不安全上下文，类型或成员的整个正文范围均被视为不安全上下文，在该范围内可使用指针
- fixed 语句禁止垃圾回收器重定位可移动的变量，只能出现在不安全的上下文中，但由于unity使用Boehm GC，不会移动object的内存位置，暂时可以不用管
- UnsafeUtility.MemCpy()	C#中的内存拷贝API

## MipMap

unity会根据相机距离由近到远生成 8 张像素逐渐降低的贴图

优点：

减少显存带宽，降低渲染压力，随着相机的推远贴图会随之切换成低像素的体贴，从而节省资源开支

缺点：

增加游戏包体的大小和占用一定量的内存，过的Mipmap使用会对内存造成很大压力，建议只对大物件设置Mipmap

## CubeMap立方体贴图

CubeMap包含6个2D正方形纹理，每个2D纹理都是立方体的一个面，Up,Down,Forward,Back,Left,Right

通常用来作为具有反射属性物体的反射源,可以使用方向向量进行索引采样

待学习[CubeMap](https://blog.csdn.net/v_xchen_v/article/details/79474193)

## Shader相关

### Fixed Function Shader - 固定函数着色器

### Vertex Shader - 顶点着色器

### Fragment Shader - 片元着色器

### Surface Shader - 表面着色器

### Compute Shader - 计算着色器

- DX11新特性，独立于其他shader，不在传统的渲染管线流程内，不需要操作网格mesh和贴图texture数据，在OpenGL或DirectX存储空间中工作，相对独立的处理过程。并且能输出数据的缓冲或贴图，跨线程的执行分享存储。

- 把一些大计算量又不是特别复杂的运算直接交给GPU运算，即Compute Shader是在GPU上运行的程序。

- 游戏中可以使用Compute Shader实现光线追踪，A-Buffer采样抗锯齿，物理特性，AI运算，图像处理，PostProcessing等等。

- Unity Compute Shader必须由C#驱动
  1. C#创建渲染所需的信息包括模型、贴图、参数等然后调用Compute Shader
  2. Compute Shader计算出新的模型位置 对贴图进行处理 改变参数等
  3. Vertex Fragment Shader获得Compute Shader产生的信息进行渲染
  
- 优点

  涉及到大量的数学计算，并且是可以并行的没有很多分支的计算，都可以采用ComputeShader

- 缺点

  需要把数据从CPU传到GPU，然后GPU运算完之后，又从GPU传到CPU，这样的话可能会有点延迟，而且他们之间的传输速率也是一个瓶颈，但如果真的有大量的计算需求的话，还是不要犹豫地使用ComputeShader，它能带来的性能提升绝对是值得的

### Geometry Shader - 几何着色器

### Tessellation Shader - 细分曲面着色器

