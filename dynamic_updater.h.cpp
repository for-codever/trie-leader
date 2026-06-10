#ifndef DYNAMIC_UPDATER_H
#define DYNAMIC_UPDATER_H

#include "search_engine.h"
#include "location_store.h"
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

class DynamicUpdater {
public:
    static DynamicUpdater& getInstance();

    void start(size_t update_interval_ms = 5000);

    void stop();

    void addLocationToUpdate(const std::string& id, const std::string& name,
        double latitude, double longitude,
        const std::string& description);

    void removeLocationToUpdate(const std::string& id);

    void forceUpdate();

    bool isRunning() const;

    size_t pendingUpdates() const;

private:
    DynamicUpdater() = default;
    ~DynamicUpdater();
    DynamicUpdater(const DynamicUpdater&) = delete;
    DynamicUpdater& operator=(const DynamicUpdater&) = delete;

    void updateLoop();

    void processUpdates();

    struct UpdateTask {
        enum Type { ADD, REMOVE } type;
        std::string id;
        std::string name;
        double latitude;
        double longitude;
        std::string description;
    };

    std::thread update_thread_;
    std::atomic<bool> running_{ false };
    size_t update_interval_ms_{ 5000 };

    std::queue<UpdateTask> update_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
};

#endif