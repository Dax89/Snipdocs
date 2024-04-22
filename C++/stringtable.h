#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>

class StringTable {
public:
    using Id = size_t;

public:
    StringTable(const StringTable&) = delete;
    StringTable(StringTable&&) = delete;
    StringTable& operator=(const StringTable&) = delete;
    StringTable& operator=(StringTable&&) = delete;

public:
    inline bool empty() const { return !m_ntable; }
    inline size_t size() const { return m_ntable; }
    std::string_view find(Id id) const;
    Id insert(std::string_view s);
    void erase(std::string_view s);
    void erase(Id id);

private:
    const char* get_string(const char* item) const;
    char* get_item(Id id);
    Id& get_next_id();

private:
    inline Id get_next_id() const {
        return const_cast<StringTable*>(this)->get_next_id();
    }

    inline const char* get_item(Id id) const {
        return const_cast<StringTable*>(this)->get_item(id);
    }

private:
    void check_rehash();

private:
    std::vector<char> m_table;
    std::unordered_map<size_t, Id> m_hashes; // Hash = Id
    std::unordered_map<Id, size_t> m_ids;    // Id = Index
    size_t m_nfree{0}, m_ntable{0};
};
