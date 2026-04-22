# RemapDrv

[English](README.md) | 中文文档

> 面向 Windows x64 的驱动通信研究项目。它通过重映射驱动镜像并接管目标系统调用回调指针，在 R3 与 R0 之间建立一条相对隐蔽的通信路径。

## 1. 项目名称

**RemapDrv**

## 2. 项目简介

`RemapDrv` 演示了一种基于系统调用回调劫持的驱动通信思路。仓库同时包含内核态驱动样例与用户态控制台样例，覆盖以下核心部分：

- 驱动自重映射与重定位，此时常规手扫和部分 ARK 工具可能无法直接定位驱动模块信息
- 回调安装与基础通信分发测试
- R3 侧通信包封装与控制码调用

下图展示了本项目使用的通信 Hook 点：

![HookComm](Image/HookComm.png)

## 3. 功能与使用示例

### 驱动入口

驱动入口会先将自身镜像重映射到新的内存区域，再计算重映射后的回调地址并安装通信回调。

对应源码：`RemapDrv/Entry/drv_main.cpp`

```cpp
EXTERN_C
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DrvObj,
    PUNICODE_STRING RegPath
    )
{
    UNREFERENCED_PARAMETER(RegPath);

    PUCHAR RemapBase = (PUCHAR)Remap::RemapSelf(DrvObj);
    PUCHAR RemapCallbackBase = RemapBase + ((PUCHAR)HookCallback - (PUCHAR)DrvObj->DriverStart);

    InstallComm((fn_CommCallback)RemapCallbackBase);
    return STATUS_UNSUCCESSFUL;
}
```

### 通信回调

回调函数会校验通信包是否合法，识别自定义 `Magic` 与 `CtlCode`，非目标调用则继续转发给原始回调。

对应源码：`RemapDrv/Entry/drv_main.cpp`

```cpp
__int64
HookCallback(
    IN __int64 a1,
    IN __int64 a2,
    IN __int64 a3,
    IN __int64 a4
    )
{
    PDRV_COMM_PACKAGE CommPackage = (PDRV_COMM_PACKAGE)a1;
    if (a1 == 0 || a2 != sizeof(DRV_COMM_PACKAGE) || CommPackage->Magic != DRV_COMM_MAGIC)
    {
        return g_OldCommCallback(a1, a2, a3, a4);
    }

    CommPackage->RetValue = 0x12345678;
    return 0;
}
```

### R3 调用示例

用户态控制台程序通过 `DrvClient::SendCtl(...)` 打包并发送控制请求：

对应源码：`RemapClient/exe_main.cpp`

```cpp
std::array<UCHAR, 0x100> Buffer = {};
ULONG RetValue = 0;

const NTSTATUS Status = Client.SendCtl(
    0x1000,
    Buffer.data(),
    static_cast<ULONG>(Buffer.size()),
    &RetValue
);
```

### 通信包格式

R3 与 R0 之间交换的数据结构定义如下：

对应源码：`RemapDrv/Comm/DrvCommDef.h`

```cpp
typedef struct _DRV_COMM_PACKAGE
{
    ULONG64 Magic;
    ULONG CtlCode;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG RetValue;
} DRV_COMM_PACKAGE, *PDRV_COMM_PACKAGE;
```

## 4. 支持环境

### 已测试系统版本

- Windows 10 19044
- Windows 11 22H2
- Windows 11 24H2
- Windows 11 25H2

### 通讯测试截图

Windows 10 19044：

![CommTestWin10](Image/CommTest_19044.png)

Windows 11 25H2：

![CommTestWin11](Image/CommTest_25H2.png)

### 预期兼容范围

当前只在上述虚拟机环境中完成验证。结合代码中的版本分支与特征码匹配逻辑，理论上可预期支持：

- Windows 10 19041 ~ Windows 11 25H2

实际兼容性仍建议以目标系统实测结果为准。

## 5. 构建

### 构建环境

- Visual Studio 2017
- WDK 10
- 仅支持 `x64`

### 构建步骤

1. 使用 Visual Studio 2017 打开 `RemapDrv.sln`
2. 选择目标平台为 `x64`
3. 编译 `RemapDrv` 驱动项目
4. 编译 `RemapClient` 控制台项目

## 6. 仓库结构

```text
.
+-- Image/
|   +-- HookComm.png
|   +-- CommTest_19044.png
|   +-- CommTest_25H2.png
+-- RemapClient/
|   +-- exe_main.cpp              Invocation entry example
|   +-- DrvClient.cpp/.h          User-mode communication wrapper
|   +-- DrvComm.cpp/.h            Low-level sending logic
+-- RemapDrv/
|   +-- Entry/                    DriverEntry and HookCallback
|   +-- Comm/                     Communication installation and packet definitions
|   +-- Remap/                    Driver self-remapping logic
|   +-- Support/PatternScan/      Module lookup and signature scanning helpers
+-- RemapDrv.sln
```
