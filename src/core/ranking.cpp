// === C++ Libraries ===
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <iterator>
#include <sstream>

// === Header Files ===
#include "ranking.hpp"

namespace {
bool isNumberChar(char c) {
    return c == '+' || c == '-' || c == '.' || c == 'e' || c == 'E' || (c >= '0' && c <= '9');
}
} // namespace

namespace core {

bool RankingManager::load(const std::string& path) {
    path_ = path;
    entries_.clear();
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(path_, ec))
        return true;

    std::ifstream in(path_);
    if (!in)
        return false;

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (!parseEntries(content, entries_)) {
        entries_.clear();
        return false;
    }

    std::sort(entries_.begin(), entries_.end(), [](const RankingEntry& lhs, const RankingEntry& rhs) {
        if (lhs.totalSeconds != rhs.totalSeconds)
            return lhs.totalSeconds < rhs.totalSeconds;
        if (lhs.faults != rhs.faults)
            return lhs.faults < rhs.faults;
        return lhs.id < rhs.id;
    });

    return true;
}

bool RankingManager::save() const {
    if (path_.empty())
        return false;

    namespace fs = std::filesystem;
    std::error_code ec;
    auto parent = fs::path(path_).parent_path();
    if (!parent.empty())
        fs::create_directories(parent, ec);

    std::ofstream out(path_, std::ios::trunc);
    if (!out)
        return false;

    out << "[\n";
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        const auto& entry = entries_[i];
        out << "  {\n";
        out << "    \"id\": " << entry.id << ",\n";
        out << "    \"playerName\": \"" << escapeString(entry.playerName) << "\",\n";
        out << std::fixed << std::setprecision(3);
        out << "    \"totalSeconds\": " << entry.totalSeconds << ",\n";
        out << "    \"faults\": " << entry.faults << ",\n";
        out << "    \"started\": \"" << escapeString(entry.started) << "\"\n";
        out << "  }";
        if (i + 1 < entries_.size())
            out << ",\n";
        else
            out << "\n";
    }
    out << "]";

    return out.good();
}

int RankingManager::recordEntry(RankingEntry entry, std::size_t maxEntries) {
    entries_.push_back(entry);
    std::sort(entries_.begin(), entries_.end(), [](const RankingEntry& lhs, const RankingEntry& rhs) {
        if (lhs.totalSeconds != rhs.totalSeconds)
            return lhs.totalSeconds < rhs.totalSeconds;
        if (lhs.faults != rhs.faults)
            return lhs.faults < rhs.faults;
        return lhs.id < rhs.id;
    });

    if (entries_.size() > maxEntries)
        entries_.resize(maxEntries);

    int rank = -1;
    for (std::size_t idx = 0; idx < entries_.size(); ++idx) {
        if (entries_[idx].id == entry.id) {
            rank = static_cast<int>(idx) + 1;
            break;
        }
    }

    save();
    return rank;
}

std::string RankingManager::escapeString(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

void RankingManager::skipWhitespace(const std::string& raw, std::size_t& pos) {
    while (pos < raw.size() && std::isspace(static_cast<unsigned char>(raw[pos])))
        ++pos;
}

std::string RankingManager::parseString(const std::string& raw, std::size_t& pos) {
    std::string result;
    if (pos >= raw.size() || raw[pos] != '"')
        return result;

    ++pos;
    while (pos < raw.size()) {
        char c = raw[pos++];
        if (c == '\\' && pos < raw.size()) {
            char escaped = raw[pos++];
            switch (escaped) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += escaped; break;
            }
        }
        else if (c == '"') {
            return result;
        }
        else {
            result += c;
        }
    }

    return result;
}

std::uint64_t RankingManager::parseUnsigned(const std::string& raw, std::size_t& pos) {
    skipWhitespace(raw, pos);
    if (pos >= raw.size() || !std::isdigit(static_cast<unsigned char>(raw[pos])))
        return 0;

    char* endPtr = nullptr;
    const char* beginPtr = raw.data() + pos;
    unsigned long long value = std::strtoull(beginPtr, &endPtr, 10);
    pos = static_cast<std::size_t>(endPtr - raw.data());
    return static_cast<std::uint64_t>(value);
}

int RankingManager::parseInt(const std::string& raw, std::size_t& pos) {
    skipWhitespace(raw, pos);
    if (pos >= raw.size())
        return 0;

    char* endPtr = nullptr;
    const char* beginPtr = raw.data() + pos;
    long value = std::strtol(beginPtr, &endPtr, 10);
    pos = static_cast<std::size_t>(endPtr - raw.data());
    return static_cast<int>(value);
}

double RankingManager::parseDouble(const std::string& raw, std::size_t& pos) {
    skipWhitespace(raw, pos);
    if (pos >= raw.size())
        return 0.0;

    char* endPtr = nullptr;
    const char* beginPtr = raw.data() + pos;
    double value = std::strtod(beginPtr, &endPtr);
    if (endPtr == beginPtr) {
        ++pos;
        return 0.0;
    }
    pos = static_cast<std::size_t>(endPtr - raw.data());
    return value;
}

bool RankingManager::parseEntries(const std::string& raw, std::vector<RankingEntry>& out) {
    std::size_t pos = 0;
    skipWhitespace(raw, pos);
    if (pos >= raw.size() || raw[pos] != '[')
        return false;
    ++pos;

    skipWhitespace(raw, pos);
    if (pos < raw.size() && raw[pos] == ']')
        return true;

    while (pos < raw.size()) {
        skipWhitespace(raw, pos);
        if (pos >= raw.size() || raw[pos] != '{')
            return false;
        ++pos;

        RankingEntry entry;
        while (pos < raw.size()) {
            skipWhitespace(raw, pos);
            if (pos >= raw.size())
                return false;
            if (raw[pos] == '}') {
                ++pos;
                break;
            }

            std::string key = parseString(raw, pos);
            skipWhitespace(raw, pos);
            if (pos >= raw.size() || raw[pos] != ':')
                return false;
            ++pos;
            skipWhitespace(raw, pos);

            if (key == "id")
                entry.id = parseUnsigned(raw, pos);
            else if (key == "playerName")
                entry.playerName = parseString(raw, pos);
            else if (key == "totalSeconds")
                entry.totalSeconds = parseDouble(raw, pos);
            else if (key == "faults")
                entry.faults = parseInt(raw, pos);
            else if (key == "started")
                entry.started = parseString(raw, pos);
            else if (pos < raw.size() && raw[pos] == '"')
                parseString(raw, pos);
            else
                parseDouble(raw, pos);

            skipWhitespace(raw, pos);
            if (pos < raw.size() && raw[pos] == ',') {
                ++pos;
                continue;
            }
        }

        out.push_back(std::move(entry));
        skipWhitespace(raw, pos);
        if (pos < raw.size() && raw[pos] == ',') {
            ++pos;
            continue;
        }
        if (pos < raw.size() && raw[pos] == ']') {
            ++pos;
            break;
        }
    }

    skipWhitespace(raw, pos);
    return true;
}

} // namespace core
