//calculator for hex-grids
#include<vector>
#include<tuple>
#include<map>
#include<algorithm>
#include"gameunit.h"
#include"calculator.h"

namespace calculator {
// left-up, right-up, left-down, right-down, left-right, up, down
std::vector<Point> MAPBORDER() {
    std::vector<Point> border = {
        Point(-7, -8, 15), Point(-8, -7, 15), Point(8, 7, -15), Point(7, 8, -15),
        Point(7, -8, 1), Point(8, -8, 0), Point(8, -7, -1),
        Point(-8, 7, 1), Point(-8, 8, 0), Point(-7, 8,  -1) 
    };
    for (int i = 0; i < 14; i++) {
        border.push_back(Point(-6+i, -9, 15-i));
        border.push_back(Point(9, 6-i, -15+i));
        border.push_back(Point(-9, -6+i, 15-i));
        border.push_back(Point(6-i, 9, -15+i));
    }
    return border;
}

bool contained(const Point& pos, std::vector<Point> list) {
    for (int i = 0; i < list.size(); i++) {
        if (pos == list[i]) return true;
    }
    return false;
}

int cube_distance(Point a, Point b) {
    //return distance between two unit, a/b is position
    int distance = (abs(std::get<0>(a) - std::get<0>(b)) +
                    abs(std::get<1>(a) - std::get<1>(b)) +
                    abs(std::get<2>(a) - std::get<2>(b)))/2;
    return int(distance + 1e-8);
}

Point cube_neighbor(Point pos, int dir) {
    //neighbor of pos, dir ranges from 0 to 5
    int _dir = dir%6;
    Point neighbor;
    if (_dir == 0)   // first neighbor
        neighbor = Point(std::get<0>(pos)+1, std::get<1>(pos), std::get<2>(pos)-1);
    else if (_dir == 1)
        neighbor = Point(std::get<0>(pos)+1, std::get<1>(pos)-1, std::get<2>(pos));
    else if (_dir == 2)
        neighbor = Point(std::get<0>(pos), std::get<1>(pos)-1, std::get<2>(pos)+1);
    else if (_dir == 3)
        neighbor = Point(std::get<0>(pos)-1, std::get<1>(pos), std::get<2>(pos)+1);
    else if (_dir == 4)
        neighbor = Point(std::get<0>(pos)-1, std::get<1>(pos)+1, std::get<2>(pos));
    else
        neighbor = Point(std::get<0>(pos), std::get<1>(pos)+1, std::get<2>(pos)-1);
    return neighbor;
}

Node::Node() {}
Node::Node(Point pos, int G, int H, Node* parent) :
        _pos(pos), _G(G), _H(H), _parent(parent) {}
bool Node::operator< (const Node& b) const {
    return this->_G + this->_H < b._G + b._H;
}

Point Node::pos() const {
    return this->_pos;
}

int Node::G() const {
    return this->_G;
}

int Node::H() const {
    return this->_H;
}

Node* Node::parent() const {
    return this->_parent;
}

int Node::setG(int g) {
    this->_G = g;
    return this->_G;
}

int Node::setH(int h) {
    this->_H = h;
    return this->_H;
}

Node* Node::setParent(Node* p) {
    this->_parent = p;
    return this->_parent;
}

/*def __str__(self):
    return '''
            pos: {},
            G: {},
            F: {},
           '''.format(self.pos, self.G, self.H)*/

bool contained(const Point& pos, std::map<Point, Node> map) {
    for (auto it = map.begin(); it != map.end(); it++) {
        if (pos == it->first) return true;
    }
    return false;
}

std::vector<Point> search_path(Point start, Point to,
    std::vector<Point> obstacles, std::vector<Point> obstructs) {
    //return shortest path
    Point _start = start;
    Point _to = to;
    if (contained(_to, obstacles))
        return std::vector<Point>();
    std::map<Point, Node> opened;
    std::map<Point, Node> closed;
    opened[_start] =  Node(start, 0, cube_distance(start, to));
    while (opened.size()) {
        std::vector<Node> openedValues, closedValues;
        for (auto it = opened.begin(); it != opened.end(); it++)
            openedValues.push_back(it->second);
        std::sort(openedValues.begin(), openedValues.end());
        Node* cur_node = &(opened[openedValues[0].pos()]);
        for (int i = 0; i < 6; i++) {
            Point neighbor = cube_neighbor(cur_node->pos(), i);
            if (!contained(neighbor, closed) && !contained(neighbor, obstacles)) {
                if (contained(neighbor, opened)) {
                    if (cur_node->G()+1 < opened[neighbor].G()) {
                        opened[neighbor].setG(cur_node->G() + 1);
                        opened[neighbor].setParent(cur_node);
                    }
                } else {
                    opened[neighbor] = Node(neighbor, cur_node->G()+1, cube_distance(neighbor, _to), cur_node);
                    if (neighbor == _to) {
                        std::vector<Point> final_path;
                        Node* node = &(opened[neighbor]);
                        while (node != nullptr) {
                            final_path.insert(final_path.begin(), node->pos());
                            node = node->parent();
                        }
                        return final_path;
                    } else if (contained(neighbor, obstructs))
                        opened.erase(neighbor);
                }
            }
        }
        closed[cur_node->pos()] = *cur_node;
        opened.erase(cur_node->pos());
    }
    return std::vector<Point>();
}

std::vector<std::vector<Point>> cube_reachable(Point start, int movement, 
        std::vector<Point> obstacles, std::vector<Point> obstructs) {
    //return reachable position from start Point in steps limited by movement
    std::vector<Point> visited = {};    // positions that have been visited
    visited.push_back(start);
    std::vector<std::vector<Point>> fringes = {};    // list of list of reachable Points in certain steps(subscripts means steps)
    fringes.push_back(std::vector<Point>({start}));

    for (int i = 0; i < movement; i++) {
        fringes.push_back(std::vector<Point>());
        for (int j = 0; j < fringes[i].size(); j++) {
            Point pos = fringes[i][j];
            if (contained(pos, obstructs)) continue;
            for (int k = 0; k < 6; k++) {
                Point neighbor = cube_neighbor(pos, k);
                if (!contained(neighbor, visited) && !contained(neighbor, obstacles)
						&& in_map(neighbor)) {
                    visited.push_back(neighbor);
                    fringes[i+1].push_back(neighbor);
                }
            }
        }
    }
    return fringes;
}

std::vector<Point> get_obstacles_by_unit(gameunit::Unit unit, gameunit::Map _map) {
    /*returns all obstacles for a unit
    unfinished, currently only units have been taken into account*/
    std::vector<Point> obstacles = MAPBORDER();
    std::vector<gameunit::Obstacle> obstacles_on_map = unit.flying ? _map.flying_obstacles : _map.ground_obstacles;
    for (int i = 0; i < obstacles_on_map.size(); i++)
        obstacles.push_back(obstacles_on_map[i].pos);
    std::vector<gameunit::Unit> obstacle_unit = _map.units;
    for (int i = 0; i < obstacle_unit.size(); i++) {
        gameunit::Unit obstacle = obstacle_unit[i];
        if (obstacle.flying == unit.flying) {
            obstacles.push_back(obstacle.pos);
        }
    }
    return obstacles;
}

std::vector<Point> get_obstructs_by_unit(gameunit::Unit unit, gameunit::Map _map) {
    /*returns all obstructs for a unit
    obstruct means a point which the unit can stay but cannot pass*/
    std::vector<Point> obstructs;
    std::vector<gameunit::Unit> obstacle_unit = _map.units;
    for (int i = 0; i < obstacle_unit.size(); i++) {
        gameunit::Unit obstacle = obstacle_unit[i];
        if (obstacle.camp != unit.camp) {
			if (obstacle.flying == unit.flying) {
				for (int j = 0; j < 6; j++) {
					Point cur = cube_neighbor(obstacle.pos, j);
					if (cur != unit.pos)
						obstructs.push_back(cube_neighbor(obstacle.pos, j));
				}
			} else if (obstacle.pos != unit.pos) {
				obstructs.push_back(obstacle.pos);
			}
        }
    }
    return obstructs;
}

//below are public sdk

int distance_between(Point a, Point b) {
    //return Manhattan distance between point a and b
    return cube_distance(a, b);
}

std::vector<Point> path(gameunit::Unit unit, Point dest, gameunit::Map _map) {
    //public sdk for search_path
    //return an empty vector if failed
    std::vector<Point> obstacles = get_obstacles_by_unit(unit, _map);
    std::vector<Point> obstructs = get_obstructs_by_unit(unit, _map);
    std::vector<Point> result = search_path(unit.pos, dest, obstacles, obstructs);
    return result;
}

std::vector<std::vector<Point>> reachable(gameunit::Unit unit, gameunit::Map _map) {
    //public sdk for cube_reachable
    std::vector<Point> obstacles = get_obstacles_by_unit(unit, _map);
    std::vector<Point> obstructs = get_obstructs_by_unit(unit, _map);
    std::vector<std::vector<Point>> result =
        cube_reachable(unit.pos, unit.max_move, obstacles, obstructs);
    return result;
}

std::vector<gameunit::Unit> units_in_range(Point pos, int dist, gameunit::Map _map, int camp,
                                bool flyingIncluded, bool onlandIncluded) {
    /*return list of units whose distance to the pos is less than dist
    default camp = -1, return units of both camp, 0 for the first camp, 1 for the second
    flyingIncluded = True will include flying units,
    onlandIncluded = True will include onland units*/
    std::vector<gameunit::Unit> units;
    std::vector<gameunit::Unit> all_units = _map.units;
    for (int i = 0; i < all_units.size(); i++) {
        gameunit::Unit _unit = all_units[i];
        if (cube_distance(_unit.pos, pos) <= dist &&
           (camp == -1 || camp == _unit.camp) &&
           ((_unit.flying && flyingIncluded) ||
           (!(_unit.flying) && onlandIncluded))) {
               units.push_back(_unit);
        }
    }
    return units;
}

bool in_map(Point pos) {
	if (std::get<0>(pos) > 8 || std::get<0>(pos) < -8 ||
		std::get<1>(pos) > 8 || std::get<1>(pos) < -8 ||
		std::get<2>(pos) > 14 || std::get<2>(pos) < -14) {
		return false;
	} else if (pos == Point(8, -8, 0) || pos == Point(-8, 8 ,0)) {
		return false;
	}
	return true;
}

std::vector<Point> all_pos_in_map() {
	std::vector<Point> all_pos;
	for (int i = -8; i < 9; i++) {
		for (int j = -8; j < 9; j++) {
			Point cur_pos = Point(i, j, -(i+j));
			if (in_map(cur_pos))
				all_pos.push_back(cur_pos);
		}
	}
	return all_pos;
}
}
