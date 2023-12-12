#include <cstddef>
#include <vector>
#include <string_view>
#include <array>
#include <fmt/core.h>
#include <sstream>
#include <iomanip>
#include <fstream>

std::string file_size(double size) {
    static constexpr std::array<const char*, 9> UNITS = {
        "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    int i = 0;

    while(size > 1024) {
        size /= 1024.0;
        i++;
    }

#if __has_include(<fmt/core.h>)
    return fmt::format("{:.2f}{}", size, UNITS.at(i));
#else
    std::ostringstream o;
    o << std::fixed 
      << std::setprecision(2) 
      << size
      << UNITS.at(i);

    return o.str();
#endif
}

std::vector<std::string_view> split(std::string_view sv, char sep) {
    std::vector<std::string_view> s;
    size_t start = 0;

    for(size_t i = start; i < sv.size(); ++i)
    {
        if(sv[i] != sep) continue;
        if(i > start) s.emplace_back(sv.data() + start, i - start);
        start = i + 1;
    }

    if(start < sv.size()) s.push_back(sv.substr(start));
    return s;
}

std::string_view trim(std::string_view v) {
    int start = 0, end = v.size() - 1;

    while(start < static_cast<int>(v.size()) && std::isblank(v[start]))
        start++;

    while(end >= 0 && std::isblank(v[end]))
        end--;

    if(start < end)
        return v.substr(start, end - start + 1);

    return std::string_view{};
}

template <typename Function>
void splittrim_each(std::string_view s, char ch, Function f) {
    size_t i = 0, start = 0;

    for(; i < s.size(); i++) {
        if(s[i] != ch)
            continue;

        f(trim(s.substr(start, i - start)));
        start = i + 1;
    }

    if(start < s.size())
        f(trim(s.substr(start)));
}

Data read_file(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    Data result(pos);

    ifs.seekg(0, std::ios::beg)
        .read(reinterpret_cast<char*>(result.data()), pos);

    return result;
}

bool write_file(const std::string& filepath, const Data& data) {
    std::ofstream ofs;

    ofs.open(filepath, std::ios::binary);
    if(!ofs.is_open())
        return false;

    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}
