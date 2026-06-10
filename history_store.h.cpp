#ifndef HISTORY_STORE_H
#define HISTORY_STORE_H

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <shared_mutex>
#include <chrono>

class HistoryStore {
public:
    static HistoryStore& getInstance();

    void addSearchHistory(const std::string& query,
        const std::vector<std::string>& results);

    void addClickHistory(const std::string& query,
        const std::string& location_id);

    std::vector<std::pair<std::string, std::chrono::system_clock::time_point>>
        getRecentSearches(size_t limit = 50) const;

    std::vector<std::pair<std::string, size_t>>
        getTopSearches(size_t limit = 10) const;

    std::vector<std::pair<std::string, size_t>>
        getTopClickedLocations(size_t limit = 10) const;

    void clearHistory();

    void clearSearchHistory();

    void clearClickHistory();

    size_t searchHistorySize() const;

    size_t clickHistorySize() const;

private:
    HistoryStore() = default;
    ~HistoryStore() = default;
    HistoryStore(const HistoryStore&) = delete;
    HistoryStore& operator=(const HistoryStore&) = delete;

    struct SearchEntry {
        std::string query;
        std::vector<std::string> results;
        std::chrono::system_clock::time_point timestamp;
    };

    struct ClickEntry {
        std::string query;
        std::string location_id;
        std::chrono::system_clock::time_point timestamp;
    };

    std::deque<SearchEntry> search_history_;
    std::deque<ClickEntry> click_history_;
    std::unordered_map<std::string, size_t> search_count_;
    std::unordered_map<std::string, size_t> click_count_;

    mutable std::shared_mutex mutex_;
    const size_t max_history_size_ = 1000;
};

#endif