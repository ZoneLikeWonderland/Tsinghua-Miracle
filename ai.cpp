#include "ai_client.h"
#include "calculator.h"
#include "card.hpp"
#include "gameunit.h"

#include <fstream>
#include <queue>
#include <random>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <unordered_set>

using gameunit::Pos;
using gameunit::Unit;

using calculator::all_pos_in_map;
using calculator::cube_distance;
using calculator::cube_reachable;
using calculator::reachable;
using calculator::units_in_range;

using card::CARD_DICT;

using std::default_random_engine;
using std::get;
using std::make_tuple;
using std::map;
using std::string;
using std::uniform_int_distribution;
using std::vector;

double simlog(double dist) {
    if (dist <= 1) return 0;
    if (dist <= 2) return 1;
    if (dist <= 3) return 2;
    if (dist <= 4) return 2.5;
    if (dist <= 5)
        return 3;
    else
        return 3.5;
}

double eps() {
    return 0;
    static bool init = true;
    if (init) {
        srand(time(NULL));
        init = false;
    }
    return double(rand() - RAND_MAX / 2) / double(RAND_MAX) * 0.1f;
}

struct poshash {
    size_t operator()(const Pos &pos) const {
        return (std::get<0>(pos) << 20) + (std::get<1>(pos) << 10) +
               std::get<2>(pos);
    }
};

bool inrange(Unit atker, Unit victim) {
    if (cube_distance(atker.pos, victim.pos) >= atker.atk_range[0] &&
        cube_distance(atker.pos, victim.pos) <= atker.atk_range[1] &&
        (!victim.flying || victim.flying && atker.atk_flying))
        return true;
    return false;
}

std::vector<Pos> around(Pos pos, int radius) {
    std::vector<Pos> start = {
        make_tuple(get<0>(pos), get<1>(pos) + radius, get<2>(pos) - radius),
        make_tuple(get<0>(pos), get<1>(pos) - radius, get<2>(pos) + radius),
        make_tuple(get<0>(pos) + radius, get<1>(pos), get<2>(pos) - radius),
        make_tuple(get<0>(pos) - radius, get<1>(pos), get<2>(pos) + radius),
        make_tuple(get<0>(pos) + radius, get<1>(pos) - radius, get<2>(pos)),
        make_tuple(get<0>(pos) - radius, get<1>(pos) + radius, get<2>(pos))};
    return start;
}

class AI : public AiClient {
  public:
    Pos miracle_pos;

    Pos enemy_pos;

    Pos target_barrack;

    Pos posShift(Pos pos, string direct);

    std::unordered_set<Pos, poshash> LandObs;
    std::unordered_set<Pos, poshash> Map;
    std::unordered_set<Pos, poshash> Highway;
    std::unordered_map<Pos, double, poshash> LandPosCost;
    std::unordered_map<Pos, double, poshash> Safety;
    std::unordered_map<Pos, double, poshash> NumOfAlly3;

  public:
    //选择初始卡组
    void chooseCards(); //(根据初始阵营)选择初始卡组

    void play(); //玩家需要编写的ai操作函数

    void battle(); //处理生物的战斗

    void march(); //处理生物的移动
};

void AI::chooseCards() {
    // (根据初始阵营)选择初始卡组

    /*
     * artifacts和creatures可以修改
     * 【进阶】在选择卡牌时，就已经知道了自己的所在阵营和先后手，因此可以在此处根据先后手的不同设置不同的卡组和神器
     */
    my_artifacts = {"HolyLight"};
    my_creatures = {"Archer", "Swordsman", "Priest"};
    init();
}

Pos invalid(99999, 99999, 99999);
void printpos(Pos pos) {
    std::cerr << "(" << std::get<0>(pos) << "," << std::get<1>(pos) << ","
              << std::get<2>(pos) << ")"
              << "\n";
}

