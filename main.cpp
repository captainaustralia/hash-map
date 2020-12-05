#define CATCH_CONFIG_MAIN
#include "hash_map.hpp"
#include "catch.hpp"
#include <string>
#include <cmath>

using namespace std;
using namespace fefu; // :0
TEST_CASE("sanya.com") {
    SECTION("0000") {
        hash_map<char, string> map(10);
        hash_map<char, string> tmp({pair<char, string>('!', "asd"), pair<char, string>('@', "kmf")}, 10);
        map.merge(tmp);
        CHECK(map['!'] == "asd");
        CHECK(map['@'] == "kmf");
        CHECK(map.size() == 2);
    }

    SECTION("0000") {
        hash_map<string, int32_t> hash_map(11);

        pair<string, int> p = pair<string, int>("123", 123);
        auto res = hash_map.insert(p);
        CHECK(123 == res.first->second);
        CHECK(res.second);
        pair<string, int> p1 = pair<string, int>("123", 321);
        auto res1 = hash_map.insert(p1);
        CHECK(!res1.second);

        hash_map.insert(make_pair("321", 123));
        CHECK(hash_map["321"] == 123);
    }

    SECTION("000") {
        hash_map<size_t, size_t> hash_map(10);
        CHECK(hash_map.bucket_count() == 10);
    }

    SECTION("0000") {
        hash_map<size_t, size_t> hash_map(10);
        size_t count = 3;
        for (size_t i = 0; i < count; i++) {
            pair<size_t, size_t> p = pair<size_t, size_t>(i, i);
            hash_map.insert(p);
        }
        hash_map.max_load_factor(0.5);
        CHECK(hash_map.max_load_factor() == 0.5);
        CHECK(hash_map.load_factor() == 0.3f);
    }

    SECTION("000") {
        hash_map<char, string> hash_map(100);
        hash_map.insert(std::pair<char, string>('0', "abc"));
        hash_map.insert(std::pair<char, string>('1', "zxc"));
        hash_map.insert(std::pair<char, string>('2', "klj"));

        auto iterator = hash_map.begin();
        CHECK("abc" == iterator->second);
        CHECK("zxc" == iterator.operator++()->second);
        CHECK("klj" == iterator.operator++()->second);
    }

    SECTION("000") {
        hash_map<size_t, size_t> hash_map(10);
        hash_map[0] = 123;
        CHECK(hash_map[0] == 123);
        CHECK(hash_map[1] == 0);
        hash_map[1] = 1000;
        CHECK(hash_map[1] == 1000);
    }

    SECTION("000") {
        hash_map<size_t, size_t> hash_map(10);
        CHECK(!hash_map.contains(0));
        hash_map.insert(std::pair<size_t, size_t>(0, 123));
        CHECK(hash_map.contains(0));
    }

    SECTION("000") {
        hash_map<size_t, size_t> hash_map(10);
        CHECK(hash_map.count(0) == 0);
        hash_map.insert(std::pair<size_t, size_t>(0, 123));
        CHECK(hash_map.count(0) == 1);
    }

    SECTION("000") {
        hash_map<size_t, size_t> hash_map(10);
        CHECK(hash_map.find(0) == hash_map.end());
        hash_map.insert(std::pair<size_t, size_t>(0, 123));
        CHECK(hash_map.find(0)->second == 123);
    }

    SECTION("000") {
        hash_map<size_t, size_t> hash_map(10);
        hash_map.hash_function();
        hash_map.key_eq();
    }

    SECTION("000") {
        hash_map<string, int32_t> hash_map(10);
        pair<string, int> p = pair<string, int32_t>("123", 123);
        hash_map.insert(p);
        auto iterator = hash_map.find("123");
        CHECK(iterator->second == 123);
        auto res = hash_map.erase(iterator);
        CHECK(res == hash_map.end());
    }

    SECTION("000") {
        hash_map<string, int32_t> hash_map(10);
        pair<string, int> p = pair<string, int>("123", 123);
        hash_map.insert(p);
        auto res = hash_map.erase("123");
        CHECK(res == 1);
        res = hash_map.erase("123");
        CHECK(res == 0);
    }

    SECTION("ne rabotaet destroy powel ka ya na.......") {
        hash_map<char, string> hash_map_(100);
        hash_map_.insert(std::pair<char, string>('0', "abc"));
        hash_map_.insert(std::pair<char, string>('1', "zxc"));
        hash_map_.insert(std::pair<char, string>('2', "klj"));

        CHECK(hash_map_.find('2')->second == "klj");
        CHECK(hash_map_.size() == 3);

        hash_map_.clear();
        CHECK(hash_map_.find('0') == hash_map_.end());
        CHECK(hash_map_.find('1') == hash_map_.end());
        CHECK(hash_map_.find('2') == hash_map_.end());
        CHECK(hash_map_.size() == 0);
        CHECK(hash_map_.empty());
    }

    SECTION("000") {
        hash_map<string, int32_t> hash_map(10);
        string key = "123";
        auto res1 = hash_map.insert_or_assign<int32_t>(key, 123);
        CHECK(res1.second);
        auto res2 = hash_map.insert_or_assign<int32_t>(key, 666);
        CHECK(!res2.second);
        CHECK(hash_map[key] == 666);
    }
    
}