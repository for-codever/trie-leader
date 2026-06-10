#include "search_engine.h"
#include <algorithm>
#include <set>

SearchEngine& SearchEngine::getInstance() {
    static SearchEngine instance;
    return instance;
}

void SearchEngine::initialize() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (initialized_) return;

    trie_ = std::make_unique<Trie>();

    const auto& location_ids = LocationStore::getInstance().getLocationIds();
    for (const auto& id : location_ids) {
        auto [name, lat, lon, desc] = LocationStore::getInstance().getLocation(id);
        trie_->insert(name, id);
    }

    initialized_ = true;
}

std::vector<std::tuple<std::string, std::string, double, double, std::string>>
SearchEngine::search(const std::string& query, size_t limit) {
    if (!initialized_) initialize();

    std::shared_lock<std::shared_mutex> lock(mutex_);

    std::set<std::string> unique_ids;
    auto exact_results = trie_->searchExact(query);
    for (const auto& id : exact_results) {
        unique_ids.insert(id);
    }

    auto prefix_results = trie_->searchPrefix(query, limit);
    for (const auto& id : prefix_results) {
        unique_ids.insert(id);
    }

    auto fuzzy_results = LocationStore::getInstance().searchByName(query);
    for (const auto& id : fuzzy_results) {
        unique_ids.insert(id);
    }

    std::vector<std::string> result_ids(unique_ids.begin(), unique_ids.end());
    auto ranked = rankResults(result_ids);

    if (ranked.size() > limit) {
        ranked.resize(limit);
    }

    std::vector<std::string> ids_only;
    for (const auto& res : ranked) {
        ids_only.push_back(std::get<0>(res));
    }
    recordSearch(query, ids_only);

    return ranked;
}

std::vector<std::string> SearchEngine::getSuggestions(const std::string& prefix,
    size_t limit) {
    if (!initialized_) initialize();

    std::shared_lock<std::shared_mutex> lock(mutex_);
    return trie_->searchPrefix(prefix, limit);
}

std::vector<std::tuple<std::string, std::string, double, double, std::string>>
SearchEngine::searchNearby(double latitude, double longitude,
    double radius_km, size_t limit) {
    if (!initialized_) initialize();

    auto nearby_ids = LocationStore::getInstance().searchByRadius(
        latitude, longitude, radius_km);

    auto ranked = rankResults(nearby_ids);

    if (ranked.size() > limit) {
        ranked.resize(limit);
    }

    return ranked;
}

void SearchEngine::recordSearch(const std::string& query,
    const std::vector<std::string>& result_ids) {
    HistoryStore::getInstance().addSearchHistory(query, result_ids);
}

void SearchEngine::recordClick(const std::string& query,
    const std::string& location_id) {
    HistoryStore::getInstance().addClickHistory(query, location_id);
}

std::vector<std::pair<std::string, std::chrono::system_clock::time_point>>
SearchEngine::getRecentSearches(size_t limit) const {
    return HistoryStore::getInstance().getRecentSearches(limit);
}

std::vector<std::pair<std::string, size_t>>
SearchEngine::getTrendingSearches(size_t limit) const {
    return HistoryStore::getInstance().getTopSearches(limit);
}

void SearchEngine::clearAllData() {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    LocationStore::getInstance().clear();
    HistoryStore::getInstance().clearHistory();
    if (trie_) {
        trie_->clear();
    }

    initialized_ = false;
}

std::vector<std::tuple<std::string, std::string, double, double, std::string>>
SearchEngine::rankResults(const std::vector<std::string>& location_ids) const {
    std::vector<std::tuple<std::string, std::string, double, double, std::string>> results;

    auto top_clicks = HistoryStore::getInstance().getTopClickedLocations(100);
    std::unordered_map<std::string, size_t> click_scores;
    for (size_t i = 0; i < top_clicks.size(); ++i) {
        click_scores[top_clicks[i].first] = top_clicks.size() - i;
    }

    std::vector<std::pair<std::string, size_t>> scored_ids;
    for (const auto& id : location_ids) {
        size_t score = click_scores.count(id) ? click_scores[id] : 0;
        scored_ids.emplace_back(id, score);
    }

    std::sort(scored_ids.begin(), scored_ids.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

    for (const auto& [id, score] : scored_ids) {
        auto [name, lat, lon, desc] = LocationStore::getInstance().getLocation(id);
        if (!name.empty()) {
            results.emplace_back(id, name, lat, lon, desc);
        }
    }

    return results;
}