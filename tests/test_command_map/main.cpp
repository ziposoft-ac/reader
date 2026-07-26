
#include "zipolib/zipolib.h"
#include "zipolib/z_factory.h"
#include "zipolib/z_time.h"
#include "zipolib/z_console.h"
#include "zipolib/z_error.h"

#include <stdio.h>

#include <iostream>
#include <unordered_map>
#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

// 1. Define an enum for the registered function types
enum class FuncType {
    Getter,
    Setter,
    Action
};

// 2. The central registry managing type-erased callbacks
class CommandRegistry {
private:
    // Maps a string key to a pair containing the metadata enum and the type-erased function
    std::unordered_map<std::string, std::pair<FuncType, std::any>> registry;

public:
    // The register function uses template deduction to extract Return and Args types
    template <typename Obj, typename Ret, typename... Args>
    void register_func(std::string_view key, FuncType type, Ret (Obj::*mem_ptr)(Args...), Obj* instance) {

        // Wrap the member function pointer and instance into a type-erased std::move_only_function
        std::move_only_function<Ret(Args...)> wrapped = [instance, mem_ptr](Args&&... args) -> Ret {
            return (instance->*mem_ptr)(std::forward<Args>(args)...);
        };

        // Store the enum alongside the std::any container holding our move_only_function
        registry[std::string(key)] = std::make_pair(type, std::any(std::move(wrapped)));
    }

    // Invoker function that casts the std::any back to the exact required signature
    template <typename Ret, typename... Args>
    Ret invoke(std::string_view key, Args&&... args) {
        auto it = registry.find(std::string(key));
        if (it == registry.end()) {
            throw std::runtime_error("Function not found");
        }

        // Extract the std::move_only_function from std::any
        using TargetFunc = std::move_only_function<Ret(Args...)>;
        auto* func_ptr = std::any_cast<TargetFunc>(&it->second.second);

        if (!func_ptr) {
            throw std::bad_any_cast();
        }

        // std::move_only_function requires std::move to be invoked if it's an rvalue reference
        return std::move(*func_ptr)(std::forward<Args>(args)...);
    }

    // Helper to peek at the deduced enum type without invoking
    FuncType get_type(std::string_view key) {
        return registry.at(std::string(key)).first;
    }
};

// 3. Example target class containing member functions with different signatures
class Player {
public:
    int get_health() { return 100; }
    void set_name(const std::string& name) { std::cout << "Name set to: " << name << "\n"; }
    void attack(std::string_view enemy, int damage) {
        std::cout << "Attacking " << enemy << " for " << damage << " damage!\n";
    }
};

int main() {
    CommandRegistry manager;
    Player player_instance;

    // The register_func deduces Return, Object Type, and Argument Pack automatically
    manager.register_func("GetHP", FuncType::Getter, &Player::get_health, &player_instance);
    manager.register_func("SetName", FuncType::Setter, &Player::set_name, &player_instance);
    manager.register_func("Atk", FuncType::Action, &Player::attack, &player_instance);

    // Later invocation with completely different signatures:

    // Call int()
    int hp = manager.invoke<int>("GetHP");
    std::cout << "Retrieved HP: " << hp << "\n";

    // Call void(const std::string&)
    manager.invoke<void>("SetName", std::string("Hero"));

    // Call void(std::string_view, int)
    manager.invoke<void>("Atk", std::string_view("Dragon"), 45);

    return 0;
}
