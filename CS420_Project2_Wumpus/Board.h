#pragma once

#include "KB.h"
#include <fstream>

struct Tile {
	bool is_gold = false;
	bool is_wumpus = false;
	bool is_pit = false;
	bool is_stench = false;
	bool is_breeze = false;
	bool is_visited = false;
	friend std::ostream& operator<<(std::ostream& os, const Tile& pos) {
		os	<< "Gold:   "	<< print(pos.is_gold)
			<< "Wumpus: "	<< print(pos.is_wumpus)
			<< "Pit:    "	<< print(pos.is_pit)
			<< "Stench: "	<< print(pos.is_stench)
			<< "Breeze: "	<< print(pos.is_breeze);
		return os;
	}
private:
	static std::string print(bool fact) {
		if (fact) return "Yes\n";
		else return "No\n";
	}
};

struct Board {
public:
	Board(std::string filepath) {
		std::ifstream in(filepath);
		if (in) {
			std::string temp;
			std::getline(in, temp);
			int width_height = std::stoi(temp);
			map_ = std::vector<std::vector<Tile>>(width_height, std::vector<Tile>(width_height));
			int i = 0;
			for (int j = width_height - 1; j >= 0; --j) {
				for (int i = 0; i < width_height; ++i) {
					if (i == width_height - 1) {
						std::getline(in, temp);
					}
					else {
						std::getline(in, temp, '.');
					}
					if (temp.find('S') < temp.size()) {
						map_[i][j].is_stench = true;
					}
					if (temp.find('B') < temp.size()) {
						map_[i][j].is_breeze = true;
					}
					if (temp.find('G') < temp.size()) {
						map_[i][j].is_gold = true;
					}
					if (temp.find('W') < temp.size()) {
						map_[i][j].is_wumpus = true;
					}
					if (temp.find('P') < temp.size()) {
						map_[i][j].is_pit = true;
					}
					if (temp.find('A') < temp.size()) {
						agent_starting_point_ = { i, j };
						map_[i][j].is_visited = true;
					}
				}
			}
			in.close();
		}
		else {
			std::cout << "Wrong file path\n";
		}
	}

	bool is_visited(const Position& pos) const {
		return map_[pos.x][pos.y].is_visited;
	}
	bool is_valid(const Position& pos) const {
		auto [x, y] = pos;
		return x >= 0 && y >= 0 && x < width() && y < height();
	}
	Tile& get_tile(const Position& pos) {
		return map_[pos.x][pos.y];
	}
	size_t width()const {
		return map_.capacity();
	}
	size_t height()const {
		return map_[0].capacity();
	}
	Position get_agent_starting_point() const {
		return agent_starting_point_;
	}

	void mark_visited(const Position& pos) {
		map_[pos.x][pos.y].is_visited = true;
	}
	bool check_shoot(const Position& pos, const Position& direct) {
		auto [x, y] = pos;
		if (direct == UP) {
			while (y < height()) {
				if (map_[x][y].is_wumpus) {
					map_[x][y].is_wumpus = false;
					return true;
				}
				++y;
			}
		}
		else if (direct == RIGHT) {
			while (x < width()) {
				if (map_[x][y].is_wumpus) {
					map_[x][y].is_wumpus = false;
					return true;
				}
				++x;
			}
		}
		else if (direct == DOWN) {
			while (y >= 0) {
				if (map_[x][y].is_wumpus) {
					map_[x][y].is_wumpus = false;
					return true;
				}
				--y;
			}
		}
		else if (direct == LEFT) {
			while (x >= 0) {
				if (map_[x][y].is_wumpus) {
					map_[x][y].is_wumpus = false;
					return true;
				}
				--x;
			}
		}
		return false;
	}

	void print(const Position& agent_pos)const {
		for (int j = int(map_[0].size()) - 1; j >= 0; --j) {
			for (int i = 0; i < map_.size(); ++i) {
				bool nothing = false;
				bool printA = false;
				if (map_[i][j].is_breeze) {
					std::cout << 'B';
					nothing = true;
				}
				if (map_[i][j].is_gold) {
					std::cout << 'G';
					nothing = true;
				}
				if (map_[i][j].is_pit) {
					std::cout << 'P';
					nothing = true;
				}
				if (map_[i][j].is_stench) {
					std::cout << 'S';
					nothing = true;
				}
				if (map_[i][j].is_wumpus) {
					std::cout << 'W';
					nothing = true;
				}
				if (agent_pos == Position{ i,j }) {
					std::cout << 'A';
					nothing = true;
					printA = true;
				}
				if (map_[i][j].is_visited && !printA) {
					std::cout << 'x';
					nothing = true;
				}
				if (!nothing) {
					std::cout << "--";
				}
				std::cout << "\t";
			}
			std::cout << '\n';
		}
		std::cout << '\n';
	}

private:
	Position agent_starting_point_;
	std::vector<std::vector<Tile>> map_;
};