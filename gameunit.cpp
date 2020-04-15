#include "gameunit.h"

gameunit::Map::Map()
{
    barracks = {Barrack(-1, {-6, -6, 12}, {{-7, -5, 12}, {-5, -7, 12}, {-5, -6, 11}}),
                Barrack(-1, {6, 6, -12}, {{7, 5, -12}, {5, 7, -12}, {5, 6, -11}}),
                Barrack(-1, {0, -5, 5}, {{0, -4, 4}, {-1, -4, 5}, {-1, -5, 6}}),
                Barrack(-1, {0, 5, -5}, {{0, 4, -4}, {1, 4, -5}, {1, 5, -6}})};
    miracles = {Miracle(0, 30, 30, {-7, 7, 0}, {{-8, 6, 2}, {-7, 6, 1}, {-6, 6, 0}, {-6, 7, -1}, {-6, 8, -2}}, 0),
                Miracle(1, 30, 30, {7, -7, 0}, {{8, -6, -2}, {7, -6, -1}, {6, -6, 0}, {6, -7, 1}, {6, -8, 2}}, 1)};
    obstacles = {Obstacle("Miracle", {-7, 7, 0}, false, false),
                 Obstacle("Miracle", {7, -7, 0}, false, false)};
    std::vector<Pos> ABYSS_POS_LIST = {{0, 0, 0}, {-1, 0, 1}, {0, -1, 1}, {1, -1, 0}, {1, 0, -1}, {0, 1, -1}, {-1, 1, 0}, {-2, -1, 3}, {-1, -2, 3}, {-2, -2, 4}, {-3, -2, 5}, {-4, -4, 8}, {-5, -4, 9}, {-4, -5, 9}, {-5, -5, 10}, {-6, -5, 11}, {1, 2, -3}, {2, 1, -3}, {2, 2, -4}, {3, 2, -5}, {4, 4, -8}, {5, 4, -9}, {4, 5, -9}, {5, 5, -10}, {6, 5, -11},
    {5,8,-13}, {6,7,-13}, {7,6,-13}, {8,5,-13}, {6,8,-14}, {7,7,-14}, {8,6,-14}, {-5,-8,13}, {-6,-7,13}, {-7,-6,13}, {-8,-5,13}, {-6,-8,14}, {-7,-7,14}, {-8,-6,14}};
    for (int i = 0; i < ABYSS_POS_LIST.size(); ++i)
        obstacles.push_back(Obstacle("Abyss", ABYSS_POS_LIST[i], true, false));
    ground_obstacles = obstacles;
}

void gameunit::from_json(const json &j, Unit &u)
{
    j[0].get_to(u.id);
    j[1].get_to(u.camp);
    int t;
    j[2].get_to(t);
    u.type = UNIT_TYPE[t];
    j[3].get_to(u.cost);
    j[4].get_to(u.atk);
    j[5].get_to(u.max_hp);
    j[6].get_to(u.hp);
    j[7].get_to(u.atk_range);
    j[8].get_to(u.max_move);
    j[9].get_to(u.cool_down);
    j[10].get_to(u.pos);
    j[11].get_to(u.level);
    u.flying = j[12]==1;
    u.atk_flying = j[13]==1;
    u.agility = j[14]==1;
    u.holy_shield = j[15]==1;
    u.can_atk = j[16]==1;
    u.can_move = j[17]==1;
}

void gameunit::from_json(const json &j, Artifact &a)
{
    int n,s,t;
    j[0].get_to(a.camp);
    j[1].get_to(n);
    a.name = ARTIFACT_NAME[n];
    a.id = a.camp;
    j[2].get_to(a.cost);
    j[3].get_to(a.max_cool_down);
    j[4].get_to(a.cool_down_time);
    j[5].get_to(s);
    a.state = ARTIFACT_STATE[s];
    j[6].get_to(t);
    a.target_type = ARTIFACT_TARGET[t];
    j[7].get_to(a.last_used_pos);
}

void gameunit::from_json(const json &j, CreatureCapacity &c)
{
    int t;
    j[0].get_to(t);
    c.type = UNIT_TYPE[t];
    j[1].get_to(c.available_count);
    j[2].get_to(c.cool_down_list);
}

void gameunit::from_json(const json &j, Map &m)
{
    j.at("units").get_to(m.units);
    json barracks_camp = j["barracks"];
    for (int i = 0; i < barracks_camp.size(); ++i)
        m.barracks[i].camp = barracks_camp[i];
    json miracle_hp = j["miracles"];
    m.miracles[0].hp = miracle_hp[0];
    m.miracles[1].hp = miracle_hp[1];
}

void gameunit::from_json(const json &j, Player &p)
{
    j[0].get_to(p.artifact);
    j[1].get_to(p.mana);
    j[2].get_to(p.max_mana);
    j[3].get_to(p.creature_capacity);
    j[4].get_to(p.new_summoned_id_list);
}