#pragma once

#include <QtCore/qglobal.h>

/// 动态库导出宏（写法与 thirdparty 里 QWindowKit 的 `QWK_*_EXPORT` 一致）。
///
/// 三种状态：
///
///   `QMRIBBON_STATIC`   静态库：本目标与使用方都要定义。由 CMake 以 PUBLIC 方式挂在
///                       qmribbon 目标上，链接该目标就会自动带上；**脱离 CMake 手工链接
///                       静态库时必须自己定义**，否则下面会展开成 `Q_DECL_IMPORT`，
///                       使用方链接必然失败。
///   `QMRIBBON_LIBRARY`  正在编译动态库本身，只有 qmribbon 目标自己定义（PRIVATE）。
///   两者都没有          使用动态库：类名前面的宏展开成 `Q_DECL_IMPORT`。
///
/// 整个导出策略是「显式标注」而不是 `WINDOWS_EXPORT_ALL_SYMBOLS`：Qt 的 Q_OBJECT 类需要
/// 连同 staticMetaObject / vtable 一起导出，让编译器去猜不如写清楚。
/// 也没有打开 `CXX_VISIBILITY_PRESET hidden`：Windows 上出口由 `Q_DECL_EXPORT` 决定，
/// Linux/macOS 保持默认可见性本来就能跑；额外打开隐藏可见性只会让漏标注在 Linux 上也变成
/// 链接错误，收益不值这个风险。
///
/// **新增公共类时，别忘了在类名前面加上这个宏。** 漏了的话静态库构建看不出任何问题，
/// 动态库构建下使用方会报 LNK2019（找不到该类的构造函数 / metaobject）。
/// `tests/consumer` 那个独立工程会触达所有公共类，就是用来兜住这个疏漏的。
#ifndef QMRIBBON_EXPORT
#  ifdef QMRIBBON_STATIC
#    define QMRIBBON_EXPORT
#  else
#    ifdef QMRIBBON_LIBRARY
#      define QMRIBBON_EXPORT Q_DECL_EXPORT
#    else
#      define QMRIBBON_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif
