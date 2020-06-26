#pragma once

#include "Literal.h"

struct Position {
	Position& operator+=(const Position& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Position operator+(const Position& rhs) const {
		Position temp{ *this };
		return temp += rhs;
	}

	Position& operator-=(const Position& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	Position operator-(const Position& rhs) const {
		return Position(*this) += rhs;
	}

	bool operator==(const Position& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Position& rhs) const {
		return !(*this == rhs);
	}
	bool operator<(const Position& rhs) const {
		if (x > rhs.x) return true;
		if (x == rhs.x) return y > rhs.y;
		return false;
	}

	friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
		os << "Location: " << pos.x << ' ' << pos.y << '\n';
		return os;
	}

	int x{};
	int y{};
};

constexpr Position possible_pos[] = { { 0, 1 } ,{ 1, 0 } ,{ 0, -1 } ,{ -1, 0 } };
constexpr Position UP = { 0, 1 };
constexpr Position RIGHT = { 1, 0 };
constexpr Position DOWN = { 0, -1 };
constexpr Position LEFT = { -1, 0 };

CNFSentence get_literal(std::string type, const Position& pos) {
	return CNFSentence{ Literal{type + std::to_string(pos.x) + std::to_string(pos.y)} };
}

class KB {
public:
	KB() = default;
	KB(size_t width_, size_t height_, const Position& pos) : width(width_), height(height_) {
		CNFSentence at_least_one_wumpus{ Clause{} };
		kb = kb & get_literal("~P", pos) & get_literal("~W", pos);
		for (int i = 0; i < width; ++i) {
			for (int j = 0; j < height; ++j) {
				kb = kb & get_breeze_pit({ i, j });
				kb = kb & get_stench_wumpus({ i, j });
				at_least_one_wumpus = std::move(at_least_one_wumpus) | get_literal("W", { i, j });
			}
		}
		kb = kb & std::move(at_least_one_wumpus);
	}

	void add_percepts(const CNFSentence& percept) {
		add_percepts(CNFSentence(percept));
	}
	void add_percepts(CNFSentence&& percept){
		kb = kb & std::move(percept);
	}

	bool answer(const CNFSentence& question) {
		return kb.entails(question);
	}

private:
	CNFSentence get_breeze_pit(const Position& pos) {
		CNFSentence res = get_literal("B", pos);
		CNFSentence possible_pit{ Clause{} };
		for (const auto& direct : possible_pos) {
			if (is_valid(pos + direct)) {
				possible_pit = possible_pit | get_literal("P", pos + direct);
			}
		}
		return res.equivalent(possible_pit);
	}
	CNFSentence get_stench_wumpus(const Position& pos) {
		CNFSentence res = get_literal("S", pos);
		CNFSentence possible_pit{ Clause{} };
		for (const auto& direct : possible_pos) {
			if (is_valid(pos + direct)) {
				possible_pit = possible_pit | get_literal("W", pos + direct);
			}
		}
		return std::move(res).equivalent(std::move(possible_pit));
	}

	bool is_valid(const Position& pos) {
		auto [x, y] = pos;
		return x >= 0 && y >= 0 && x < width && y < height;
	}

	const size_t width;
	const size_t height;
	CNFSentence kb;
};