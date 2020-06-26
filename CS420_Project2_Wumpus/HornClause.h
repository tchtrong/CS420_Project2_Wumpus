#pragma once

#include "Agent.h"

class HornClause {
public:
	HornClause(const std::set<Clause>& premise_, const std::set<Clause>& conclusion_):
		premise(premise_),
		conclusion(conclusion_)
	{}
	bool operator<(const HornClause& rhs)const {
		if (premise < rhs.premise && conclusion < rhs.conclusion)return true;
		if (premise == rhs.premise)return conclusion < rhs.conclusion;
		if (conclusion == rhs.conclusion)return premise < rhs.premise;
		return false;
	}
	bool operator==(const HornClause& rhs)const {
		return premise == conclusion;
	}
private:
	std::set<Clause> premise;
	std::set<Clause> conclusion;
	friend KB_HornClause;
};

class KB_HornClause {
public:
	KB_HornClause() = default;
	KB_HornClause(size_t width_, size_t height_, const Position& pos) : width(width_), height(height_) {
		CNFSentence at_least_one_wumpus{ Clause{} };
		add_sentence(get_literal("~P", pos) & get_literal("~W", pos));
		for (int i = 0; i < width; ++i) {
			for (int j = 0; j < height; ++j) {
				get_breeze_pit({ i, j });
				get_stench_wumpus({ i, j });
			}
		}
		//cout_clauses();
	}

	void add_implies(const CNFSentence& premise, const CNFSentence& conclusion) {
		std::set<Clause> premise_;
		std::set<Clause> conclusion_;
		/*for (const auto& p : premise.clauses) {
			premise_.emplace(p);
		}
		for (const auto& c : conclusion.clauses) {
			conclusion_.emplace(c);
		}
		add_clause({ std::move(premise_), std::move(conclusion_) });*/
		auto neg_premise = ~premise;
		auto neg_conclusion = ~conclusion;
		premise_.clear();
		conclusion_.clear();
		for (const auto& p : neg_premise.clauses) {
			premise_.emplace(p);
		}
		for (const auto& c : neg_conclusion.clauses) {
			conclusion_.emplace(c);
		}
		add_clause({ std::move(premise_), std::move(conclusion_) });
	}
	void add_equivalent(const CNFSentence& first, const CNFSentence& second) {
		add_implies(first, second);
		add_implies(second, first);
	}
	bool entails(const CNFSentence& sentence) const {
		return std::includes(agenda.begin(), agenda.end(),
			sentence.clauses.begin(), sentence.clauses.end());
	}
	void add_sentence(const CNFSentence& sentence) {
		for (auto& cl : sentence.clauses) {
			agenda.emplace(cl);
		}
		for (auto i = clauses.begin(); i != clauses.end(); ++i) {
			if (std::includes(agenda.begin(), agenda.end(),
				(*i).premise.begin(), (*i).premise.end())) {
				agenda.merge(std::set<Clause>{(*i).conclusion});
			}
		}
	}


private:
	bool includes(const CNFSentence& sentence) const {
		return std::includes(agenda.begin(), agenda.end(),
			sentence.clauses.begin(), sentence.clauses.end());
	}

	void get_breeze_pit(const Position& pos) {
		CNFSentence res = get_literal("B", pos);
		CNFSentence possible_pit{ Clause{} };
		for (const auto& direct : possible_pos) {
			if (is_valid(pos + direct)) {
				possible_pit = possible_pit | get_literal("P", pos + direct);
			}
		}
		add_equivalent(res, possible_pit);
	}

	void get_stench_wumpus(const Position& pos) {
		CNFSentence res = get_literal("S", pos);
		CNFSentence possible_pit{ Clause{} };
		for (const auto& direct : possible_pos) {
			if (is_valid(pos + direct)) {
				possible_pit = possible_pit | get_literal("W", pos + direct);
			}
		}
		add_equivalent(res, possible_pit);
	}

	bool is_valid(const Position& pos) {
		auto [x, y] = pos;
		return x >= 0 && y >= 0 && x < width&& y < height;
	}

	void add_clause(HornClause&& h) {
		if (std::find(clauses.begin(), clauses.end(), h) == clauses.end()) {
			clauses.emplace_back(std::move(h));
		}
	}

	void cout_clauses() {
		for (auto& cl : clauses) {
			for (auto& p : cl.premise) {
				std::cout << p << ' ';
			}
			std::cout << "=> ";
			for (auto& c : cl.conclusion) {
				std::cout << c << ' ';
			}
			std::cout << '\n';
		}
	}

	const size_t width;
	const size_t height;
	std::set<Clause> agenda;
	std::vector<HornClause> clauses;
};