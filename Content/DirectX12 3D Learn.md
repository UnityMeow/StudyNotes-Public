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

 