#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

class Trie {
public:
    Trie();
    ~Trie() = default;

    void insert(const std::string& word, const std::string& location_id);

    std::vector<std::string> searchPrefix(const std::string& prefix,
        size_t limit = 10) const;

    std::vector<std::string> searchExact(const std::string& word) const;

    bool remove(const std::string& word, const std::string& location_id);

    void clear();

    size_t size() const;

private:
    struct TrieNode {
        std::unordered_map<char, std::unique_ptr<TrieNode>> children;
        std::vector<std::string> location_ids;
        bool is_end_of_word = false;
    };

    std::unique_ptr<TrieNode> root_;
    size_t word_count_;
    mutable std::shared_mutex mutex_;
};

#endif