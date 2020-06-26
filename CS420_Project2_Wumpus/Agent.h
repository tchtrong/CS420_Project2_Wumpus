#pragma once

#include "Board.h"

enum class Question {
	SAFE,
	NOT_UNSAFE,
	WUMPUS,
	UNVISITED,
};

enum class Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

Direction& operator++(Direction& d) {
	switch (d)
	{
	case Direction::UP:
		d = Direction::RIGHT;
		break;
	case Direction::DOWN:
		d = Direction::LEFT;
		break;
	case Direction::LEFT:
		d = Direction::UP;
		break;
	case Direction::RIGHT:
		d = Direction::DOWN;
		break;
	default:
		break;
	}
	return d;
}

Direction& operator--(Direction& d) {
	switch (d)
	{
	case Direction::UP:
		d = Direction::LEFT;
		break;
	case Direction::DOWN:
		d = Direction::RIGHT;
		break;
	case Direction::LEFT:
		d = Direction::DOWN;
		break;
	case Direction::RIGHT:
		d = Direction::UP;
		break;
	default:
		break;
	}
	return d;
}

class Agent {
public:
	Agent(KB& kb_, Board& brd_, const Position& pos_, int limit):
		kb(kb_),
		brd(brd_),
		pos(pos_),
		limit(limit)
	{}

	void run() {
		std::cout << "Agent start at:\n" << pos << brd.get_tile(pos);
		while (limit) {
			tell_kb(new_percepts(pos));

			auto cur = brd.get_tile(pos);
			if (cur.is_pit || cur.is_wumpus) {
				std::cout << "Agent has died, game over\n";
				point -= 10000;
				std::cout << "Point: " << point;
				break;
			}
			
			//Check for gold
			if (cur.is_gold) {
				std::cout << "Agent pick up a gold and receive 100 points\n";
				point += 100;
			}
			std::cout << "Point: " << point << "\n\n";

			//Get surrounding position that is safe
			auto safe_direct = ask_possible_pos(Question::SAFE);

			//Check for safe and unvisited locations
			if (to_visit.empty()) {
				auto unvisited_direct = ask_possible_pos(Question::UNVISITED);
				std::vector<Position> pos_to_visit;
				for (auto& s_dir : safe_direct) {
					if (std::find(unvisited_direct.begin(), unvisited_direct.end(), s_dir)!=unvisited_direct.end()) {
						pos_to_visit.emplace_back(s_dir);
					}
				}
				size_t pos_added = pos_to_visit.size();
				std::copy_if(
					std::make_move_iterator(pos_to_visit.begin()), std::make_move_iterator(pos_to_visit.end()),
					std::back_inserter(to_visit),
					[&](auto p) {
						return std::find(to_visit.begin(), to_visit.end(), p + pos) == to_visit.end();
					}
					);
				std::transform(to_visit.begin(), to_visit.end(), to_visit.begin(), 
					[&](Position x) {
						return x += pos;
					}
				);
			}

			//No safe location available: Kill wumpus if possible to get going
			if (to_visit.empty()) {
				auto wumpus_direct = ask_possible_pos(Question::WUMPUS);
				if (wumpus_direct.size()) {
					to_visit.emplace_back(std::move(wumpus_direct[0]) + pos);
				}
			}

			//Cannot find any possible of wumpus nearby, choose to go to maybe safe location
			if (to_visit.empty()) {
				auto maybe_safe_direct = ask_possible_pos(Question::NOT_UNSAFE);
				if (maybe_safe_direct.size()) {
					to_visit.emplace_back(std::move(maybe_safe_direct[0]) + pos);
				}
			}

			//If cannot make any further decision, break the loop
			if (to_visit.empty()) {
				std::cout << "Agent cannot move anymore\n";
				break;
			}

			//Move
			move(to_visit.back());
			to_visit.pop_back();
		}
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
		return kb.answer(percept);
	}
	void tell_kb(const CNFSentence& percept) {
		kb.add_percepts(CNFSentence(percept));
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
					if (ask_kb(get_safe(new_pos))) res.emplace_back(new_pos);
					break;
				case Question::NOT_UNSAFE:
					if (!ask_kb(get_not_unsafe(new_pos))) res.emplace_back(new_pos);
					break;
				case Question::WUMPUS:
					if (!ask_kb(~get_wumpus(new_pos))) res.emplace_back(new_pos);
					break;
				case Question::UNVISITED:
					if (!brd.get_tile(new_pos).is_visited) res.emplace_back(new_pos);
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
		--limit;
		pos = location;
		brd.mark_visited(pos);
		std::cout << "Agent move to:\n" << pos << brd.get_tile(pos);
	}
	void shoot(const Position& direct) {
		point -= 1000;
		brd.check_shoot(pos, direct);
		auto direct_ = [&]() {
			if (direct == UP) {
				return "top of the map";
			}
			if (direct == DOWN) {
				return "bottom of the map";
			}
			if (direct == UP) {
				return "left of the map";
			}
			if (direct == UP) {
				return "right of the map";
			}
		}();
		std::cout << "Agent shoot from " << pos << "to the " << direct_;
	}

	//Attributes
	KB& kb;
	Board& brd;
	Position pos;
	int limit;
	int point = 0;
	std::vector<Position> to_visit;
};