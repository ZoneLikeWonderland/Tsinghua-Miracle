#ifndef CARD_H_
#define CARD_H_
#include <iostream>
#include <string>
#include "json.hpp"
using json = nlohmann::json;

namespace card
{

struct Creature // 生物
{
    std::string type;    // 种类
    int available_count; // 生物槽容量
    int level;           // 等级
    int cost;            // 法力消耗
    int atk;             // 攻击
    int max_hp;          // 最大生命值
    int min_atk_range;   // 最小攻击范围
    int max_atk_range;   // 最大攻击范围
    int max_move;        // 行动力
    int cool_down;       // 冷却时间
    bool flying;         // 是否飞行
    bool atk_flying;     // 能否对空
    bool agility;        // 是否迅捷
    bool holy_shield;    // 有无圣盾
    Creature() {}
    Creature(std::string _type, int _count, int _level, int _cost, int _atk, int _maxhp,
             int _minatk, int _maxatk, int _maxmove, int _cool, bool _fly,
             bool _atkfly, bool _agility, bool _holyshield)
        : type(_type), available_count(_count), level(_level), cost(_cost), atk(_atk),
          max_hp(_maxhp), min_atk_range(_minatk), max_atk_range(_maxatk),
          max_move(_maxmove), cool_down(_cool),
          flying(_fly), atk_flying(_atkfly),
          agility(_agility), holy_shield(_holyshield) {}
};

struct Artifact // 神器
{
    std::string name;        // 名字
    int cost;                // 法力消耗
    int cool_down;           // 冷却时间
    std::string target_type; // 目标类型
    Artifact() {}
    Artifact(std::string _name, int _cost, int _cool, std::string _targettype)
        : name(_name), cost(_cost), cool_down(_cool), target_type(_targettype) {}
};

Creature SWORDSMAN[4];     // 剑士
Creature ARCHER[4];        // 弓箭手
Creature BLACKBAT[4];      // 黑蝙蝠
Creature PRIEST[4];        // 牧师
Creature VOLCANOGRAGON[4]; // 火山之龙
Creature FROSTDRAGON[4];   // 冰霜之龙
Creature INFERNO;          // 地狱火

Artifact HOLYLIGHT;        // 圣光之耀
Artifact SALAMANDERSHIELD; // 阳炎之盾
Artifact INFERNOFLAME;     // 地狱之火
Artifact WINDBLESSING;     // 风神之佑

const std::map<std::string, Creature *> CARD_DICT = {
    std::map<std::string, Creature *>::value_type("Swordsman", SWORDSMAN),
    std::map<std::string, Creature *>::value_type("Archer", ARCHER),
    std::map<std::string, Creature *>::value_type("BlackBat", BLACKBAT),
    std::map<std::string, Creature *>::value_type("Priest", PRIEST),
    std::map<std::string, Creature *>::value_type("VolcanoDragon", VOLCANOGRAGON),
    std::map<std::string, Creature *>::value_type("FrostDragon", FROSTDRAGON)};

void get_data_from_json(json all_data);

void get_data_from_json(json all_data)
{
    json creature_data = all_data["UnitData"];
    json artifact_data = all_data["Artifacts"];
    for (auto i = CARD_DICT.begin(); i != CARD_DICT.end(); i++)
    {
        std::string name = i->first;
        json data = creature_data[name];
        for (int level = 1; level < 4; ++level)
        {
            i->second[level] = Creature(name, data["duplicate"][level - 1], level, data["cost"][level - 1],
                                        data["atk"][level - 1], data["hp"][level - 1], data["atk_range"][level - 1][0],
                                        data["atk_range"][level - 1][1], data["max_move"][level - 1], data["cool_down"][level - 1],
                                        data["flying"], data["atk_flying"], data["agility"], data["holy_shield"]);
        }
    }

    INFERNO = Creature("Inferno", creature_data["Inferno"]["duplicate"][0], 1, creature_data["Inferno"]["cost"][0],
                       creature_data["Inferno"]["atk"][0], creature_data["Inferno"]["hp"][0],
                       creature_data["Inferno"]["atk_range"][0][0], creature_data["Inferno"]["atk_range"][0][1],
                       creature_data["Inferno"]["max_move"][0], creature_data["Inferno"]["cool_down"][0],
                       creature_data["Inferno"]["flying"], creature_data["Inferno"]["atk_flying"],
                       creature_data["Inferno"]["agility"], creature_data["Inferno"]["holy_shield"]);
    HOLYLIGHT = Artifact("Holylight", artifact_data["HolyLight"]["cost"],
                         artifact_data["HolyLight"]["cool_down"], artifact_data["HolyLight"]["target_type"]);
    SALAMANDERSHIELD = Artifact("SalamanderShield", artifact_data["SalamanderShield"]["cost"],
                                artifact_data["SalamanderShield"]["cool_down"], artifact_data["SalamanderShield"]["target_type"]);
    INFERNOFLAME = Artifact("InfernoFlame", artifact_data["InfernoFlame"]["cost"],
                            artifact_data["InfernoFlame"]["cool_down"], artifact_data["InfernoFlame"]["target_type"]);
    WINDBLESSING = Artifact("WindBlessing", artifact_data["WindBlessing"]["cost"],
                            artifact_data["WindBlessing"]["cool_down"], artifact_data["WindBlessing"]["target_type"]);
}
}; // namespace card

#endif