bool IsSummonStupid(Pos pos, card::Creature cr, gameunit::Unit enemy) {
    auto dist = cube_distance(pos, enemy.pos);
    if (enemy.atk_range[0] <= dist && dist <= enemy.atk_range[1]) {
        if (dist < cr.min_atk_range || dist > cr.max_atk_range) return true;
    }
    return false;
}

int mana;
// vector<card::Creature> summon_list;
::map<string, int> available_count;
bool call(card::Creature todo) {
    if (available_count[todo.type] > 0 && mana >= todo.cost) {
        // summon_list.push_back(todo);
        mana -= todo.cost;
        available_count[todo.type] -= 1;
        return true;
    }
    return false;
}

void AI::play() {
    //玩家需要编写的ai操作函数

    /*
    本AI采用这样的策略：
    在首回合进行初期设置、在神迹优势路侧前方的出兵点召唤一个1星弓箭手
    接下来的每回合，首先尽可能使用神器，接着执行生物的战斗，然后对于没有进行战斗的生物，执行移动，最后进行召唤
    在费用较低时尽可能召唤星级为1的兵，优先度剑士>弓箭手>火山龙
    【进阶】可以对局面进行评估，优化神器的使用时机、调整每个生物行动的顺序、调整召唤的位置和生物种类、星级等
    */
    if (round == 0 || round == 1) {
        //先确定自己的基地、对方的基地
        miracle_pos = map.miracles[my_camp].pos;
        enemy_pos = map.miracles[my_camp ^ 1].pos;
        // 在正中心偏右召唤一个弓箭手，用来抢占驻扎点
        summon("Archer", 1, posShift(miracle_pos, "SF"));
        // summon("Swordsman", 1, posShift(miracle_pos, "SF"));

        for (auto &x : map.ground_obstacles) LandObs.insert(x.pos);
        for (auto &x : all_pos_in_map()) Map.insert(x), Safety[x] = 0;
        for (auto &x : all_pos_in_map()) Map.insert(x), NumOfAlly3[x] = 0;
        Highway = {Pos(-2, 0, 2), Pos(2, 0, -2), Pos(-1, -1, 2),
                   Pos(1, 1, -2), Pos(0, -2, 2), Pos(0, 2, -2)};

        {
            // std::unordered_set<Pos, poshash> visit;
            std::queue<Pos> q;
            q.push(enemy_pos);
            LandPosCost[enemy_pos] = 0;
            while (!q.empty()) {
                auto x = q.front();
                q.pop();
                // visit.insert(x);
                for (auto &neighbour : around(x, 1)) {
                    if (LandObs.count(neighbour)) continue;
                    if (!Map.count(neighbour)) continue;
                    if (!LandPosCost.count(neighbour) ||
                        LandPosCost[neighbour] > LandPosCost[x] + 1) {
                        LandPosCost[neighbour] = LandPosCost[x] + 1;
                        q.push(neighbour);
                    }
                }
            }
        }
    }
    //设定目标驻扎点为最近的驻扎点

    auto mybarrack = miracle_pos;
    for (const auto &barrack : map.barracks)
        if (barrack.camp == my_camp) mybarrack = barrack.pos;
    target_barrack = invalid;
    if (mybarrack == miracle_pos ||
        round > 200) { //确定离自己基地最近的驻扎点的位置
        for (const auto &barrack : map.barracks) {
            // std::cerr << barrack.camp << " ";
            if (barrack.camp == -1 &&
                cube_distance(mybarrack, barrack.pos) <
                    cube_distance(mybarrack, target_barrack))
                target_barrack = barrack.pos;
        }
    }
    // printpos(target_barrack);
    // std::cerr << round << "\n";

    //神器能用就用，选择覆盖单位数最多的地点
    if (players[my_camp].mana >= 6 &&
        players[my_camp].artifact[0].state == "Ready") {
        auto pos_list = all_pos_in_map();
        auto best_pos = pos_list[0];
        int max_benefit = 0;
        for (auto pos : pos_list) {
            auto unit_list = units_in_range(pos, 2, map, my_camp);
            if (unit_list.size() > max_benefit) {
                best_pos = pos;
                max_benefit = unit_list.size();
            }
        }
        use(players[my_camp].artifact[0].id, best_pos);
    }

    //之后先战斗，再移动
    battle();

    for (auto &x : Safety) x.second = 0;
    for (auto &enemy : getUnitsByCamp(my_camp ^ 1)) {
        for (auto &x : Safety)
            x.second += simlog(cube_distance(x.first, enemy.pos));
    }
    auto threat = units_in_range(miracle_pos, 5, map, my_camp ^ 1);
    if (threat.size() > 0) {
        for (auto &enemy : threat) {
            for (auto &x : Safety)
                x.second += 2 * simlog(cube_distance(x.first, enemy.pos));
        }
    }

    march();

    //最后进行召唤
    //将所有本方出兵点按照到对方基地的距离排序，从近到远出兵
    auto summon_pos_list = getSummonPosByCamp(my_camp);
    sort(summon_pos_list.begin(), summon_pos_list.end(),
         [this](Pos _pos1, Pos _pos2) {
             return this->Safety[_pos1] < this->Safety[_pos2];
         });
    vector<Pos> available_summon_pos_list;
    for (auto pos : summon_pos_list) {
        auto unit_on_pos_ground = getUnitByPos(pos, false);
        if (unit_on_pos_ground.id == -1)
            available_summon_pos_list.push_back(pos);
    }

    //统计各个生物的可用数量，在假设出兵点无限的情况下，按照1个剑士、1个弓箭手、1个火山龙的顺序召唤
    mana = players[my_camp].mana;
    auto deck = players[my_camp].creature_capacity;
    for (const auto &card_unit : deck)
        available_count[card_unit.type] = card_unit.available_count;

    threat = units_in_range(miracle_pos, 5, map, my_camp ^ 1);
    vector<card::Creature> priority = {
        CARD_DICT.at("Priest")[3],    CARD_DICT.at("Archer")[3],
        CARD_DICT.at("Archer")[2],    CARD_DICT.at("Swordsman")[3],
        CARD_DICT.at("Priest")[2],    CARD_DICT.at("Swordsman")[2],
        CARD_DICT.at("Swordsman")[1],
    };

    // int i = 0;
    for (auto pos : available_summon_pos_list) {
        if (mana <= 1) break;
        for (auto &card : priority) {
            bool bad = false;
            for (auto &enemy : threat) {
                bad |= IsSummonStupid(pos, card, enemy);
            }
            if (!bad) {
                if (call(card)) {
                    summon(card.type, card.level, pos);
                    break;
                }
            }
        }
    }

    endRound();
}

