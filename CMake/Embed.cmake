# Based from: https://thatonegamedev.com/cpp/how-to-embed-files-in-your-c-project

function(target_embed_files target_name)
    set(options)
    set(single_value)
    set(multiple_value FILES)

    cmake_parse_arguments(EF
        "${options}"
        "${single_value}"
        "${multiple_value}"
        ${ARGN}
    )

    string(TOLOWER "${target_name}" target_name_lower)
    string(MAKE_C_IDENTIFIER "${target_name_lower}" target_name_lower)

    set(generate_header_location "${CMAKE_BINARY_DIR}/embed/include")
    set(embed_headers_location "${generate_header_location}/embed")
    set(header_filename "${target_name_lower}.h")
    set(array_declarations)
    set(map_declarations)

    foreach(input IN LISTS EF_FILES)
        # Convert input to list
        string(REPLACE ":" ";" input "${input}")
        list(LENGTH input input_length)

        if(input_length GREATER 1)
            list(GET input 0 filename)
            list(GET input 1 input_file)
        else()
            list(GET input 0 input_file)
            get_filename_component(filename "${input_file}" NAME_WE)
            string(TOLOWER "${filename}" filename)
            string(MAKE_C_IDENTIFIER "${filename}" filename)
        endif()

        file(READ "${input_file}" bytes HEX)
        file(SIZE "${input_file}" bytes_size)
        string(REGEX REPLACE "(..)" "0x\\1, " bytes "${bytes}")

        message(STATUS "Embedding ${input_file} as '${filename}'")

        string(CONFIGURE "constexpr std::array<std::uint8_t, ${bytes_size}> ${filename} = { ${bytes} }\;" current_list_declaration)
        list(APPEND array_declarations "${current_list_declaration}")

        string(CONFIGURE "{\"${filename}\", {files::${filename}.data(), ${bytes_size}}}," current_map_declaration)
        list(APPEND map_declarations "${current_map_declaration}")
    endforeach()

    string(JOIN "\n" array_declarations ${array_declarations})
    string(JOIN "\n" map_declarations ${map_declarations})

    set(basic_header "\
#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <string_view>

namespace embed {

namespace files {

${array_declarations}

} // namespace files

struct EmbedRef {
    const std::uint8_t* data;
    std::size_t size;
};

inline const std::unordered_map<std::string_view, EmbedRef> index = {
${map_declarations}
};

} // namespace embed
")

    string(CONFIGURE "${basic_header}" basic_header)
    file(WRITE "${embed_headers_location}/${header_filename}" "${basic_header}")
    target_include_directories(${target_name} PUBLIC "${generate_header_location}")
endfunction()
