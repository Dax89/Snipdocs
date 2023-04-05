#include <vector>
#include <string_view>

std::vector<std::string_view> split(std::string_view sv, char sep)
{
    std::vector<std::string_view> s;
    usize start = 0;

    for(usize i = 0; i < sv.size(); ++i)
    {
        if(sv[i] != sep) continue;
        if(i > start) s.emplace_back(sv.data() + start, i - start);
        start = i + 1;
    }

    if(start < sv.size()) s.push_back(sv.substr(start));
    return s;
}