Pos AI::posShift(Pos pos, string direct) {
    /*
     * 对于给定位置，给出按照自己的视角（神迹在最下方）的某个方向移动一步后的位置
     * 本段代码可以自由取用
     * @param pos:  (x, y, z)
     * @param direct: 一个str，含2个字符，意义见注释
     * @return: 移动后的位置 (x', y', z')
     */
    transform(direct.begin(), direct.end(), direct.begin(), ::toupper);
    if (my_camp == 0) {
        if (direct == "FF") //正前方
            return make_tuple(get<0>(pos) + 1, get<1>(pos) - 1, get<2>(pos));
        else if (direct == "SF") //优势路前方（自身视角右侧为优势路）
            return make_tuple(get<0>(pos) + 1, get<1>(pos), get<2>(pos) - 1);
        else if (direct == "IF") //劣势路前方
            return make_tuple(get<0>(pos), get<1>(pos) + 1, get<2>(pos) - 1);
        else if (direct == "BB") //正后方
            return make_tuple(get<0>(pos) - 1, get<1>(pos) + 1, get<2>(pos));
        else if (direct == "SB") //优势路后方
            return make_tuple(get<0>(pos), get<1>(pos) - 1, get<2>(pos) + 1);
        else if (direct == "IB") //劣势路后方
            return make_tuple(get<0>(pos) - 1, get<1>(pos), get<2>(pos) + 1);
    } else {
        if (direct == "FF") //正前方
            return make_tuple(get<0>(pos) - 1, get<1>(pos) + 1, get<2>(pos));
        else if (direct == "SF") //优势路前方（自身视角右侧为优势路）
            return make_tuple(get<0>(pos) - 1, get<1>(pos), get<2>(pos) + 1);
        else if (direct == "IF") //劣势路前方
            return make_tuple(get<0>(pos), get<1>(pos) - 1, get<2>(pos) + 1);
        else if (direct == "BB") //正后方
            return make_tuple(get<0>(pos) + 1, get<1>(pos) - 1, get<2>(pos));
        else if (direct == "SB") //优势路后方
            return make_tuple(get<0>(pos), get<1>(pos) + 1, get<2>(pos) - 1);
        else if (direct == "IB") //劣势路后方
            return make_tuple(get<0>(pos) + 1, get<1>(pos), get<2>(pos) - 1);
    }
    return invalid;
}

