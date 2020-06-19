# DirectX12 3D 学习笔记

[官网API文档](https://docs.microsoft.com/en-us/windows/win32/api/)

[官方代码示例库](https://github.com/d3dcoder/d3d12book)

## 数学知识

### 向量代数

#### 向量Vector

表示大小和方向的量，同一个向量在不同坐标系中有着不同的对应坐标表示

Direct3D采用左手坐标系

向量加法：同维向量之间才可以进行加法运算

标量乘法(scalar multiplication)：向量与标量相乘

向量减法：可以通过向量加法和标量乘法表示v1 - v2 = v1  + （-1 * v2） = v1 + （-v2）

#### 长度和单位向量

向量的长度（模）：向量的大小

单位化向量（normalizing）：把一个向量的长度变为单位长度

#### 点积（dot product）

计算结果为标量的向量乘法运算

v1 * v2 = 0    两向量正交 v1 ⊥ v2

v1 * v2 > 0    两向量之间夹角小于90°

v1 * v2 < 0    两向量之间夹角大于90°

正交投影 

规范正交

#### 叉积（cross product）

计算结果为向量的向量乘法运算

只有3D向量的叉积有定义，不存在2D叉积

v1 × v2 = v3    v3与v1、v2彼此正交

叉积的反交换定律

v1 × v2 ≠ v2 × v1

v1 × v2 = -v2 × v1

2D向量伪叉积：

v1 × v2 = (v1.x, v1.y) * (-v1.y, v1.x) = 0     v1 ⊥ v2 ，v1 ⊥ -v2

#### 点

位置向量：点仅表示位置，向量表示着大小方向

#### DirectXMath API 向量运算

- Direct3D应用程序的3D数学库，借助128位宽的单指令多数据寄存器（SIMD），利用一条SIMD指令即可同时对4个32位浮点数或整数进行运算。比普通向量运算效率高

- 使用directXMath要导入的头文件

  `#include <DirectXPackedVector.h>`

  所有代码都以内联的方式实现在头文件里，DirectXMath.h 文件中的代码都存在于DirectX命名空间中，DirectXPackedVector.h文件中的代码都位于`DirectX::PackedVector`命名空间内

- x86平台需要启用SSE2指令集，x64则不需要

- 核心向量类型：XMVECTOR

  它会被映射到SIMD硬件寄存器，通过SIMD指令配合进行向量运算处理

  `typedef __m128 XMVECTOR;`

  __m128是一种特殊的SIMD类型，在计算向量的过程中，必须通过此类型才可充分利用SIMD技术，在计算过程中用不到的向量分量则将置零并忽略

- XMVECTOR类型数据需要按16字节对齐，这对局部变量全局变量都是自动实现的，而类中的数据成员需要使用XMFLOAT2、XMFLOAT3、XMFLOAT4来代替，但如果将这些类型直接用于计算则无法充分发挥SIMD的高效特性，所以要将这些类型的实例转为XMVECTOR类型

  XMFLOAT*n* 转 XMVECTOR：

  `XMVECTOR XM_CALLCONV XMLoadFloat2(const XMFLOAT2* pSource);`

  XMVECTOR 转 XMFLOAT*n*：

  `void XM_CALLCONV XMStoreFloat2(XMFLOAT2* pDestination, FXMVECTOR V);`

  XMVECTOR中取分量：

  `float XM_CALLCONV XMVectorGetX(FXMVECTOR V);`

  分量 转 XMVECTOR：

  `XMVECTOR XM_CALLCONV XMVectorSetX(FXMVECTOR V, float x);`

  为了代码更具通用性，不受具体平台编译器的影响

  - XM_CALLCONV 调用约定注解：会根据编译器版本确定对应的调用约定属性， 一点要把XM_CALLCONV加到函数名之前

  - 为了提高效率可以将XMVECTOR类型的值作为函数的参数直接传到SSE寄存器里，而不是栈里，此方式传递的参数数量取决于用户使用的平台和编译器，所以要用FXMVECTOR、GXMVECTOR、HXMVECTOR、CXMVECTOR

    - 传递规则：

      前3个 FXMVECTOR

      第4个 GXMVECTOR

      第5、6个 HXMVECTOR

      其余的 CXMVECTOR

- 常用API

  - 向量模长

    `XMVECTOR XM_CALLCONV XMVector3Length();`

  - 向量模的平方

    `XMVECTOR XM_CALLCONV XMVector3LengthSq();`

  - 两个向量的点积

    `XMVECTOR XM_CALLCONV XMVector3Dot();`

  - 两个向量的叉积

    `XMVECTOR XM_CALLCONV XMVector3Cross();`

  - 单位向量

    `XMVECTOR XM_CALLCONV XMVector3Normalize();`
    
  - 使用一个参考法线向量，将一个三维向量分成平行和垂直与法线的分量
  
    `XM_CALLCONV XMVector3ComponentsFromNormal(XMVECTOR* pParallel,  XMVECTOR* pPerpendicular,FXMVECTOR V,FXMVECTOR Normal);`
  
  - 返回两个向量之间的弧度
  
    `XMVECTOR XMVector3AngleBetweenVectors(FXMVECTOR v1, FXMVECTOR v2);`
  
  - 测试两个向量是否相等
  
    `XM_CALLCONV XMVector3Equal(FXMVECTOR V1, FXMVECTOR V2 );`
  
  - 测试两个向量是否不相等
  
    `XM_CALLCONV XMVector3NotEqual(FXMVECTOR V1, FXMVECTOR V2);`
  
  - 弧度转角度
  
    `XM_CONSTEXPR float XMConvertToDegrees(float fRadians);`

### 矩阵代数

#### 矩阵Matrix

- 用于描述几何体的变换，例如缩放、旋转、平移

- 借助矩阵可以将点或向量的坐标在不同的坐标系之间进行转换

- 行数和列数的乘积表示矩阵的维度

- 矩阵相等、加法运算、标量乘法、减法运算

  - 两个矩阵相等，行数列数必须相同

  - 只有行数列数相同的两个矩阵相加才有意义

  - 标量乘法将一个标量依次与矩阵内的每个元素相乘

  - 利用矩阵的加法和标量乘法可以定义矩阵减法

    A - B = A + (-1 * B) = A + (-B)

#### 矩阵乘法

- 横*竖

- A * B   A的横向元素数与B的竖向元素数相等才有意义

- 向量与矩阵的乘法是一种线性组合

#### 转置矩阵

- 原矩阵的行与列进行交换所得到的新矩阵
- A + B的转置矩阵 = A的转置矩阵 + B的转置矩阵
- n * A的转置矩阵 = n * A的转置矩阵
- A * B的转置矩阵 = B的转置矩阵 * A的转置矩阵
- A的转置矩阵的转置矩阵 = A

#### 单位矩阵

- 主对角线元素均为1，其它元素都为0的方阵
- 单位矩阵是矩阵的乘法单位元
- A * 单位矩阵 = A

#### 矩阵行列式

- 余子阵 一个n * n的矩阵去掉矩阵的第i行 第j列
- 用来手推逆矩阵的

#### 逆矩阵

- 矩阵不存在除法概念，但定义了矩阵乘法的逆运算
- 只有方阵才具有逆矩阵，但不是每个方阵都有逆矩阵
- 存在逆矩阵的方阵为可逆矩阵，不存在逆矩阵的方阵为奇异矩阵
- 可逆矩阵的逆矩阵是唯一的
- 矩阵 * 矩阵的逆矩阵 = 矩阵的单位矩阵
- B的逆矩阵 * A的逆矩阵 = A * B 的逆矩阵

#### DirectXMath API 矩阵相关

- 矩阵类型 XMMATRIX

- XMMATRIX底层由4个XMVECTOR实例构成，并借此来使用SIMD技术

- 建议用XMFLOAT4X4来存储来存储类中的矩阵类型数据成员

- XMMATRIX 与 XMFLOAT4X4内存排列一致，对齐不一致，是可以进行内存拷贝的

- 常用API

  - 返回单位矩阵

    `XMMATRIX XM_CALLCONV XMMatrixIdentity();`

  - 返回是否为单位矩阵

    `bool XM_CALLCONV XMMatrixIsIdentity(FXMMATRIX M);`

  - 返回矩阵的转置矩阵

    `XMMATRIX XM_CALLCONV XMMatrixTranspose(FXMMATRIX M);`

  - 返回矩阵的行列式

    `XMVECTOR XM_CALLCONV XMMatrixDeterminant(FXMMATRIX M);`

  - 输入矩阵的行列式以及矩阵本身，返回矩阵的逆矩阵

    `XMMATRIX XM_CALLCONV XMMatrixInverse(XMVECTOR* pDeterminant, FXMMATRIX  M)`

- 参数传递规则与XMVECTOR一致

### 变换

#### 线性变换

- 缩放矩阵

  ```
  x,0,0,0
  0,y,0,0
  0,0,z,0
  0,0,0,1
  ```

- 旋转矩阵

  ```c++
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

- 正交矩阵

  矩阵的行向量都是规范正交，则该矩阵为正交矩阵，正交矩阵的逆矩阵与转置矩阵是相等的，所有的旋转矩阵都是正交矩阵

#### 仿射变换

- 由线性变换与平移变换组合而成

- 向量的性质中没有位置这个概念

- (x,y,z,0)表示向量    (x,y,z,1)表示点

- 两点之差得的是一个向量，点与向量之和得到的是一个点

- 平移矩阵

  ```
  1,0,0,0
  0,1,0,0
  0,0,1,0
  x,y,z,1
  ```

#### 变换的复合

矩阵之间的乘法运算不满足交换律，多种变换注意连乘顺序，顺序不同得到的结果是不同的

#### 坐标系变换

- 不同坐标系之间的坐标转换，在坐标系变换过程中几何体本身不会发生改变，改变的仅仅是参考坐标系
- 通过逆矩阵可进行世界坐标与本地坐标的相互转换

#### DirectXMath API  变换相关

- 常用API

  - 构建一个缩放矩阵

    `XMMATRIX XM_CALLCONV XMMatrixScaling(float ScaleX, float ScaleY, float ScaleZ);`

  - 用一个3D向量来构建缩放矩阵

    `XMMATRIX XM_CALLCONV XMMatrixScalingFromVector(FXMVECTOR Scale);`

  - 构建一个绕X轴旋转多少弧度的矩阵(YZ轴类似)

    `XMMATRIX XM_CALLCONV XMMatrixRotationX(float Angle);`

  - 构建平移矩阵

    `XMMATRIX XM_CALLCONV XMMatrixTranslation(float OffsetX, float OffsetY, float OffsetZ);`

- 可以直接用封装好的库

##  Direct3D 基础

### Direct3D的初始化

#### 预备知识

- Direct3D 12 基本概念

  底层图形应用程序编程接口，可在应用程序中对图形处理器进行控制和编程，借此以硬件加速的方式渲染虚拟的3D场景

- 组件对象模型(COM)

  - 使DirectX不受编程语言束缚，并且使之向后兼容的技术

  - 通常将COM对象是为一种接口，可以把它当作一个C++类来使用

  - 要获取指向某COM接口的指针，需要借助特定函数或另一COM接口的方法，而不是用C++的new去创建一个COM接口

  - COM采用引用计数方式管理内存，在使用完某个接口后，要调用Release方法，而不是delete来删除，当COM对象的引用计数为0时，会自动释放自己所占用的内存

  - COM接口的所有功能都是从`IUnknown`这个COM接口继承而来，包括`Release`方法

  - 为了方便用户管理COM对象的生命周期，Windows运行时库提供了`#include <wrl.h>`  `ComPtr`类，可以当作COM对象的智能指针，超出作用域会自动调用Release

  - 常用API

    `Get`：返回指向该底层COM接口的指针

    `GetAddressOf`：返回执行该底层COM接口的指针的地址，也就是&(COM接口指针)

    `Reset`：将该ComPtr实例设置为nullptr，释放相关的所有引用(同时减少其底层COM的引用计数)，和ComPtr目标实例赋值为nullptr的效果相同

- 纹理格式(texture)

  - 2Dtexture由数据元素构成的矩阵，可以看作2D数组，用来存储2D图像数据，纹理中每个元素存储的都是一个像素(*pixel 构成图像的基本元素，图像的采样单位，在Direct3D中，像素被抽象为具有一定长宽的色块*)的颜色，但纹理并不只是用来存储颜色信息的。例如，法线贴图(normal mapping) 纹理内的每个元素存储的是一个3D向量而不是颜色信息
  - 简单理解，纹理相当于特定数据元素所构成的数组，但不只是数据数组这么简单，可能还具有多种mipmap层级，GPU会据此对它们进行特殊处理，例如，运用过滤器(filter)和进行多重采样(multisample)
  - 并不是任意类型的数据元素都能用于组成纹理，只能存储`DXGI_FORMAT`枚举类型中描述的特定格式的数据元素
  - 无类型(typeless)格式纹理仅用来预留内存，在纹理被绑定到渲染流水线后，再具体给他相应的数据类型，类似C++的强转
  - `DXGI_FORMAT`枚举类型 也可用于描述顶点以及索引的数据格式

- 交换链和页面翻转（屏幕二次缓冲/双缓冲/double buffering）

  - 避免动画出现画面闪烁，将动画帧完整绘制在后台缓冲区的离屏(off-screen)纹理内，将指定动画帧的整个场景绘制到后台缓冲区，就会以一个完整的帧画面显示在屏幕上，便不会看出帧绘制的过程

  - 为此需要利用硬件管理的两种纹理缓冲区：

    - 前台缓冲区(front buffer)

      存储当前显示在屏幕上的图像数据  

    - 后台缓冲区(back buffer)

      动画的下一帧被毁旨在后台缓冲区

  - 呈现(present)

    前后台缓冲互换属于高效操作，只需要交换前后台缓冲区指针即可

    当后台缓冲区的动画帧绘制完成后，后台缓冲区变为前台缓冲区显示新一帧的画面，之前的前台缓冲区转为后台缓冲区为下一帧的展示填充数据

  - 交换链(swap chain)

    前台缓冲区和后台缓冲区构成了交换链，在Direct3D中用`IDXGISwapChain`接口来表示，该接口提供了存储前后台缓冲区的两种纹理，修改缓冲区大小，呈现缓冲区内容等方法，用的时候查API文档

- 深度缓冲(depth buffer)

  - 为了确定不同物体间的像素前后顺序，Direct3D采用深度缓冲(depth buffering)或z缓冲(z-buffering，z指坐标)技术
  - 若使用了深度缓冲，则不需要在乎物体的绘制顺序

  - 深度缓冲区也是一种纹理，这种纹理资源存储的是特定像素的深度信息，并非图像数据，深度值范围为0.0-1.0，0.0代表观察者在视锥体中能离自己最近的物体，1.0代表观察者在视锥体中能看到离自己最远的物体
  - 深度缓冲区元素与后台缓冲区的像素呈对应关系
  - 深度缓冲原理
    - 计算每个像素的深度值，并执行深度测试
  - 深度缓冲工作步骤
    - 后台缓冲区清理为默认颜色(0,0,0)，深度缓冲区清理为默认值(通常为1.0)
    - 物体深度测试，深度小于默认深度，更新缓冲区以及默认深度，深度大于默认深度，不更新
  - 需要用明确的数据格式来创建深度缓冲
    - `DXGI_FORMAT_D32_FLOAT_S8X24_UINT`
    - `DXGI_FORMAT_D32_FLOAT`
    - `DXGI_FORMAT_D24_UNORM_S8_UINT`
    - `DXGI_FORMAT_D16_UNORM`
  - *模板缓冲区待学习*

- 资源与描述符(视图)

- 多重采样

- 功能级别

- DirectX图形基础结构

- 功能支持的检测

- 资源驻留

#### CPU与GPU之间的交互

#### 初始化Direct3D

#### 计时与动画

#### 应用程序框架示例

#### 调试

### 渲染流水线

### Direct3D绘制几何体

### 光照

### 纹理贴图

### 混合

### 计算着色器



