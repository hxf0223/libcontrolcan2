## README

### ��������
- VS2022��չ��Clang Power Tools ��
- ������boost����Ҫ����ϵͳ�������� BOOST_ROOT��
- ʹ��gtestԴ�� [v1.11.0](https://github.com/google/googletest/releases/tag/release-1.11.0)��
- ʹ��glogԴ�� [v0.6.0](https://github.com/google/glog/releases/tag/v0.6.0)��

gtest���µİ汾��Ҫ Abseil���룬���ܻ�Ƚ��鷳��
gtestԴ��Ŀ¼����ڹٷ�gtest�ֿ��޸ģ�
- googletest/CMakeLists.txt��set(GOOGLETEST_VERSION 1.11.0)�������Ҳ���GOOGLETEST_VERSION�ᱨ��
- googletest/cmake/internal_utils.cmake������cxx_library_with_type�и���RUNTIME_OUTPUT_DIRECTORY�ȱ�����·��ΪCMAKE_ARCHIVE_OUTPUT_DIRECTORY��ʹ���·���븸CMakeLists.txt����һ�¡�
- googletest/CMakeLists.txt����� set(gtest ${gtest} PARENT_SCOPE)����testĿ¼�ܹ�������gtest֮����룻
- googletest/CMakeLists.txt��gtest��gtest_main ��̬�����޸�Ϊ cxx_shared_library ��̬���롣

glogԴ������ڹٷ�glog���޸ģ�
- glog/CMakeLists.txt�� set(BUILD_TESTING OFF)�����ñ���glog�Ĳ���������
- glog/CMakeLists.txt��set(glog ${glog} PARENT_SCOPE)��
- ��� glog/include/glogĿ¼����install�õ���ͷ�ļ���������Ŀ¼�¡�

### �ٷ�glogʹ��vs2022��������
```bash
cmake .. -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE="Release" -DBUILD_SHARED_LIBS="ON" -DCMAKE_INSTALL_BINDIR="bin" -DCMAKE_INSTALL_SBINDIR="bin" -DCMAKE_INSTALL_LIBEXECDIR="bin" -DCMAKE_INSTALL_LIBDIR="lib" -DCMAKE_INSTALL_INCLUDEDIR="include" -DCMAKE_INSTALL_DATAROOTDIR="share" -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY="ON" -DWITH_THREADS="True" -DWITH_SYMBOLIZE="True" -DWITH_UNWIND="True" -DBUILD_TESTING="False"
```

### �ο�
- [CMake����MSVC����MT/MTd/MD/MDd](https://blog.csdn.net/Copperxcx/article/details/123084367)
- [USBCAN����](https://www.zlg.cn/can/down/down/id/22.html)
- [windows�ٽ�����std::lock_guargd���ܶԱ�](https://gitee.com/vaughnHuang/cs_lock_perf_test)
