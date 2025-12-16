#pragma once
// === C++ Libraries ===
#include <cstdint>     // Stores the unique ID for each recorded session.
#include <cstddef>     // Provides std::size_t used in the manager API.
#include <string>      // Holds player names and timestamps.
#include <vector>      // Keeps the list of saved ranking entries.

namespace core {

struct RankingEntry {
    std::uint64_t id = 0;
    std::string playerName;
    double totalSeconds = 0.0;
    int faults = 0;
    std::string started;
};

class RankingManager {
public:
    RankingManager() = default;

    // Loads entries from the provided JSON path. Returns false on error.
    bool load(const std::string& path);
    // Persist the current entries to disk.
    bool save() const;
    // Adds a new entry, keeps at most @maxEntries sorted by time, and returns the 1-based rank.
    int recordEntry(RankingEntry entry, std::size_t maxEntries = 128);

    const std::vector<RankingEntry>& entries() const noexcept { return entries_; }

private:
    static std::string escapeString(const std::string& value);
    static bool parseEntries(const std::string& raw, std::vector<RankingEntry>& out);
    static std::uint64_t parseUnsigned(const std::string& raw, std::size_t& pos);
    static double parseDouble(const std::string& raw, std::size_t& pos);
    static int parseInt(const std::string& raw, std::size_t& pos);
    static std::string parseString(const std::string& raw, std::size_t& pos);
    static void skipWhitespace(const std::string& raw, std::size_t& pos);

    std::vector<RankingEntry> entries_;
    std::string path_;
};

} // namespace core
