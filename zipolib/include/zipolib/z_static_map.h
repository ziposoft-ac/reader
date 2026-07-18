//
// Created by ac on 7/18/26.
//

#ifndef ZIPOSOFT_Z_STATIC_MAP_H
#define ZIPOSOFT_Z_STATIC_MAP_H
#include <iostream>
#include <array>
#include <utility>
#include <stdexcept>
#include <string_view>


// 1. Define the Key-Value Pair structure
template <typename K, typename V>
struct ConstexprPair {
    K key;
    V value;
};

// 2. Define the Fixed-Size Map
template <typename K, typename V, std::size_t N>
class StaticMap {
public:
    std::array<ConstexprPair<K, V>, N> data;

    // Compile-time lookup function
    constexpr const V& at(const K& key) const {
        for (const auto& item : data) {
            if (item.key == key) {
                return item.value;
            }
        }
        throw std::out_of_range("Key not found in StaticMap");
    }

    constexpr std::size_t size() const { return N; }
};

// 3. Optional: Helper function to enable template argument deduction (CTAD)
template <typename K, typename V, std::size_t N>
constexpr auto make_static_map(ConstexprPair<K, V> (&&init)[N]) {
    StaticMap<K, V, N> map{};
    for (std::size_t i = 0; i < N; ++i) {
        map.data[i] = init[i];
    }
    return map;
}



#endif //ZIPOSOFT_Z_STATIC_MAP_H
