cmake_minimum_required(VERSION 3.16)

# プロジェクト名の設定
get_filename_component(PROJECT_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_ID})
project(${PROJECT_ID})

# In-source ビルドの抑止
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR 
        "In-source builds are not allowed. "
        "Run 'cmake -B build' instead.")
endif()

# 32 bit環境でビルドしようとしている場合に警告
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING
        "********* 32 bit build warning *********\n"
        "It seems you are trying to build on a 32 bit platform.")
endif()

# ビルドオプションの設定
option(YOURLIB_BUILD_SHARED "Build yourlib as a shared library" ON)

# ビルドモードの設定
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 実行ファイルと動的ライブラリを置く場所を決める
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "" FORCE)
endif()

# yourlib/src/CMakeLists.txtを呼び出す
add_subdirectory(src)
