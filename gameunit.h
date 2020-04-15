#ifndef GAMEUNIT_H_
#define GAMEUNIT_H_

#include <vector>
#include <string>
#include <tuple>
#include "json.hpp"
using json = nlohmann::json;

namespace gameunit
{

typedef std::tuple<int, int, int> Pos; // 坐标

const std::string UNIT_TYPE[7] = {"Archer", "Swordsman", "BlackBat", "Priest", "VolcanoDragon", "Inferno", "FrostDragon"};
const std::string ARTIFACT_NAME[4] = {"HolyLight", "SalamanderShield", "InfernoFlame", "WindBlessing"};
const std::string ARTIFACT_STATE[3] = {"Ready", "In Use", "Cooling Down"};
const std::string ARTIFACT_TARGET[2] = {"Pos", "Unit"};

struct Unit // 生物
{
    int id;                     // id
    int camp;                   // 阵营
    std::string type;           // 种类
    int cost;                   // 法力消耗
    int atk;                    // 攻击
    int max_hp;                 // 生命上限
    int hp;                     // 当前生命
    std::vector<int> atk_range; // 最小攻击范围 最大攻击范围
    int max_move;               // 行动力
    int cool_down;              // 冷却时间
    Pos pos;                    // 位置
    int level;                  // 等级
    bool flying;                // 是否飞行
    bool atk_flying;            // 是否对空
    bool agility;               // 是否迅捷
    bool holy_shield;           // 有无圣盾
    bool can_atk;               // 能否攻击
    bool can_move;              // 能否移动
};

struct Barrack // 驻扎点
{
    Pos pos;                          // 位置
    int camp;                         // 阵营
    std::vector<Pos> summon_pos_list; // 出兵点位置
    Barrack(int _camp, Pos _pos, std::vector<Pos> _list) : pos(_pos), camp(_camp), summon_pos_list(_list) {}
};

struct Miracle // 神迹
{
    int camp;                         // 阵营
    int max_hp;                       // 最大生命值
    int hp;                           // 当前生命值
    Pos pos;                          // 位置
    std::vector<Pos> summon_pos_list; // 初始出兵点位置
    int id;                           // id
    Miracle(int _camp, int _maxhp, int _hp, Pos _pos, std::vector<Pos> _list, int _id) : camp(_camp), max_hp(_maxhp), hp(_hp), pos(_pos), summon_pos_list(_list), id(_id) {}
};

struct Obstacle
{
    std::string type;  // 种类
    Pos pos;           // 位置
    bool allow_flying; // 是否允许飞行生物通过
    bool allow_ground; // 是否允许地面生物通过
    Obstacle(std::string _type, Pos _pos, bool _f, bool _g) : type(_type), pos(_pos), allow_flying(_f), allow_ground(_g) {}
};

struct Artifact // 神器
{
    int id;                  // id
    std::string name;        // 名字
    int camp;                // 阵营
    int cost;                // 法力消耗
    int max_cool_down;       // 最大冷却时间
    int cool_down_time;      // 当前冷却时间
    std::string state;       // 使用状态
    std::string target_type; // 目标种类
    Pos last_used_pos;       // 上次使用目标位置（上次目标单位的位置）
};

struct CreatureCapacity
{
    std::string type;                // 种类
    int available_count;             // 生物槽容量
    std::vector<int> cool_down_list; // 冷却时间
};

struct Map // 地图
{
    std::vector<Unit> units;
    std::vector<Barrack> barracks;
    std::vector<Miracle> miracles;
    std::vector<Obstacle> obstacles;
    std::vector<Obstacle> flying_obstacles;
    std::vector<Obstacle> ground_obstacles;
    Map();
};

struct Player // 玩家
{
    int camp;                       // 阵营
    std::vector<Artifact> artifact; // 神器
    int mana;                       // 当前法力值
    int max_mana;                   // 最大法力值
    std::vector<CreatureCapacity> creature_capacity;
    std::vector<int> new_summoned_id_list; // 最新召唤的生物id
};

void from_json(const json &j, Unit &u);

void from_json(const json &j, Artifact &a);

void from_json(const json &j, CreatureCapacity &c);

void from_json(const json &j, Map &m);

void from_json(const json &j, Player &p);

} // namespace gameunit

#endif