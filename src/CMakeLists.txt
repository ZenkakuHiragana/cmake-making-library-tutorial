# 作成するライブラリを宣言
set(TARGET_NAME yourlib)
if(YOURLIB_BUILD_SHARED)
    add_library(${TARGET_NAME} SHARED)
else()
    add_library(${TARGET_NAME} STATIC)
endif()

# ソースファイルの登録
target_sources(${TARGET_NAME} PRIVATE source1.c source2.c cpp.cpp)

# インクルード検索パスの設定
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 公開する関数の設定
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} MSVC       IS_MSVC)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} Clang      IS_CLANG)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} AppleClang IS_ACLANG)
string(COMPARE EQUAL ${CMAKE_C_COMPILER_ID} GNU        IS_GNU)
if(IS_MSVC OR (WIN32 AND IS_CLANG))
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.def)
    set(EXPORT_FLAG ${CMAKE_LINK_DEF_FILE_FLAG})
elseif(IS_ACLANG OR (APPLE AND IS_CLANG))
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.exp)
    set(EXPORT_FLAG -Wl,-exported_symbols_list,)
elseif(IS_GNU OR IS_CLANG)
    set(EXPORT_LIST ${PROJECT_SOURCE_DIR}/cmake/${TARGET_NAME}.map)
    set(EXPORT_FLAG -Wl,--version-script=)
endif()
set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS ${EXPORT_FLAG}"${EXPORT_LIST}"
    LINK_DEPENDS "${EXPORT_LIST}")

# インストール先の設定
if(NOT YOURLIB_INSTALL_DIR)
    include(GNUInstallDirs)
    set(YOURLIB_LIBRARY_TYPE $<IF:$<BOOL:${YOURLIB_BUILD_SHARED}>,Shared,Static>)
    set(YOURLIB_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/${CMAKE_SYSTEM_NAME}/${YOURLIB_LIBRARY_TYPE})
endif()

# 共有ライブラリの配置
if(YOURLIB_BUILD_SHARED)
    install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${YOURLIB_INSTALL_DIR}
        RUNTIME DESTINATION ${YOURLIB_INSTALL_DIR})
endif()

# ライブラリ単体をビルドする時，インポートライブラリと静的ライブラリの配置
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION ${YOURLIB_INSTALL_DIR})
endif()

# 追加のファイルの配置（フォルダのコピー）
set(DIRECTORIES_TO_INSTALL
    ${PROJECT_SOURCE_DIR}/cmake/cfg
    ${PROJECT_SOURCE_DIR}/cmake/misc)
install(DIRECTORY ${DIRECTORIES_TO_INSTALL} DESTINATION ${YOURLIB_INSTALL_DIR})

# 追加のファイルの配置（ファイルのコピー）
set(FILES_TO_INSTALL
    ${PROJECT_SOURCE_DIR}/cmake/sample.txt)
install(FILES ${FILES_TO_INSTALL} DESTINATION ${YOURLIB_INSTALL_DIR})

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
# MinGWでlibなんとか.dllになっちゃうのを直す
if(WIN32 AND NOT MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" IMPORT_PREFIX "")
endif()
