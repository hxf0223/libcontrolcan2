## README

### 开发环境

- VS2022扩展：Clang Power Tools ；
- VS2022扩展：Format Document On Save ；
- 依赖于boost，需要定义系统环境变量 BOOST_ROOT ；
- 使用gtest源码 [v1.11.0](https://github.com/google/googletest/releases/tag/release-1.11.0)；
- 使用glog源码 [v0.6.0](https://github.com/google/glog/releases/tag/v0.6.0)。

gtest v1.11.0 之后的版本需要 Abseil 编译，可能会比较麻烦。
gtest源码目录相对于官方gtest仓库修改：

- googletest/CMakeLists.txt：set(GOOGLETEST_VERSION 1.11.0)，否则找不到GOOGLETEST_VERSION会报错；
- googletest/cmake/internal_utils.cmake：函数cxx_library_with_type中更改RUNTIME_OUTPUT_DIRECTORY等变量的路径为CMAKE_ARCHIVE_OUTPUT_DIRECTORY，使输出路径与父CMakeLists.txt保持一致。
- googletest/CMakeLists.txt：添加 set(gtest ${gtest} PARENT_SCOPE)，让test目录能够依赖于gtest之后编译；
- googletest/CMakeLists.txt：gtest，gtest_main 静态编译修改为 cxx_shared_library 动态编译。

glog源码相对于官方glog的修改：

- glog/CMakeLists.txt： set(BUILD_TESTING OFF)，禁用编译glog的测试用例；
- glog/CMakeLists.txt：set(glog ${glog} PARENT_SCOPE)；
- 添加 glog/include/glog目录，将install得到的头文件拷贝到此目录下。

boost编译适用于libControlCAN.so的静态库：[Linux Windows Boost编译命令](https://www.cnblogs.com/vaughnhuang/p/15848139.html)

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

### 官方glog使用vs2022编译命令

```bash
cmake .. -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE="Release" -DBUILD_SHARED_LIBS="ON" -DCMAKE_INSTALL_BINDIR="bin" -DCMAKE_INSTALL_SBINDIR="bin" -DCMAKE_INSTALL_LIBEXECDIR="bin" -DCMAKE_INSTALL_LIBDIR="lib" -DCMAKE_INSTALL_INCLUDEDIR="include" -DCMAKE_INSTALL_DATAROOTDIR="share" -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY="ON" -DWITH_THREADS="True" -DWITH_SYMBOLIZE="True" -DWITH_UNWIND="True" -DBUILD_TESTING="False"
```

### 参考

- [CMake设置MSVC工程MT/MTd/MD/MDd](https://blog.csdn.net/Copperxcx/article/details/123084367)
- [周立功 USBCAN 资料](https://www.zlg.cn/can/down/down/id/22.html)
- [周立功 USBCAN Linux资料](https://www.zlg.cn/Index/Search/search?key=linux)
- [windows临界区与std::lock_guargd性能对比](https://gitee.com/vaughnHuang/cs_lock_perf_test)
- [concurrentqueue](https://github.com/cameron314/concurrentqueue)
- [Boost ASIO例程](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/examples/cpp11_examples.html)

### 引用第三方库

- [eventpp](https://github.com/wqking/eventpp)

### template相关参考

- [泛化之美--C++11可变模版参数的妙用](https://www.cnblogs.com/qicosmos/p/4325949.html)
- [Variadic templates (C++11)](https://www.ibm.com/docs/en/zos/2.3.0?topic=only-variadic-templates-c11)
- [std true_type false_type的使用](https://stackoverflow.com/questions/20368187/when-would-i-use-stdintegral-constant-over-constexpr)

### template std::true_type std::false_type

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
