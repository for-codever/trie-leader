#include "search_engine.h"
#include "dynamic_updater.h"
#include <iostream>
#include <iomanip>
#include <thread>

int main() {
    auto& loc_store = LocationStore::getInstance();
    auto& search_engine = SearchEngine::getInstance();
    auto& updater = DynamicUpdater::getInstance();

    loc_store.addLocation("loc1", "第一教学楼", 30.7501, 104.0001,
        "主教学楼，包含1-10号教室");
    loc_store.addLocation("loc2", "第二教学楼", 30.7503, 104.0012,
        "理科实验楼，配备多媒体教室");
    loc_store.addLocation("loc3", "图书馆", 30.7498, 103.9995,
        "校图书馆，藏书100万册");
    loc_store.addLocation("loc4", "一食堂", 30.7512, 104.0021,
        "学生第一食堂，提供各地美食");
    loc_store.addLocation("loc5", "二食堂", 30.7523, 104.0018,
        "学生第二食堂，清真餐厅");
    loc_store.addLocation("loc6", "学生宿舍1栋", 30.7531, 104.0032,
        "男生宿舍区");
    loc_store.addLocation("loc7", "学生宿舍5栋", 30.7535, 104.0028,
        "女生宿舍区");
    loc_store.addLocation("loc8", "田径场", 30.7489, 104.0025,
        "标准400米跑道运动场");
    loc_store.addLocation("loc9", "体育馆", 30.7492, 104.0031,
        "室内体育馆，篮球排球场地");
    loc_store.addLocation("loc10", "实验楼", 30.7505, 103.9987,
        "综合实验楼，物理化学实验室");
    loc_store.addLocation("loc11", "行政楼", 30.7485, 103.9990,
        "学校行政办公大楼");
    loc_store.addLocation("loc12", "南门", 30.7478, 104.0000,
        "学校正门");
    loc_store.addLocation("loc13", "北门", 30.7542, 104.0020,
        "学校后门，靠近生活区");
    loc_store.addLocation("loc14", "超市", 30.7528, 104.0025,
        "校园超市，日用品齐全");
    loc_store.addLocation("loc15", "校医院", 30.7518, 104.0005,
        "校内医疗服务中心");

    search_engine.initialize();
    updater.start(3000);

    std::cout << "搜索'教学楼':\n";
    auto results = search_engine.search("教学楼");
    for (const auto& res : results) {
        std::cout << std::get<1>(res) << " - " << std::get<4>(res) << "\n";
    }

    std::cout << "\n搜索'食堂':\n";
    results = search_engine.search("食堂");
    for (const auto& res : results) {
        std::cout << std::get<1>(res) << " - " << std::get<4>(res) << "\n";
    }

    updater.addLocationToUpdate("loc16", "计算机学院", 30.7507, 103.9982,
        "计算机科学与技术学院办公楼");
    updater.addLocationToUpdate("loc17", "图书馆咖啡厅", 30.7499, 103.9997,
        "图书馆一楼休闲区");

    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::cout << "\n添加后搜索'计算机':\n";
    results = search_engine.search("计算机");
    for (const auto& res : results) {
        std::cout << std::get<1>(res) << " - " << std::get<4>(res) << "\n";
    }

    std::cout << "\n附近搜索(图书馆位置, 500米):\n";
    results = search_engine.searchNearby(30.7498, 103.9995, 0.5);
    for (const auto& res : results) {
        std::cout << std::get<1>(res) << " (" << std::fixed << std::setprecision(4)
            << std::get<2>(res) << ", " << std::get<3>(res) << ")\n";
    }

    std::cout << "\n输入前缀'宿'获取建议:\n";
    auto suggestions = search_engine.getSuggestions("宿");
    for (const auto& id : suggestions) {
        auto [name, lat, lon, desc] = loc_store.getLocation(id);
        std::cout << name << "\n";
    }

    std::cout << "\n最近搜索记录:\n";
    auto recent = search_engine.getRecentSearches();
    for (const auto& [query, time] : recent) {
        std::cout << query << "\n";
    }

    search_engine.recordClick("食堂", "loc4");
    search_engine.recordClick("食堂", "loc4");
    search_engine.recordClick("教学楼", "loc1");

    std::cout << "\n热门点击地点:\n";
    auto top_clicks = HistoryStore::getInstance().getTopClickedLocations(5);
    for (const auto& [id, count] : top_clicks) {
        auto [name, lat, lon, desc] = loc_store.getLocation(id);
        std::cout << name << " - 点击次数: " << count << "\n";
    }

    updater.stop();
    return 0;
}