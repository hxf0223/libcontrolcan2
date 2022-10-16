# README

## 开发环境

- VS2022扩展：Clang Power Tools ；
- VS2022扩展：Format Document On Save ；
- 依赖于boost，需要定义系统环境变量 BOOST_ROOT ；
- 使用gtest源码 [v1.11.0](https://github.com/google/googletest/releases/tag/release-1.11.0)；
- 使用glog源码 [v0.6.0](https://github.com/google/glog/releases/tag/v0.6.0)。

## 自带第三方库的编译

1. gtest源码目录相对于官方gtest仓库修改

- googletest/CMakeLists.txt：set(GOOGLETEST_VERSION 1.11.0)，否则找不到GOOGLETEST_VERSION会报错；
- googletest/cmake/internal_utils.cmake：函数cxx_library_with_type中更改 RUNTIME_OUTPUT_DIRECTORY 等变量的路径为 CMAKE_ARCHIVE_OUTPUT_DIRECTORY ，使输出路径与父CMakeLists.txt保持一致。
- googletest/CMakeLists.txt：添加 set(gtest ${gtest} PARENT_SCOPE)，让test目录能够依赖于gtest之后编译；
- googletest/CMakeLists.txt：gtest，gtest_main 静态编译修改为 cxx_shared_library 动态编译。

2. glog源码相对于官方glog的修改

- glog/CMakeLists.txt： set(BUILD_TESTING OFF)，禁用编译glog的测试用例；
- glog/CMakeLists.txt：set(glog ${glog} PARENT_SCOPE)；
- 在 configure_file (src/glog/logging.h.in glog/logging.h @ONLY) 等后面继续添加 configure_file (src/glog/log_severity.h glog/log_severity.h @ONLY) ；
- 以及添加 configure_file (src/glog/platform.h glog/platform.h @ONLY) ；
- 在 set(GLOG_PUBLIC_H ...) 语句后面，添加 set\(GLOG_INC_DIR "\$\{CMAKE_CURRENT_BINARY_DIR\}/glog" PARENT_SCOPE\) 。在根 CMakeLists.txt 中添加 include_directories\(\$\{GLOG_INC_DIR\}\) ；
- 注释掉 set(CMAKE_DEBUG_POSTFIX d) ；

3. 引入libzmq，辅助测试代码引用libzmp

- 注释掉 RELEASE_POSTFIX ， RELWITHDEBINFO_POSTFIX ， MINSIZEREL_POSTFIX ， DEBUG_POSTFIX ，并且测试例程针对Windows 链接名称为 libzmq；
- BUILD_TESTS 设置为 OFF;
- WITH_LIBSODIUM 设置为 OFF;
- WITH_LIBBSD 设置为 OFF;
- BUILD_STATIC 设置为 OFF，否则Windows编译报错（与BUILD_SHARED二选一）；
- set(ZeroMQ ${ZeroMQ} PARENT_SCOPE);
- WITH_DOCS 位置为 OFF;

4. boost编译适用于libControlCAN.so的静态库：[Linux Windows Boost编译命令](https://www.cnblogs.com/vaughnhuang/p/15848139.html)

```bash
./boostrap.sh
./b2 -layout=versioned variant=release cxxflags='-fPIC -std=c++17' runtime-link=static link=static threading=multi
sudo ./b2 install
```

```bash
# 删除系统安装的boost
sudo apt-get --purge remove libboost-dev
sudo apt-get --purge remove libboost-all-dev
```

## 官方glog使用vs2022编译命令

```bash
cmake .. -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE="Release" -DBUILD_SHARED_LIBS="ON" -DCMAKE_INSTALL_BINDIR="bin" -DCMAKE_INSTALL_SBINDIR="bin" -DCMAKE_INSTALL_LIBEXECDIR="bin" -DCMAKE_INSTALL_LIBDIR="lib" -DCMAKE_INSTALL_INCLUDEDIR="include" -DCMAKE_INSTALL_DATAROOTDIR="share" -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY="ON" -DWITH_THREADS="True" -DWITH_SYMBOLIZE="True" -DWITH_UNWIND="True" -DBUILD_TESTING="False"
```

## Windows 仅使用命令行编译

```bash
cmake -G "Visual Studio 17 2022" -A Win32 -S ./ -B "build32"
# cmake --build build32 --config Release --clean-first -j
cmake --build build32 --config Release -j
```

[CMake Build a Project](https://cmake.org/cmake/help/latest/manual/cmake.1.html#build-tool-mode)

## 编译问题解决

1. 编译libControlCAN时，出现 **relocation against in read-only section**。是由于系统安装了多个版本的boost导致，添加 **link_directories("/usr/local/lib")**覆盖系统boost安装位置。
2. Windows编译时，导出__stdcall函数时，导出符号带有_Func_Name@num，需要将def文件带入源文件一起编译；

## 编译资源文件参考
- [TVersionFixedInfo](https://www.freepascal.org/docs-html/current/fclres/versiontypes/tversionfixedinfo.filetype.html)
- [Go利用windres.exe和.rc文件在Windows下生成的程序带有版本、版权、产品名称、图标等属性信息](https://blog.csdn.net/FlushHip/article/details/84978556)
- [llvm windows_version_resource.rc](https://github.com/llvm-mirror/llvm/blob/master/resources/windows_version_resource.rc)

## 代码clang-format全部格式化
```bash
find . -regex '.*\.\(cc\|cpp\|hpp\|cu\|c\|h\)' -exec clang-format -style=file -i {} \;
```
代码检查：[Extra Clang Tools 16.0.0git documentation](https://clang.llvm.org/extra/clang-tidy/checks/readability/identifier-naming.html)

## 参考

- [CMake设置MSVC工程MT/MTd/MD/MDd](https://blog.csdn.net/Copperxcx/article/details/123084367)
- [周立功 USBCAN 资料](https://www.zlg.cn/can/down/down/id/22.html)
- [周立功 USBCAN Linux资料](https://www.zlg.cn/Index/Search/search?key=linux)
- [windows临界区与std::lock_guargd性能对比](https://gitee.com/vaughnHuang/cs_lock_perf_test)
- [concurrentqueue](https://github.com/cameron314/concurrentqueue)
- [Boost ASIO例程](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/examples/cpp11_examples.html)
- [C++ Client Server with Boost.Asio](https://github.com/Lanskask/boost_asio_client_server)
- [Boost ASIO 教程](https://dens.website/tutorials/cpp-asio)

## 引用第三方库

- [eventpp](https://github.com/wqking/eventpp)
- [libzmq](https://github.com/zeromq/libzmq)
- [cppzmq](https://github.com/zeromq/cppzmq)
- [format](https://github.com/arajar/format)
- [boost_asio_zeromq](https://github.com/iyedb/boost_asio_zeromq)(<https://iyedb.github.io/cpp11/en/2014/07/11/asio-zeromq-cpp11.html>)
- [cmake git version](https://github.com/andrew-hardin/cmake-git-version-tracking)
- [ticks](https://en.wikipedia.org/wiki/Time_Stamp_Counter)

## template相关参考

- [泛化之美--C++11可变模版参数的妙用](https://www.cnblogs.com/qicosmos/p/4325949.html)
- [Variadic templates (C++11)](https://www.ibm.com/docs/en/zos/2.3.0?topic=only-variadic-templates-c11)
- [std true_type false_type的使用](https://stackoverflow.com/questions/20368187/when-would-i-use-stdintegral-constant-over-constexpr)

## template std::true_type std::false_type

```C++
#include <type_traits>
#include <iostream>
/* https://ideone.com/469YTq */

template<typename T>
void use_impl(const T&, std::false_type) {
 std::cout << "use_impl(false)" << std::endl;
}

template<typename T>
void use_impl(const T&, std::true_type) {
 std::cout << "use_impl(true)" << std::endl;
}

template<typename T>
void use(const T& v) {
   use_impl(v, typename std::is_integral<T>::type());
}

int main() {
   use(1);
   use(1.2);
}

```
