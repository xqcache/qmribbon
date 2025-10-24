#pragma once

#if defined(_WIN32) || defined(_WIN64)

#    if defined(QMRIBBON_STATIC)
#        define QMRIBBON_API
#    elif defined(QMRIBBON_BUILDING_LIB)
#        define QMRIBBON_API __declspec(dllexport)
#    else
#        define QMRIBBON_API __declspec(dllimport)
#    endif

#else
#    define QMRIBBON_API
#endif
