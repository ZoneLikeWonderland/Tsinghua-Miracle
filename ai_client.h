#ifndef AI_CLIENT_H_
#define AI_CLIENT_H_

#include <vector>
#include <iostream>
#include "gameunit.h"
#include "json.hpp"
using json = nlohmann::json;

// 发送字符串长度
void sendLen(std::string s);

// 发送操作
void sendMsg(int player, int round, std::string operation_type, json operation_parameters);

// 读取信息
json read();

class AiClient
{
public:
    gameunit::Map map;                                                               // 地图
    gameunit::Player players[2];                                                     // 两名玩家的信息
    int round;                                                                       // 当前回合
    int my_camp;                                                                     // 己方阵营
    std::vector<std::string> my_artifacts = {"HolyLight"};                           // 己方神器
    std::vector<std::string> my_creatures = {"Archer", "Swordsman", "VolcanoDragon"}; // 己方生物

    AiClient();

    // 更新游戏局面信息
    void updateGameInfo();

    // 选择初始卡组
    void chooseCards();

    // 结合当前局面信息做出操作
    void play();

    // 初始卡组选择artifacts神器和creatures生物
    void init();

    // 在[x,y,z]位置处召唤一个本方类型为type,星级为level的生物
    void summon(std::string type, int level, int x, int y, int z);

    // 在position位置处召唤一个本方类型为type,星级为level的生物
    void summon(std::string type, int level, std::vector<int> position);

    // 在position位置处召唤一个本方类型为type,星级为level的生物
    void summon(std::string type, int level, std::tuple<int, int, int> position);

    // 将id为mover的生物移动到(x,y,z)位置处
    void move(int mover, int x, int y, int z);

    // 将id为mover的生物移动到position位置处
    void move(int mover, std::vector<int> position);

    // 将id为mover的生物移动到position位置处
    void move(int mover, std::tuple<int, int, int> position);

    // 令id为attacker的生物攻击id为target的生物或神迹
    void attack(int attacker, int target);

    // 对id为target的生物使用artifact神器
    void use(int artifact, int target);

    // 对target位置使用artifact神器
    void use(int artifact, std::vector<int> target);

    // 对target位置使用artifact神器
    void use(int artifact, std::tuple<int, int, int> target);

    // 结束当前回合
    void endRound();

    // 获取camp阵营生物从pos_a位置到pos_b位置的地面距离(不经过地面障碍或敌方地面生物)
    int getDistanceOnGround(gameunit::Pos pos_a, gameunit::Pos pos_b, int camp);

    // 获取camp阵营生物从pos_a位置到pos_b位置的飞行距离(不经过飞行障碍或敌方飞行生物)
    int getDistanceInSky(gameunit::Pos pos_a, gameunit::Pos pos_b, int camp);

    // 对于pos位置,判断其驻扎情况
    // 不是驻扎点返回-2,中立返回-1,否则返回占领该驻扎点的阵营(0或1)
    int checkBarrack(gameunit::Pos pos);

    // 判断attacker生物能否攻击到target生物(只考虑攻击力、攻击范围)
    bool canAttack(gameunit::Unit attacker, gameunit::Unit target);

    // 判断能否对pos位置使用artifact神器(不考虑消耗、冷却)
    bool canUseArtifact(gameunit::Artifact artifact, gameunit::Pos pos, int camp);

    // 判断能否对unit生物使用artifact神器(不考虑消耗、冷却)
    bool canUseArtifact(gameunit::Artifact artifact, gameunit::Unit unit);

    // 获取位置pos上的生物
    // 如果有,返回对应的Unit,否则返回一个id为-1的Unit
    gameunit::Unit getUnitByPos(gameunit::Pos pos, bool flying);

    // 获取id为unit_id的生物
    // 如果有,返回对应的Unit,否则返回一个id为-1的Unit
    gameunit::Unit getUnitById(int unit_id);

    // 获取所有阵营为unit_camp的生物
    // 返回阵营等于unit_camp的Unit列表(没有时返回空列表)
    std::vector<gameunit::Unit> getUnitsByCamp(int unit_camp);

    // 获取所有属于阵营camp的出兵点(初始出兵点+额外出兵点)
    std::vector<gameunit::Pos> getSummonPosByCamp(int camp);
};
#endif