void AI::battle() {
    //处理生物的战斗

    /*
     * 基本思路，行动顺序:
     * 火山龙：攻击高>低 （大AOE输出），随机攻击
     * 剑士：攻击低>高 打消耗，优先打攻击力低的
     * 弓箭手：攻击高>低
     * 优先打不能反击的攻击力最高的，其次打能反击的攻击力最低的
     * 对单位的战斗完成后，对神迹进行输出
     * 【进阶】对战斗范围内敌方目标的价值进行评估，通过一些匹配算法决定最优的战斗方式
     * 例如占领着驻扎点的敌方生物具有极高的价值，优先摧毁可以使敌方下回合损失很多可用出兵点
     * 一些生物不攻击而移动，或完全不动，可能能带来更大的威慑力，而赢得更多优势
     */

    auto ally_list = getUnitsByCamp(my_camp);

    //自定义排列顺序
    auto cmp = [](const Unit &unit1, const Unit &unit2) {
        if (unit1.can_atk != unit2.can_atk) //首先要能动
            return unit2.can_atk < unit1.can_atk;
        else if (unit1.type != unit2.type) { //火山龙>剑士>弓箭手
            auto type_id_gen = [](const string &type_name) {
                if (type_name == "Swordsman")
                    return 0;
                else if (type_name == "Archer")
                    return 1;
                else
                    return 2;
            };
            return (type_id_gen(unit1.type) < type_id_gen(unit2.type));
        } else if (unit1.type == "Priest" or unit1.type == "Archer")
            return unit2.atk < unit1.atk;
        else
            return unit1.atk < unit2.atk;
    };
    //最!后攻击神迹
    ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(), cmp);
    for (auto ally : ally_list) {
        if (!ally.can_atk) break;
        int dis = cube_distance(ally.pos, enemy_pos);
        if (ally.atk_range[0] <= dis && dis <= ally.atk_range[1])
            attack(ally.id, my_camp ^ 1);
    }
    //按顺序排列好单位，依次攻击
    ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(), cmp);
    for (const auto &ally : ally_list) {
        if (!ally.can_atk) break;
        auto enemy_list = getUnitsByCamp(my_camp ^ 1);
        vector<Unit> target_list;
        for (const auto &enemy : enemy_list)
            if (AiClient::canAttack(ally, enemy))
                if (ally.type == "Priest") {
                    if (!inrange(enemy, ally)) target_list.push_back(enemy);
                } else
                    target_list.push_back(enemy);
        if (target_list.empty()) continue;
        if (ally.type == "Priest") {
            default_random_engine g(static_cast<unsigned int>(time(nullptr)));
            int tar = uniform_int_distribution<>(0, target_list.size() - 1)(g);
            attack(ally.id, target_list[tar].id);
        } else if (ally.type == "Swordsman") {
            nth_element(enemy_list.begin(), enemy_list.begin(),
                        enemy_list.end(),
                        [](const Unit &_enemy1, const Unit &_enemy2) {
                            return _enemy1.atk < _enemy2.atk;
                        });
            attack(ally.id, target_list[0].id);
        } else if (ally.type == "Archer") {
            sort(enemy_list.begin(), enemy_list.end(),
                 [](const Unit &_enemy1, const Unit &_enemy2) {
                     return _enemy1.atk > _enemy2.atk;
                 });
            bool suc = false;
            for (const auto &enemy : target_list)
                if (!canAttack(enemy, ally)) {
                    attack(ally.id, enemy.id);
                    suc = true;
                    break;
                }
            if (suc) continue;
            nth_element(enemy_list.begin(), enemy_list.begin(),
                        enemy_list.end(),
                        [](const Unit &_enemy1, const Unit &_enemy2) {
                            return _enemy1.atk < _enemy2.atk;
                        });
            attack(ally.id, target_list[0].id);
        }
    }
    //最后攻击神迹
    ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(), cmp);
    for (auto ally : ally_list) {
        if (!ally.can_atk) break;
        int dis = cube_distance(ally.pos, enemy_pos);
        if (ally.atk_range[0] <= dis && dis <= ally.atk_range[1])
            attack(ally.id, my_camp ^ 1);
    }
}

