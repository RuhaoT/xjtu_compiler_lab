# 西安交通大学 编译实验 2025春

因时间限制未能测试所有代码，可能存在隐藏错误。

## 复现

确保以下包对CMake可见：

- `gtest`
- `yaml-cpp`
- `spdlog`
- `tabulate`
- `boost graph`

创建`build`目录并进入，执行CMake构建语句：

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

在`build`目录运行所有单元测试：

```bash
./test_all
```