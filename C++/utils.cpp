#include <cstddef>
#include <vector>
#include <string_view>

std::vector<std::string_view> split(std::string_view sv, char sep)
{
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