std::unordered_set<Pos, poshash> circle(Pos pos, int radius) {
    if (radius == 0) return std::unordered_set<Pos, poshash>({pos});
    std::unordered_set<Pos, poshash> ret;
    std::vector<Pos> start = around(pos, radius);
    for (int i = 1; i <= radius; i++) {
        for (auto &x : start) ret.insert(x);
        start[0] = make_tuple(get<0>(start[0]) - 1, get<1>(start[0]),
                              get<2>(start[0]) + 1);
        start[1] = make_tuple(get<0>(start[1]) + 1, get<1>(start[1]),
                              get<2>(start[1]) - 1);
        start[2] = make_tuple(get<0>(start[2]) - 1, get<1>(start[2]) + 1,
                              get<2>(start[2]));
        start[3] = make_tuple(get<0>(start[3]) + 1, get<1>(start[3]) - 1,
                              get<2>(start[3]));
        start[4] = make_tuple(get<0>(start[4]), get<1>(start[4]) + 1,
                              get<2>(start[4]) - 1);
        start[5] = make_tuple(get<0>(start[5]), get<1>(start[5]) - 1,
                              get<2>(start[5]) + 1);
    }
    return ret;
}
std::unordered_set<Pos, poshash> getdanger(Pos pos, int l, int r) {
    std::unordered_set<Pos, poshash> ret;
    for (int i = l; i <= r; i++) ret.merge(circle(pos, i));
    return ret;
}

