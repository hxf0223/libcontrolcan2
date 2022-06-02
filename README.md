## README

### 开发环境
- VS2022扩展：Clang Power Tools ；
- 依赖于boost，需要定义系统环境变量 BOOST_ROOT；
- 使用gtest源码 [v1.11.0](https://github.com/google/googletest/releases/tag/release-1.11.0)；
- 使用glog源码 [v0.6.0](https://github.com/google/glog/releases/tag/v0.6.0)。

gtest更新的版本需要 Abseil编译，可能会比较麻烦。
gtest源码目录相对于官方gtest仓库修改：
- googletest/CMakeLists.txt：set(GOOGLETEST_VERSION 1.11.0)，否则找不到GOOGLETEST_VERSION会报错；
- googletest/cmake/internal_utils.cmake：函数cxx_library_with_type中更改RUNTIME_OUTPUT_DIRECTORY等变量的路径为CMAKE_ARCHIVE_OUTPUT_DIRECTORY，使输出路径与父CMakeLists.txt保持一致。
- googletest/CMakeLists.txt：添加 set(gtest ${gtest} PARENT_SCOPE)，让test目录能够依赖于gtest之后编译；
- googletest/CMakeLists.txt：gtest，gtest_main 静态编译修改为 cxx_shared_library 动态编译。

glog源码相对于官方glog的修改：
- glog/CMakeLists.txt： set(BUILD_TESTING OFF)，禁用编译glog的测试用例；
- glog/CMakeLists.txt：set(glog ${glog} PARENT_SCOPE)；
- 添加 glog/include/glog目录，将install得到的头文件拷贝到此目录下。

### 官方glog使用vs2022编译命令
```bash
cmake .. -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE="Release" -DBUILD_SHARED_LIBS="ON" -DCMAKE_INSTALL_BINDIR="bin" -DCMAKE_INSTALL_SBINDIR="bin" -DCMAKE_INSTALL_LIBEXECDIR="bin" -DCMAKE_INSTALL_LIBDIR="lib" -DCMAKE_INSTALL_INCLUDEDIR="include" -DCMAKE_INSTALL_DATAROOTDIR="share" -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY="ON" -DWITH_THREADS="True" -DWITH_SYMBOLIZE="True" -DWITH_UNWIND="True" -DBUILD_TESTING="False"
```

### 参考
- [CMake设置MSVC工程MT/MTd/MD/MDd](https://blog.csdn.net/Copperxcx/article/details/123084367)
- [USBCAN资料](https://www.zlg.cn/can/down/down/id/22.html)
- [windows临界区与std::lock_guargd性能对比](https://gitee.com/vaughnHuang/cs_lock_perf_test)
