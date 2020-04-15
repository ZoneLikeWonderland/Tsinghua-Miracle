#ifndef CALCULATOR
#define CALCULATOR
//calculator for hex-grids
#include<vector>
#include<tuple>
#include<map>
#include<algorithm>
#include"gameunit.h"

namespace calculator {
typedef std::tuple<int, int, int> Point;
// left-up, right-up, left-down, right-down, left-right, up, down
std::vector<Point> MAPBORDER();

bool contained(const Point& pos, std::vector<Point> list);

//return distance between two unit, a/b is position
int cube_distance(Point a, Point b);

//neighbor of pos, dir ranges from 0 to 5
Point cube_neighbor(Point pos, int dir);

class Node {

private:
    Point _pos;
    int _G, _H;
    Node* _parent;

public:
	Node();
    Node(Point pos, int G, int H, Node* parent=nullptr);
    bool operator< (const Node& b) const;
    Point pos() const;
    int G() const;
    int H() const;
    Node* parent() const;
    int setG(int g);
    int setH(int h);
    Node* setParent(Node* p);
    /*def __str__(self):
        return '''
                pos: {},
                G: {},
                F: {},
               '''.format(self.pos, self.G, self.H)*/
};

bool contained(const Point& pos, std::map<Point, Node> map);

//return shortest path
std::vector<Point> search_path(Point start, Point to,
    std::vector<Point> obstacles={}, std::vector<Point> obstructs={});

//return reachable position from start Point in steps limited by movement
std::vector<std::vector<Point>> cube_reachable(Point start, int movement, 
        std::vector<Point> obstacles={}, std::vector<Point> obstructs={});

//returns all obstacles for a unit
std::vector<Point> get_obstacles_by_unit(gameunit::Unit unit, gameunit::Map _map);

/*returns all obstructs for a unit
obstruct means a point which the unit can stay but cannot pass*/
std::vector<Point> get_obstructs_by_unit(gameunit::Unit unit, gameunit::Map _map);

//below are public sdk

//return Manhattan distance between point a and b
int distance_between(Point a, Point b);

//public sdk for search_path
//return an empty vector if failed
std::vector<Point> path(gameunit::Unit unit, Point dest, gameunit::Map _map);

//public sdk for cube_reachable
std::vector<std::vector<Point>> reachable(gameunit::Unit unit, gameunit::Map _map);

/*return list of units whose distance to the pos is less than dist
default camp = -1, return units of both camp, 0 for the first camp, 1 for the second
flyingIncluded = True will include flying units,
onlandIncluded = True will include onland units*/
std::vector<gameunit::Unit> units_in_range(Point pos, int dist, gameunit::Map _map, int camp=-1,
                                bool flyingIncluded=true, bool onlandIncluded=true);

bool in_map(Point pos);

std::vector<Point> all_pos_in_map();

}
#endif
