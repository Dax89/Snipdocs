CPMAddPackage(
    NAME RapidJSON
    GIT_TAG "ab1842a2dae061284c0a62dca1cc6d5e7e37e346"
    GITHUB_REPOSITORY "Tencent/rapidjson"
    OPTIONS
        "RAPIDJSON_BUILD_TESTS OFF"
        "RAPIDJSON_BUILD_DOC OFF"
        "RAPIDJSON_BUILD_EXAMPLES OFF"
        "RAPIDJSON_BUILD_CXX11 OFF"
        "RAPIDJSON_BUILD_CXX17 ON"
)

if(RapidJSON_ADDED)
    add_library(rapidjson INTERFACE IMPORTED)
    target_include_directories(rapidjson INTERFACE "${RapidJSON_SOURCE_DIR}/include")

    target_compile_definitions(rapidjson
        INTERFACE
            "RAPIDJSON_HAS_STDSTRING"
    )
endif()
