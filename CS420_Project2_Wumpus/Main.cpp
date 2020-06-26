#include "Agent_HornClause.h"
#include <chrono>
#include <random>

void create_random_map(std::string filepath) {
	//Create random devices
	std::random_device rd;
	std::mt19937 rdg(rd());

	//Random the maximum number that can be for gold, pit and wumpus
	std::uniform_int_distribution rd_max(1, 30);
	int max_gold = rd_max(rdg);
	int max_pit = rd_max(rdg);
	int max_wumpus = rd_max(rdg);

	//Random the number of gold, pit and wumpus
	std::uniform_int_distribution rd_num_gold(1, max_gold);
	int num_gold = rd_num_gold(rdg);
	std::uniform_int_distribution rd_num_pit(0, max_pit);
	int num_pit = rd_num_pit(rdg);
	std::uniform_int_distribution rd_num_wumpus(1, max_wumpus);
	int num_wumpus = rd_num_wumpus(rdg);

	//Range of x and y
	std::uniform_int_distribution xy_range(0, 9);
	std::string map_[10][10];
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			map_[i][j] = '-';
		}
	}

	/*
	On map:
	0: Empty
	1: Gold
	2: Pit
	3: Wumpus
	4: Agent
	*/

	auto random_pos = [&](const int& max, const int& num) {
		for (int i = 0; i < max; ++i) {
			int x_pos = xy_range(rdg);
			int y_pos = xy_range(rdg);
			if (map_[x_pos][y_pos] == "-") {
				map_[x_pos][y_pos] = "";
			}
			if (num == 1) {
				map_[x_pos][y_pos] += "G";
			}
			else if (num == 2) {
				map_[x_pos][y_pos] += "P";
			}
			else if (num == 3) {
				map_[x_pos][y_pos] += "W";
			}
			else if (num == 4) {
				map_[x_pos][y_pos] += "A";
			}
		}};

	//Random position of gold
	random_pos(num_gold, 1);

	//Random position of pit
	random_pos(num_pit, 2);

	//Random position of wumpus
	random_pos(num_wumpus, 3);

	//Random position of agent
	random_pos(1, 4);

	std::ofstream os(filepath);
	for (int j = 9; j >= 0; --j) {
		for (int i = 0; i < 10; ++i) {
			os << map_[i][j];
			if (i < 9) {
				os << ".";
			}
		}
		os << '\n';
	}
	os.close();
}

int main() {
	Board brd("map1.txt");
	KB_HornClause kb(brd.width(), brd.height(), brd.get_agent_starting_point());
	Agent_HornClause agent(kb, brd, brd.get_agent_starting_point(), 100);
	agent.run();
	return 0;
}