void AI::march() {
    //处理生物的移动

    /*
     * 先动所有剑士，尽可能向敌方神迹移动
     * 若目标驻扎点上没有地面单位，则让弓箭手向目标驻扎点移动，否则尽可能向敌方神迹移动
     * 然后若目标驻扎点上没有地面单位，则让火山之龙向目标驻扎点移动，否则尽可能向敌方神迹移动
     * 【进阶】一味向敌方神迹移动并不一定是个好主意
     * 在移动的时候可以考虑一下避开敌方生物攻击范围实现、为己方强力生物让路、堵住敌方出兵点等策略
     * 如果采用其他生物组合，可以考虑抢占更多驻扎点
     */
    auto enemy_list = getUnitsByCamp(my_camp ^ 1);

    int cap_id = -1;

    auto ally_list = getUnitsByCamp(my_camp);
    sort(ally_list.begin(), ally_list.end(),
         [](const Unit &_unit1, const Unit &_unit2) {
             auto type_id_gen = [](const string &type_name) {
                 if (type_name == "Swordsman")
                     return 0;
                 else if (type_name == "Archer")
                     return 1;
                 else
                     return 2;
             };
             return type_id_gen(_unit1.type) < type_id_gen(_unit2.type);
         });
    int empty_slot = 3;
    for (const auto &ally : ally_list) {
        if (cube_distance(ally.pos, enemy_pos) <= 1) empty_slot--;
    }
    bool done = false;
    for (const auto &ally : ally_list) {
        if (!ally.can_move) continue;
        if (ally.type == "Priest" && !done) {
            for (auto &x : NumOfAlly3) x.second = 0;
            for (auto &ally : getUnitsByCamp(my_camp)) {
                for (auto &x : NumOfAlly3)
                    x.second += cube_distance(x.first, ally.pos) <= 3 ? 1 : 0;
            }
            done = true;
        }
        {
            std::unordered_set<Pos, poshash> dangerzone;
            for (auto &enemy : enemy_list) {
                for (int i = enemy.atk_range[0]; i <= enemy.atk_range[1]; i++) {
                    if (ally.atk_range[0] <= i && i <= ally.atk_range[1] &&
                        (!enemy.flying || enemy.flying && ally.atk_flying))
                        if (ally.type == "Priest") {
                            if (!inrange(enemy, ally)) continue;
                        } else
                            continue;
                    dangerzone.merge(circle(enemy.pos, i));
                }
            }
            //获取所有可到达的位置
            auto reach_pos_with_dis = reachable(ally, map);
            //压平
            vector<std::pair<Pos, double>> reach_pos_list;
            for (const auto &reach_pos : reach_pos_with_dis) {
                for (auto pos : reach_pos) {
                    double cost = LandPosCost[pos];
                    if (ally.type == "Swordsman") cost = Safety[pos];
                    if (ally.type == "Priest") cost = -NumOfAlly3[pos];
                    if (cost == 0) continue;
                    if (cost < ally.atk_range[0]) cost += 3;
                    if (dangerzone.count(pos)) cost += 10;
                    if (ally.type == "Archer" && Highway.count(pos)) cost += 2;
                    reach_pos_list.push_back(std::make_pair(pos, cost + eps()));
                }
            }
            if (reach_pos_list.empty()) continue;

            //优先走到未被占领的兵营，否则走到
            if (ally.type == "Archer" && target_barrack != invalid &&
                cap_id == -1) {
                nth_element(
                    reach_pos_list.begin(), reach_pos_list.begin(),
                    reach_pos_list.end(),
                    [this](std::pair<Pos, double> _pos1,
                           std::pair<Pos, double> _pos2) {
                        return cube_distance(_pos1.first, target_barrack) <
                               cube_distance(_pos2.first, target_barrack);
                    });
                move(ally.id, reach_pos_list[0].first);
                cap_id = ally.id;
            } else {
                nth_element(reach_pos_list.begin(), reach_pos_list.begin(),
                            reach_pos_list.end(),
                            [this](std::pair<Pos, double> _pos1,
                                   std::pair<Pos, double> _pos2) {
                                return _pos1.second < _pos2.second;
                            });
                auto backup = ally.pos;
                move(ally.id, reach_pos_list[0].first);
                // for (auto &pos : circle(reach_pos_list[0].first, 1))
                //     if (Safety.count(pos)) Safety[pos] += 0.1;
                // std::cerr << round << ":" << ally.id << ":";
                // std::cerr << reach_pos_list[0].second << ","
                //           << reach_pos_list[1].second << "\n";
                // printpos(reach_pos_list[0].first);
            }
        }
    }
}

int main() {
    std::ifstream datafile("Data.json");
    json all_data;
    datafile >> all_data;
    card::get_data_from_json(all_data);

    AI player_ai;
    player_ai.chooseCards();
    while (true) {
        player_ai.updateGameInfo();
        player_ai.play();
    }
    return 0;
}
