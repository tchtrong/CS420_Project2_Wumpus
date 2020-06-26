#pragma once

#include "HornClause.h"

class Agent_HornClause {
public:
	Agent_HornClause(KB_HornClause& kb_, Board& brd_, const Position& pos_, int limit) :
		kb(kb_),
		brd(brd_),
		pos(pos_),
		limit(limit)
	{}

	void run() {
		brd.print(pos);
		std::cout << "Agent start at:\n" << pos << brd.get_tile(pos);
		std::cout << "Point: " << point << "\n\n";
		while (limit) {
			bool will_shoot = false;
			bool move_to_not_unsafe = false;

			tell_kb(new_percepts(pos));

			//Get surrounding position that is safe
			auto safe_direct = ask_possible_pos(Question::SAFE);

			//Check for safe and unvisited locations
			auto unvisited_direct = ask_possible_pos(Question::UNVISITED);
			std::vector<Position> pos_to_visit;
			for (auto& s_dir : safe_direct) {
				if (std::find(unvisited_direct.begin(), unvisited_direct.end(), s_dir) != unvisited_direct.end()) {
					pos_to_visit.emplace_back(s_dir);
				}
			}

			std::transform(pos_to_visit.begin(), pos_to_visit.end(), pos_to_visit.begin(),
				[&](Position x) {
					return x += pos;
				}
			);

			std::copy_if(
				std::make_move_iterator(pos_to_visit.begin()), std::make_move_iterator(pos_to_visit.end()),
				std::back_inserter(to_visit),
				[&](auto p) {
					return std::find(to_visit.begin(), to_visit.end(), p) == to_visit.end();
				}
			);

			//No safe location available: Kill wumpus if possible to get going
			if (to_visit.empty()) {
				auto wumpus_direct = ask_possible_pos(Question::WUMPUS);
				for (auto& w : wumpus_direct) {
					if (std::find(wumpus_pos.begin(), 
						wumpus_pos.end(), 
						std::make_pair(pos, w)) == wumpus_pos.end()) {
						wumpus_pos.emplace_back(pos, w);
					}
				}
				if (wumpus_pos.size()) will_shoot = true;
			}

			//Cannot find any possible of wumpus nearby, choose to go to maybe safe location
			if (to_visit.empty() && !will_shoot) {
				auto maybe_safe_direct = ask_possible_pos(Question::NOT_UNSAFE);
				for (auto& w : maybe_safe_direct) {
					if (std::find(not_unsafe.begin(), 
						not_unsafe.end(), 
						std::make_pair(pos, w)) == not_unsafe.end() &&
						!brd.get_tile(Position{ pos + w }).is_visited) {
						not_unsafe.emplace_back(pos, w);
					}
				}
				if (not_unsafe.size()) move_to_not_unsafe = true;
			}

			//If cannot make any further decision, break the loop
			if (to_visit.empty() && !will_shoot && !move_to_not_unsafe) {
				std::cout << "Agent cannot figure anything and will decide to quit the dungeon\n";
				break;
			}

			//Act
			bool pick_gold = false;
			if (!will_shoot && !move_to_not_unsafe) {
				move(to_visit.back());
				if (pick_gold = brd.get_tile(pos).is_gold) {
					point += 100;
					brd.get_tile(pos).is_gold = false;
				}
				brd.print(pos);
				std::cout << "Agent move to:\n" << pos;
				if (pick_gold) {
					std::cout << "Agent pick up gold\n";
				}
				std::cout << brd.get_tile(pos);
				to_visit.pop_back();
			}
			else if (will_shoot) {
				move(wumpus_pos.back().first);
				bool kill = shoot(wumpus_pos.back().second);
				if (pick_gold = brd.get_tile(pos).is_gold) {
					point += 100;
					brd.get_tile(pos).is_gold = false;
				}
				brd.print(pos);
				auto direct_ = [&]() {
					if (wumpus_pos.back().second == UP) {
						return "top of the map";
					}
					else if (wumpus_pos.back().second == DOWN) {
						return "bottom of the map";
					}
					else if (wumpus_pos.back().second == LEFT) {
						return "left of the map";
					}
					else if (wumpus_pos.back().second == RIGHT) {
						return "right of the map";
					}
				}();
				std::cout << "Agent cannot find safe location and will guess wumpus location to shoot\n";
				std::cout << "Agent move back to:\n" << pos;
				std::cout << "Agent shoot from: " << pos << "to the " << direct_;
				if (kill) std::cout << "Agent has killed a wumpus";
				if (pick_gold) {
					std::cout << "Agent pick up gold\n";
				}
				std::cout  << pos << brd.get_tile(pos) << "\nPoint: " << point << "\n\n";
				move(pos + wumpus_pos.back().second);
				brd.print(pos);
				std::cout << "Agent move to:\n" << pos << brd.get_tile(pos) << "\nPoint: " << point << "\n\n";;
				wumpus_pos.pop_back();
			}
			else if (move_to_not_unsafe) {
				move(not_unsafe.back().first);
				if (pick_gold = brd.get_tile(pos).is_gold) {
					point += 100;
					brd.get_tile(pos).is_gold = false;
				}
				brd.print(pos);
				std::cout << "Agent cannot find safe location\nor any possible location of wumpus to shoot\nand will choose location that is not unsafe\n";
				std::cout << "Agent move back to:\n" << pos << brd.get_tile(pos) << '\n';
				std::cout << "Point: " << point << "\n\n";
				move(pos + not_unsafe.back().second);
				if (brd.get_tile(pos).is_gold) {
					point += 100;
				}
				brd.print(pos);
				std::cout << "Agent move to:\n" << pos;
				if (pick_gold) {
					std::cout << "Agent pick up gold\n";
				}
				std::cout << brd.get_tile(pos);
				not_unsafe.pop_back();
			}
			std::cout << "Point: " << point << "\n\n";
		}
		if(!limit) std::cout << "Agent has visited the maximum number of rooms.";
	}

private:

	//Create percepts based on current position on the map
	CNFSentence new_percepts(const Position& pos) {
		Tile tile = brd.get_tile(pos);
		CNFSentence res;
		if (tile.is_breeze) {
			res = std::move(res) & get_breeze(pos);
		}
		else {
			res = std::move(res) & ~get_breeze(pos);
		}

		if (tile.is_wumpus) {
			res = std::move(res) & get_wumpus(pos);
		}
		else {
			res = std::move(res) & ~get_wumpus(pos);
		}

		if (tile.is_pit) {
			res = std::move(res) & get_pit(pos);
		}
		else {
			res = std::move(res) & ~get_pit(pos);
		}

		if (tile.is_stench) {
			res = std::move(res) & get_stench(pos);
		}
		else {
			res = std::move(res) & ~get_stench(pos);
		}

		return res;
	}
	CNFSentence get_safe(const Position& pos) {
		return get_literal("~P", pos) & get_literal("~W", pos);
	}
	CNFSentence get_not_unsafe(const Position& pos) {
		return ~get_safe(pos);
	}
	CNFSentence get_wumpus(const Position& pos) {
		return get_literal("W", pos);
	}
	CNFSentence get_pit(const Position& pos) {
		return get_literal("P", pos);
	}
	CNFSentence get_breeze(const Position& pos) {
		return get_literal("B", pos);
	}
	CNFSentence get_stench(const Position& pos) {
		return get_literal("S", pos);
	}

	//Interact with KB
	bool ask_kb(const CNFSentence& percept) {
		return kb.entails(percept);
	}
	void tell_kb(const CNFSentence& percept) {
		kb.add_sentence(CNFSentence(percept));
	}
	std::vector<Position> ask_possible_pos(const Question& question) {
		std::vector<Position> res;
		res.reserve(4);
		for (const auto& pos_ : possible_pos) {
			const auto new_pos = pos_ + pos;
			if (brd.is_valid(new_pos)) {
				switch (question)
				{
				case Question::SAFE:					
					if (ask_kb(get_safe(new_pos))) {
						res.emplace_back(pos_);
					}					
					break;
				case Question::NOT_UNSAFE:
					if (!ask_kb(get_not_unsafe(new_pos))) {
						res.emplace_back(pos_);
					}
					break;
				case Question::WUMPUS:
					if (!ask_kb(~get_wumpus(new_pos))) {
						res.emplace_back(pos_);
					}
					break;
				case Question::UNVISITED:
					if (!brd.get_tile(new_pos).is_visited) {
						res.emplace_back(pos_);
					}
					break;
				default:
					break;
				}
			}
		}
		return res;
	}

	//Actions
	void move(const Position& location) {
		pos = location;
		if (!brd.get_tile(pos).is_visited) --limit;
		brd.mark_visited(pos);
	}
	bool shoot(const Position& direct) {
		point -= 1000;
		return brd.check_shoot(pos, direct);
	}

	//Attributes
	KB_HornClause& kb;
	Board& brd;
	Position pos;
	int limit;
	int point = 0;
	std::vector<Position> to_visit;
	std::vector<std::pair<Position, Position>> not_unsafe;
	std::vector<std::pair<Position, Position>> wumpus_pos;
};