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

# ビルドモードの設定
set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# インストール先の設定
if(NOT YOUREXEC_INSTALL_DIR)
    include(GNUInstallDirs)
    set(YOUREXEC_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/${CMAKE_SYSTEM_NAME})
endif()

# 依存ライブラリのインストール先を指定
set(YOURLIB_INSTALL_DIR ${YOUREXEC_INSTALL_DIR})

# 依存関係の読み込み
include(FetchContent)
set(DEPENDENCY_NAME dependency)
FetchContent_Declare(${DEPENDENCY_NAME}
    GIT_REPOSITORY ${PROJECT_SOURCE_DIR}
    GIT_TAG tutorial-library)
FetchContent_MakeAvailable(${DEPENDENCY_NAME})

# ビルドターゲットの設定
set(TARGET_NAME test)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE main.cpp)

# 依存関係のリンク
target_link_libraries(${TARGET_NAME} PRIVATE yourlib)

# 実行ファイルと動的ライブラリを置く場所を決める
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "" FORCE)
endif()

# インストールコマンドで実行ファイルを配置するようにする
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${YOUREXEC_INSTALL_DIR})

# 依存関係が動的ライブラリである場合（ライブラリのビルドオプションを参照）
if(YOURLIB_BUILD_SHARED)
    # 動的ライブラリを読めるように，ランタイムパスを設定する
    set_target_properties(${TARGET_NAME} PROPERTIES
        INSTALL_RPATH $<IF:$<PLATFORM_ID:Linux>,$ORIGIN,@executable_path>)
endif()

# Windows特有の設定
# Visual Studioのデバッガ起動時の作業ディレクトリを設定する
set_target_properties(${TARGET_NAME}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    $<TARGET_FILE_DIR:${TARGET_NAME}>)
# Visual StudioでソースコードをUTF-8で読ませる
target_compile_options(${TARGET_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
# セキュリティ強化版関数を使えというおせっかい警告を黙らせる
target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
