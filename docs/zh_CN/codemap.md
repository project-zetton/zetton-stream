---
title: 代码地图
---

- [代码地图](#代码地图)
  - [目录结构](#目录结构)
  - [模块介绍](#模块介绍)

# 代码地图

本文档旨在帮助您了解 zetton-stream 代码的结构和功能。

## 目录结构

zetton-stream 代码的目录结构如下：

```bash
$ tree -L 3

.
├── cmake/
├── CMakeLists.txt
├── docs/
├── examples/
├── include/
├── LICENSE
├── package.xml
├── README.md
├── src/
└── tools/
```

其中：

- `.github/`：GitHub 相关的配置文件
- `cmake/` 与 `CMakeLists.txt`：CMake 构建相关的文件
- `docs/`：文档目录
- `examples/`：示例代码目录
- `include/`：头文件目录
- `src/`：源代码目录
- `tools/`：工具脚本目录
- `LICENSE`：软件包许可证
- `README.md`：软件包说明文档
- `README_zh-CN.md`：软件包说明文档（中文）
- `package.xml`：软件包描述文件，用于 colcon 构建

## 模块介绍

zetton-stream 代码包含如下模块：

- `base`：基础模块，包含一些基础的数据结构
- `sink`：数据输出模块，包含媒体流的输出接口
- `source`：数据输入模块，包含媒体流的输入接口
- `util`：工具模块，包含一些常用的工具函数
