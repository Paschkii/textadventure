#pragma once
// === C++ Libraries ===
#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace items {
    enum class Category {
        Consumable,
        Equipment,
        Artifact,
        Tool,
        Misc
    };

    struct ItemDefinition {
        std::string title;
        std::string description;
        Category category;
        std::vector<std::string> effects;
        std::string actionLabel;
        std::string categoryLabel;
    };

    const ItemDefinition* definitionFor(const std::string& key) noexcept;
    std::string defaultActionLabel(Category category) noexcept;
}
