#include "stringtable.h"

namespace {

constexpr float STRINGTABLE_REHASH = 0.75;

size_t stringtable_gethash(std::string_view s) {
    return std::hash<std::string_view>{}(s);
}

StringTable::Id stringtable_getid(const char* item) {
    return *reinterpret_cast<const StringTable::Id*>(++item);
}

} // namespace

std::string_view StringTable::find(Id id) const {
    const char* item = this->get_item(id);
    if(item)
        return this->get_string(item);
    return {};
}

StringTable::Id StringTable::insert(std::string_view s) {
    this->check_rehash();
    size_t h = stringtable_gethash(s);

    if(auto it = m_hashes.find(h); it != m_hashes.end()) {
        char* item = this->get_item(it->second);
        *item = 1; // Valid
        m_nfree--;
        m_ntable++;
        return stringtable_getid(item);
    }

    Id id = ++this->get_next_id();
    size_t index = m_table.size();
    const auto* pid = reinterpret_cast<const char*>(&id);

    m_table.reserve(m_table.size() + s.size() + sizeof(Id) + 2);
    m_table.push_back(1);                                 // Valid
    m_table.insert(m_table.end(), pid, pid + sizeof(Id)); // Id
    m_table.insert(m_table.end(), s.begin(), s.end());    // String
    m_table.push_back(0);                                 // Null-Terminator

    m_hashes[h] = id;
    m_ids[id] = index;
    m_ntable++;
    return id;
}

void StringTable::erase(std::string_view s) {
    auto it = m_hashes.find(stringtable_gethash(s));
    if(it != m_hashes.end())
        this->erase(it->second);
}

void StringTable::erase(Id id) {
    char* item = this->get_item(id);

    if(item && *item == 1) {
        *item = 0;
        m_nfree++;
        m_ntable--;
    }
}

StringTable::Id& StringTable::get_next_id() {
    if(m_table.empty())
        m_table.resize(sizeof(Id), 0);
    return *reinterpret_cast<Id*>(m_table.data());
}

char* StringTable::get_item(Id id) {
    auto it = m_ids.find(id);
    if(it == m_ids.end())
        return nullptr;
    return m_table.data() + it->second;
}

const char* StringTable::get_string(const char* item) const {
    if(*item != 1)
        return nullptr;
    return ++item + sizeof(Id);
}

void StringTable::check_rehash() {
    if(!m_ntable)
        return;

    float ratio = m_nfree / static_cast<float>(m_ntable);
    if(ratio < STRINGTABLE_REHASH)
        return;

    std::vector<char> newtable;
    newtable.reserve(m_table.size());

    std::unordered_map<size_t, size_t> newhashes;
    newhashes.reserve(m_hashes.size());

    std::unordered_map<size_t, size_t> newids;
    newids.reserve(m_ids.size());

    // Copy NextId
    for(size_t i = 0; i < sizeof(Id); i++)
        newtable.push_back(m_table[i]);

    for(const auto& [h, id] : m_hashes) {
        const char* olditem = m_table.data() + m_ids.at(id);
        if(*olditem != 1)
            continue;

        const auto* pid = reinterpret_cast<const char*>(&id);
        size_t newindex = newtable.size();

        newtable.push_back(1);                                  // Valid
        newtable.insert(newtable.end(), pid, pid + sizeof(Id)); // Id

        // String
        for(const char* s = this->get_string(olditem); *s; s++)
            newtable.push_back(*s);

        newtable.push_back(0); // Null Terminator

        newhashes.try_emplace(h, id);
        newids.try_emplace(id, newindex);
    }

    m_table.swap(newtable);
    m_hashes.swap(newhashes);
    m_ids.swap(newids);
    m_nfree = 0;
}
