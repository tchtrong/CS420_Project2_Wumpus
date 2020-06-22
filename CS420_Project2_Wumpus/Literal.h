#pragma once

#include "make_vector.h"

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <string>
#include <iostream>
#include <vector>

constexpr char TRUE[] = "TRUE";
constexpr char FALSE[] = "FALSE";

class Literal {
public:
	class Clause {
	public:
		class CNFSentence {
		public:
			CNFSentence() : clauses{ Clause{Literal{TRUE}} } {}

			explicit CNFSentence(const Literal& lit) : CNFSentence(Literal(lit)) {}
			explicit CNFSentence(Literal&& lit) : 
				clauses([&]() {
				std::vector<Clause> val;
				val.emplace_back(std::move(lit));
				return val;
				}())
			{}

			explicit CNFSentence(const Clause& cl) : CNFSentence(Clause(cl)) {}
			explicit CNFSentence(Clause&& cl) : clauses([&]() {
				std::vector<Clause> val;
				val.emplace_back(std::move(cl));
				return val;
				}())
			{
				make_compact();
			}

			//CNF OR Literal
			CNFSentence operator|(const Literal& lit)const& {
				return CNFSentence{ *this } | Literal{ lit };
			}
			CNFSentence operator|(Literal&& lit)const& {
				return CNFSentence{ *this } | Literal{ std::move(lit) };
			}
			CNFSentence operator|(const Literal& lit)&& {
				return CNFSentence{ std::move(*this) } | Literal{ lit };
			}
			CNFSentence operator|(Literal&& lit)&& {
				Clause temp{ std::move(lit) };
				size_t i_ = 0;
				for (; i_ < size() && clauses[i_] != temp; ++i_) {}
				if (i_ == size())return std::move(*this);
				for (auto i = clauses.begin(); i != clauses.end(); ) {
					*i = std::move(*i) | Literal{ lit };
					(*i).make_compact();
					if ((*i).is_unsatisfiable()) return CNFSentence(Literal{ FALSE });
					else if ((*i).is_tautology()) {
						i = clauses.erase(i);
					}
					else {
						++i;
					}
				}
				if (!clauses.size()) return CNFSentence(Literal{ TRUE });
				return std::move(*this);
			}

			//CNF OR Clause
			CNFSentence operator|(const Clause& lit)const& {
				return CNFSentence{ *this } | Clause{ lit };
			}
			CNFSentence operator|(Clause&& lit)const& {
				return CNFSentence{ *this } | Clause{ std::move(lit) };
			}
			CNFSentence operator|(const Clause& lit)&& {
				return CNFSentence{ std::move(*this) } | Clause{ lit };
			}
			CNFSentence operator|(Clause&& lit)&& {
				lit.make_compact();
				size_t i_ = 0;
				for (; i_ < size() && clauses[i_] != lit; ++i_) {}
				if (i_ == size())return std::move(*this);
				for (auto i = clauses.begin(); i != clauses.end(); ) {
					*i = std::move(*i) | Clause{ lit };
					(*i).make_compact();
					if ((*i).is_unsatisfiable()) return CNFSentence(Literal{ FALSE });
					else if ((*i).is_tautology()) {
						i = clauses.erase(i);
					}
					else {
						++i;
					}
				}
				if (!clauses.size()) return CNFSentence(Literal{ TRUE });
				return std::move(*this);
			}

			//CNF OR CNF
			CNFSentence operator|(const CNFSentence& lit)const& {
				return CNFSentence{ *this } | CNFSentence{ lit };
			}
			CNFSentence operator|(CNFSentence&& lit)const& {
				return CNFSentence{ *this } | CNFSentence{ std::move(lit) };
			}
			CNFSentence operator|(const CNFSentence& lit)&& {
				return CNFSentence{ std::move(*this) } | CNFSentence{ lit };
			}
			CNFSentence operator|(CNFSentence&& lit)&& {
				std::vector<Clause> res;
				res.reserve(clauses.size() * lit.size());
				this->sort();
				lit.sort();

				/*auto [first, second] = std::mismatch(clauses.begin(), clauses.end(), lit.clauses.begin(), lit.clauses.end());
				if (first == clauses.end() && second == lit.clauses.end()) {
					return std::move(*this);
				}
				else if (first != clauses.end() && first != clauses.begin()) {
					if (second == lit.clauses.end()) {
						std::move(clauses.begin(), clauses.end(), std::back_inserter(res));
						return res;
					}
					else {
						std::move(clauses.begin(), first, std::back_inserter(res));
					}
				}
				else if (second != lit.clauses.end() && second != lit.clauses.begin()) {
					if (first == clauses.end()) {
						std::move(lit.clauses.begin(), lit.clauses.end(), std::back_inserter(res));
						return res;
					}
					else {
						std::move(clauses.begin(), second, std::back_inserter(res));
					}
				}*/
				/*CNFSentence res_{ res };
				const auto clause_end = clauses.end();
				const auto lit_clause_end = lit.clauses.end();
				const auto stop_i = clause_end - 1;
				const auto stop_j = lit_clause_end - 1;
				for (auto i = first; i != clause_end && i != stop_i; ++i) {
					for (auto j = second; j != lit_clause_end && j != stop_j; ++j) {
						res_ = std::move(res_) & (*i | *j);
					}
					res_ = std::move(res_) & (std::move(*i) | *stop_j);
				}
				for (auto j = second; j != stop_j && j != lit_clause_end; ++j) {
					res_ = std::move(res_) & (*stop_i | *j);
				}
				res_ = std::move(res_) & (std::move(*stop_i) | std::move((*stop_j)));
				return res_;*/

				/*CNFSentence res_{ res };
				const auto clause_end = clauses.end();
				const auto lit_clause_end = lit.clauses.end();
				for (auto i = first; i != clause_end; ++i) {
					for (auto j = second; j != lit_clause_end; ++j) {
						res_ = std::move(res_) & (*i | *j);
					}
				}
				return res_;*/
				
				size_t pos = 0;
				for (; pos < size() && pos < lit.size() && clauses[pos] == lit.clauses[pos]; ++pos) {}
				if (pos == size() && pos == lit.size()) {
					return std::move(*this);
				}
				else if (pos < size() && pos) {
					if (pos == lit.size()) {
						std::move(clauses.begin(), clauses.end(), std::back_inserter(res));
						return res;
					}
					else {
						std::move(clauses.begin(), clauses.begin() + pos, std::back_inserter(res));
					}
				}
				else if (pos < lit.size() && pos) {
					if (pos == size()) {
						std::move(lit.clauses.begin(), lit.clauses.end(), std::back_inserter(res));
						return res;
					}
					else {
						std::move(lit.clauses.begin(), lit.clauses.begin() + pos, std::back_inserter(res));
					}
				}
				CNFSentence res_{ std::move(res) };
				size_t i = 0;
				size_t j = 0;
				for (i = pos; i < clauses.size() - 1; ++i) {
					for (j = pos; j < lit.clauses.size() - 1; ++j) {
						res_ = std::move(res_) & (clauses[i] | lit.clauses[j]);
					}
					res_ = std::move(res_) & (std::move(clauses[i]) | lit.clauses[j]);
				}
				for (j = pos; j < lit.clauses.size() - 1; ++j) {
					res_ = std::move(res_) & (clauses[i] | lit.clauses[j]);
				}
				res_ = std::move(res_) & (std::move(clauses[i]) | std::move(lit.clauses[j]));
				return res_;
			}

			//CNF AND Literal
			CNFSentence operator&(const Literal& lit)const& {
				return CNFSentence{ *this } & Literal{ lit };
			}
			CNFSentence operator&(Literal&& lit)const& {
				return CNFSentence{ *this } & Literal{ std::move(lit) };
			}
			CNFSentence operator&(const Literal& lit)&& {
				return CNFSentence{ std::move(*this) } & Literal{ lit };
			}
			CNFSentence operator&(Literal&& lit)&& {
				if (lit.is_unsatisfiable() || this->is_unsatisfiable()) {
					return CNFSentence(Literal{ FALSE });
				}
				else if (!lit.is_tautology()) {
					if (this->is_tautology()) {
						return CNFSentence(std::move(lit));
					}
					else {
						Clause tmp{ std::move(lit) };
						size_t i_ = 0;
						for (; i_ < size() && clauses[i_] != tmp; ++i_) {}
						if (i_ == size()) clauses.emplace_back(std::move(tmp));
						return std::move((*this));
					}
				}
				return std::move((*this));
			}

			//CNF AND Clause
			CNFSentence operator&(const Clause& clause)const& { 
				return CNFSentence{ *this } & Clause{ clause }; 
			}
			CNFSentence operator&(Clause&& clause)const& { 
				return CNFSentence{ *this } & Clause{ std::move(clause) }; 
			}
			CNFSentence operator&(const Clause& clause)&& { 
				return CNFSentence{ std::move(*this) } & Clause{ clause }; 
			}
			CNFSentence operator&(Clause&& clause)&& {
				if (clause.is_unsatisfiable() || this->is_unsatisfiable()) {
					return CNFSentence(Literal{ FALSE });
				}
				else if (this->is_tautology()) {
					return CNFSentence(std::move(clause));
				}
				else {
					clause.make_compact();
					size_t i_ = 0;
					for (; i_ < size() && clauses[i_] != clause; ++i_) {}
					if (i_ == size()) clauses.emplace_back(std::move(clause));
					return std::move((*this));
				}
			}

			//CNF AND CNF
			CNFSentence operator&(const CNFSentence& sentence)const& {
				return CNFSentence{ *this } &CNFSentence{ sentence };
			}
			CNFSentence operator&(CNFSentence&& sentence)const& {			
				return CNFSentence{ *this } &CNFSentence{ std::move(sentence) };
			}
			CNFSentence operator&(const CNFSentence& sentence)&& {
				return CNFSentence{ std::move(*this) } &CNFSentence{ sentence };
			}
			CNFSentence operator&(CNFSentence&& sentence)&& {
				if (sentence.is_unsatisfiable() || this->is_unsatisfiable()) {
					return CNFSentence(Literal{ FALSE });
				}
				else if (!sentence.is_tautology()) {
					if (this->is_tautology()) {
						return std::move(sentence);
					}
					else {
						std::copy_if(
							std::make_move_iterator(sentence.clauses.begin()),
							std::make_move_iterator(sentence.clauses.end()),
							std::back_inserter(clauses),
							[&](const Clause& x) {	
								size_t i_ = 0;
								for (; i_ < size() && clauses[i_] != x; ++i_) {}
								return (i_ == size());
							}
						);
					}
				}
				return std::move((*this));
			}

			//CNFSentence IMPLIES Literal
			CNFSentence implies(const Literal& lit)const& {
				return CNFSentence{ *this }.implies(Literal{ lit });
			}
			CNFSentence implies(Literal&& lit)const& {
				return CNFSentence{ *this }.implies(Literal{ std::move(lit) });
			}
			CNFSentence implies(const Literal& lit)&& {
				return CNFSentence{ std::move(*this) }.implies(Literal{ lit });
			}
			CNFSentence implies(Literal&& lit)&& {
				return CNFSentence{ ~(std::move(*this)) } | std::move(lit);
			}

			//Clause IMPLIES Clause
			CNFSentence implies(const Clause& clause)const& {
				return CNFSentence{ *this }.implies(Clause{ clause });
			}
			CNFSentence implies(Clause&& clause)const& {
				return CNFSentence{ *this }.implies(Clause{ std::move(clause) });
			}
			CNFSentence implies(const Clause& clause)&& {
				return CNFSentence{ std::move(*this) }.implies(Clause{ clause });
			}
			CNFSentence implies(Clause&& clause)&& {
				return CNFSentence{ ~(std::move(*this)) } | std::move(clause);
			}

			//Literal IMPLIES CNF
			CNFSentence implies(const CNFSentence& sentence)const& {
				return CNFSentence{ *this }.implies(CNFSentence{ sentence });
			}
			CNFSentence implies(CNFSentence&& sentence)const& {
				return CNFSentence{ *this }.implies(CNFSentence{ std::move(sentence) });
			}
			CNFSentence implies(const CNFSentence& sentence)&& {
				return CNFSentence{ std::move(*this) }.implies(CNFSentence{ sentence });
			}
			CNFSentence implies(CNFSentence&& sentence)&& {
				return CNFSentence{ ~(std::move(*this)) } | std::move(sentence);
			}

			////Literal EQUIVALENT
			CNFSentence equivalent(const Literal& lit)const& {
				return ((*this) & lit) | (~(*this) & ~lit);
			}
			//Literal EQUIVALENT
			CNFSentence equivalent(const Clause& clause)const& {
				return ((*this) & clause) | (~(*this) & ~clause);
			}
			//Literal EQUIVALENT
			CNFSentence equivalent(const CNFSentence& sentence)const& {
				return ((*this) & sentence) | (~(*this) & ~sentence);
			}

			//NEGATION
			CNFSentence operator~()const& {
				return ~CNFSentence{ *this };
			}
			CNFSentence operator~()&& {
				if (this->is_tautology()) return CNFSentence(Literal{ FALSE });
				else if (this->is_unsatisfiable()) return CNFSentence(Literal{ TRUE });

				CNFSentence res{ ~std::move(clauses.front()) };
				for (auto i = clauses.begin() + 1; i != clauses.end(); ++i) {
					res = std::move(res) | ~std::move(*i);
				}
				return res;
			}

			bool is_tautology() const {
				return clauses.size() == 1 && clauses.front().is_tautology();
			}
			bool is_unsatisfiable() const {
				return clauses.size() == 1 && clauses.front().is_unsatisfiable();
			}

			size_t size() const {
				return clauses.size();
			}

			friend std::ostream& operator<<(std::ostream& os, const CNFSentence& sentence) {
				if (!sentence.size()) {
					return os;
				}
				else if (sentence.size() == 1) {
					return os << sentence.clauses.front();
				}
				else {
					os << "(";
					for (auto clause = sentence.clauses.begin(); clause != sentence.clauses.end(); ++clause) {
						os << *clause;
						if (clause + 1 != sentence.clauses.end()) {
							os << " & ";
						}
					}
					return os << ")";
				}
			}

			void make_compact() {
				for (auto& clause : clauses) {
					clause.make_compact();
				}
			}

			CNFSentence& sort() {
				std::sort(clauses.begin(), clauses.end(), std::less<Clause>());
				return *this;
			}

		private:
			CNFSentence(const std::vector<Clause>& list) : CNFSentence(std::vector<Clause>(list)) {}
			CNFSentence(std::vector<Clause>&& list) : clauses{ std::move(list) } {
				make_compact();
				sort();
			}
			std::vector<Clause> clauses;
			friend Clause;
			friend Literal;
		};
	public:
		Clause() : literals{ Literal{FALSE} } {}

		Clause(const Literal& lit) : literals({ lit }) {}
		Clause(Literal&& lit) :literals({ std::move(lit) }) {}

		//Clause OR Literal
		Clause operator|(const Literal& clause)const& {
			return Clause{ *this } | Clause{ clause };
		}
		Clause operator|(Literal&& clause)const& {
			return Clause{ *this } | Clause{ std::move(clause) };
		}
		Clause operator|(const Literal& clause)&& {
			return Clause{ std::move(*this) } | Literal{ clause };
		}
		Clause operator|(Literal&& clause)&& {
			if (this->is_tautology() || clause.is_tautology()) {
				return { Literal{ TRUE } };
			}
			else if (clause.is_unsatisfiable()) {
				return std::move(*this);
			}
			else if (this->is_unsatisfiable()) {
				return { std::move(clause) };
			}
			else {
				size_t i_ = 0;
				for (; i_ < size() && literals[i_] != clause; ++i_) {}
				if (i_ == size()) literals.emplace_back(std::move(clause));
				this->sort();
				return std::move(*this);
			}
		}

		//Clause OR Clause
		Clause operator|(const Clause& sentence)const& {
			return  Clause{ *this } | Clause{ sentence };
		}
		Clause operator|(Clause&& sentence)const& {
			return Clause{ *this } | Clause{ std::move(sentence) };
		}
		Clause operator|(const Clause& sentence)&& {
			return Clause{ std::move(*this) } | Clause{ sentence };
		}
		Clause operator|(Clause&& sentence)&& {
			if (this->is_tautology() || sentence.is_tautology()) {
				return Literal{ TRUE };
			}
			else if (sentence.is_unsatisfiable()) {
				return std::move(*this);
			}
			else if (this->is_unsatisfiable()) {
				return { std::move(sentence) };
			}
			else {
				for (size_t i = 0; i < sentence.size(); ++i) {
					size_t j = 0;
					for (; j < size() && literals[j] != sentence.literals[i]; ++j) {}
					if (j == size()) literals.emplace_back(std::move(sentence.literals[i]));
				}
				this->sort();
				return std::move(*this);
			}
		}

		//Clause OR CNF
		CNFSentence operator|(const CNFSentence& sentence)const& {
			return  Clause{ *this } | CNFSentence{ sentence };
		}
		CNFSentence operator|(CNFSentence&& sentence)const& {
			return Clause{ *this } | CNFSentence{ std::move(sentence) };
		}
		CNFSentence operator|(const CNFSentence& sentence)&& {
			return Clause{ std::move(*this) } | CNFSentence{ sentence };
		}
		CNFSentence operator|(CNFSentence&& sentence)&& {
			return CNFSentence{ std::move(sentence) } | Clause{ std::move(*this) };
		}

		//Clause AND Literal
		CNFSentence operator&(const Literal& lit)const& {
			return CNFSentence(*this) & CNFSentence(lit);
		}
		CNFSentence operator&(Literal&& lit)const& {
			return CNFSentence(*this) & CNFSentence(std::move(lit));
		}
		CNFSentence operator&(const Literal& lit)&& {
			return CNFSentence(std::move(*this)) & CNFSentence(lit);
		}
		CNFSentence operator&(Literal&& lit)&& {
			if (lit.is_unsatisfiable() || this->is_unsatisfiable()) {
				return CNFSentence(Literal{ FALSE });
			}
			else if (lit.is_tautology()) {
				return CNFSentence(std::move(*this));
			}
			else if (this->is_tautology()) {
				return CNFSentence(std::move(lit));
			}
			else {
				this->make_compact();
				std::vector<Clause> res;
				res.reserve(this->size() + 1);

			}
			return CNFSentence(std::move(*this)) & CNFSentence(std::move(lit));
		}

		//Clause AND Clause
		CNFSentence operator&(const Clause& clause)const& {
			return Clause{ *this } & Clause{ clause };
		}
		CNFSentence operator&(Clause&& clause)const& {
			return Clause{ *this } & Clause{ std::move(clause) };
		}
		CNFSentence operator&(const Clause& clause)&& {
			return Clause{ std::move(*this) } & Clause{ clause };
		}
		CNFSentence operator&(Clause&& clause)&& {
			if ((*this).is_unsatisfiable() || clause.is_unsatisfiable()) {
				return CNFSentence(Literal{ FALSE });
			}
			this->make_compact();
			if (this->is_tautology()) {
				return CNFSentence(std::move(clause));
			}
			clause.make_compact();
			if (clause.is_tautology()) {
				return CNFSentence(std::move(*this));
			}
			if (*this == clause) return CNFSentence(std::move(clause));
			return CNFSentence(std::move(*this)) & CNFSentence(std::move(clause));
		}

		//Clause AND CNF
		CNFSentence operator&(const CNFSentence& sentence)const& {
			return  Clause{ *this } & CNFSentence{ sentence };
		}
		CNFSentence operator&(CNFSentence&& sentence)const& {
			return Clause{ *this } & CNFSentence{ std::move(sentence) };
		}
		CNFSentence operator&(const CNFSentence& sentence)&& {
			return Clause{ std::move(*this) } & CNFSentence{ sentence };
		}
		CNFSentence operator&(CNFSentence&& sentence)&& {
			return CNFSentence{ std::move(sentence) } & Clause{ std::move(*this) };
		}

		//Clause IMPLIES Literal
		CNFSentence implies(const Literal& lit)const& {
			return Clause{ *this }.implies(Literal{ lit });
		}
		CNFSentence implies(Literal&& lit)const& {
			return Clause{ *this }.implies(Literal{ std::move(lit) });
		}
		CNFSentence implies(const Literal& lit)&& {
			return Clause{ std::move(*this) }.implies(Literal{ lit });
		}
		CNFSentence implies(Literal&& lit)&& {
			return CNFSentence{ ~(std::move(*this)) } | std::move(lit);
		}

		//Clause IMPLIES Clause
		CNFSentence implies(const Clause& clause)const& {
			return Clause{ *this }.implies(Clause{ clause });
		}
		CNFSentence implies(Clause&& clause)const& {
			return Clause{ *this }.implies(Clause{ std::move(clause) });
		}
		CNFSentence implies(const Clause& clause)&& {
			return Clause{ std::move(*this) }.implies(Clause{ clause });
		}
		CNFSentence implies(Clause&& clause)&& {
			return CNFSentence{ ~(std::move(*this)) } | std::move(clause);
		}

		//Clause IMPLIES CNF
		CNFSentence implies(const CNFSentence& sentence)const& {
			return Clause{ *this }.implies(CNFSentence{ sentence });
		}
		CNFSentence implies(CNFSentence&& sentence)const& {
			return Clause{ *this }.implies(CNFSentence{ std::move(sentence) });
		}
		CNFSentence implies(const CNFSentence& sentence)&& {
			return Clause{ std::move(*this) }.implies(CNFSentence{ sentence });
		}
		CNFSentence implies(CNFSentence&& sentence)&& {
			return CNFSentence{ ~(std::move(*this)) } | std::move(sentence);
		}

		////Clause EQUIVALENT
		CNFSentence equivalent(const Literal& lit)const& {
			return ((*this) & lit) | (~(*this) & ~lit);
		}
		//Clause EQUIVALENT
		CNFSentence equivalent(const Clause& clause)const& {
			return ((*this) & clause) | (~(*this) & ~clause);
		}
		//Clause EQUIVALENT
		CNFSentence equivalent(const CNFSentence& sentence)const& {
			return ((*this) & sentence) | (~(*this) & ~sentence);
		}

		//NEGATION
		CNFSentence operator~()const& {
			return { ~Clause{ *this } };
		}
		CNFSentence operator~()&& {
			this->make_compact();
			if (this->is_tautology()) {
				return CNFSentence(Literal{ FALSE });
			}
			else if (this->is_unsatisfiable()) {
				return CNFSentence(Literal{ TRUE });
			}
			else {
				std::vector<Clause> res;
				res.reserve(size());
				for (auto& l : literals) {
					res.emplace_back(Clause{ ~std::move(l) });
				}
				return res;
			}
		}

		bool operator<(const Clause& clause) const {
			if (size() < clause.size()) return true;
			if (size() == clause.size()) {
				size_t i = 0;
				for (; i < size() && literals[i] == clause.literals[i]; ++i) {}
				if (i == size()) return false;
				if (literals[i] < clause.literals[i]) return true;
			}
			return false;
		}
		bool operator==(const Clause& clause) const {
			if (this->size() != clause.size()) return false;
			for (size_t i = 0; i < clause.size(); ++i) {
				if (literals[i] != clause.literals[i]) {
					return false;
				}
			}
			return true;
		}
		bool operator!=(const Clause& lit) const {
			return !(*this == lit);
		}

		bool is_tautology() const {
			if (literals.size() == 1 && literals[0].is_tautology()) {
				return true;
			}
			return false;
		}
		bool is_unsatisfiable() const {
			return literals.size() == 1 && literals.front().is_unsatisfiable();
		}

		size_t size() const {
			return literals.size();
		}

		friend std::ostream& operator<<(std::ostream& os, const Clause& clause) {
			if (!clause.size()) {
				return os;
			}
			else if (clause.size() == 1) {
				return os << clause.literals.front();
			}
			else {
				os << "(";
				for (auto lit = clause.literals.begin(); lit != clause.literals.end(); ++lit) {
					os << *lit;
					if (lit + 1 != clause.literals.end()) {
						os << " | ";
					}
				}
				return os << ")";
			}
		}

		void make_compact() {
			for (auto i = literals.begin(); i != literals.end(); ) {
				bool deleted = false;
				for (auto j = i + 1; j != literals.end();) {
					if ((*i).is_complementary(*j)) {
						j = literals.erase(j);
						i = literals.erase(i);
						deleted = true;
						break;
					}
					else {
						++j;
					}
				}
				if (!deleted) ++i;
			}
			if (!literals.size()) literals.emplace_back(Literal{ TRUE });
		}

		void sort() {
			std::sort(literals.begin(), literals.end(), std::less<Literal>());
		}

	private:
		std::vector<Literal> literals;
		friend Literal;
	};
public:
	using CNFSentence = Clause::CNFSentence;
	Literal(std::string name, bool negation = false) :name(name), negation(negation) {
		if (name.front() == '~') {
			this->negation = true;
			this->name.erase(this->name.begin());
		}
	}

	//Literal AND Literal
	CNFSentence operator&(const Literal& lit)const& {
		return Literal{ *this } & Literal{ lit };
	}
	CNFSentence operator&(Literal&& lit)const& {
		return Literal{ *this } & Literal{ std::move(lit) };
	}
	CNFSentence operator&(const Literal& lit)&& {
		return Literal{ std::move(*this) } & Literal{ lit };
	}
	CNFSentence operator&(Literal&& lit)&& {
		if (this->is_unsatisfiable() || lit.is_unsatisfiable() || this->is_complementary(lit)) {
			return CNFSentence(Literal{ FALSE });
		}
		else if (lit.is_tautology()) {
			return CNFSentence(std::move(*this));
		}
		else if (this->is_tautology()) {
			return CNFSentence(std::move(lit));
		}
		else if (*this == lit) {
			return CNFSentence(std::move(*this));
		}
		
		else return CNFSentence(std::move(*this)) & CNFSentence(std::move(lit));
	}
	
	//Literal AND Clause
	CNFSentence operator&(const Clause& clause)const& {
		return Literal{ *this } & Clause{ clause };
	}
	CNFSentence operator&(Clause&& clause)const& {
		return Literal{ *this } & Clause{ std::move(clause) };
	}
	CNFSentence operator&(const Clause& clause)&& {
		return Literal{ std::move(*this) } & Clause{ clause };
	}
	CNFSentence operator&(Clause&& clause)&& {		
		return Clause{ std::move(clause) } & Literal{ std::move(*this) };
	}

	//Literal AND CNF
	CNFSentence operator&(const CNFSentence& clause)const& {
		return Literal{ *this } & CNFSentence{ clause };
	}
	CNFSentence operator&(CNFSentence&& clause)const& {
		return Literal{ *this } & CNFSentence{ std::move(clause) };
	}
	CNFSentence operator&(const CNFSentence& clause)&& {
		return Literal{ std::move(*this) } & CNFSentence{ clause };
	}
	CNFSentence operator&(CNFSentence&& clause)&& {
		return CNFSentence{ std::move(clause) } & Literal{ std::move(*this) };
	}

	//Literal OR Literal
	Clause operator|(const Literal& lit)const& {
		return Literal{ *this } | Literal{ lit };
	}
	Clause operator|(Literal&& lit)const& {
		return Literal{ *this } | Literal{ std::move(lit) };
	}
	Clause operator|(const Literal& lit)&& {
		return Literal{ std::move(*this) } | Literal{ lit };
	}
	Clause operator|(Literal&& lit)&& {
		if (this->is_complementary(lit)) {
			return Literal{ TRUE };
		}
		else if (*this == lit) {
			return std::move(*this);
		}
		if (*this < lit) {
			return Clause{ std::move(*this) } | Clause{  std::move(lit)};
		}
		else return Clause{ std::move(lit) } | Clause{ std::move(*this) };
	}

	//Literal OR Clause
	Clause operator|(const Clause& lit)const& {
		return Literal{ *this } | Clause{ lit };
	}
	Clause operator|(Clause&& lit)const& {
		return Literal{ *this } | Clause{ std::move(lit) };
	}
	Clause operator|(const Clause& lit)&& {
		return Literal{ std::move(*this) } | Clause{ lit };
	}
	Clause operator|(Clause&& lit)&& {
		return Clause{ std::move(lit) } | Literal{ std::move(*this) };
	}

	//Literal OR CNF
	CNFSentence operator|(const CNFSentence& lit)const& {
		return Literal{ *this } | CNFSentence{ lit };
	}
	CNFSentence operator|(CNFSentence&& lit)const& {
		return Literal{ *this } | CNFSentence{ std::move(lit) };
	}
	CNFSentence operator|(const CNFSentence& lit)& {
		return Literal{ std::move(*this) } | CNFSentence{ lit };
	}
	CNFSentence operator|(CNFSentence&& lit)&& {
		return CNFSentence{ std::move(lit) } | Literal{ std::move(*this) };
	}

	//Literal IMPLIES Literal
	Clause implies(const Literal& lit)const& {
		return Literal{ *this }.implies(Literal{ lit });
	}
	Clause implies(Literal&& lit)const& {
		return Literal{ *this }.implies(Literal{ std::move(lit) });
	}
	Clause implies(const Literal& lit)&& {
		return Literal{ std::move(*this) }.implies(Literal{ lit });
	}
	Clause implies(Literal&& lit)&& {
		return Literal{ ~(std::move(*this)) } | std::move(lit);
	}

	//Literal IMPLIES Clause
	Clause implies(const Clause& clause)const& {
		return Literal{ *this }.implies(Clause{ clause });
	}
	Clause implies(Clause&& clause)const& {
		return Literal{ *this }.implies(Clause{ std::move(clause) });
	}
	Clause implies(const Clause& clause)&& {
		return Literal{ std::move(*this) }.implies(Clause{ clause });
	}
	Clause implies(Clause&& clause)&& {
		return Literal{ ~(std::move(*this)) } | std::move(clause);
	}

	//Literal IMPLIES CNF
	CNFSentence implies(const CNFSentence& sentence)const& {
		return Literal{ *this }.implies(CNFSentence{ sentence });
	}
	CNFSentence implies(CNFSentence&& sentence)const& {
		return Literal{ *this }.implies(CNFSentence{ std::move(sentence) });
	}
	CNFSentence implies(const CNFSentence& sentence)&& {
		return Literal{ std::move(*this) }.implies(CNFSentence{ sentence });
	}
	CNFSentence implies(CNFSentence&& sentence)&& {
		return Literal{ ~(std::move(*this)) } | std::move(sentence);
	}

	////Literal EQUIVALENT
	CNFSentence equivalent(const Literal& lit)const& {
		return ((*this) & lit) | (~(*this) & ~lit);
	}
	//Literal EQUIVALENT
	CNFSentence equivalent(const Clause& clause)const& {
		return ((*this) & clause) | (~(*this) & ~clause);
	}
	//Literal EQUIVALENT
	CNFSentence equivalent(const CNFSentence& sentence)const& {
		return ((*this) & sentence) | (~(*this) & ~sentence);
	}

	//NEGATION
	Literal operator~() const& {
		return { name, !negation };
	}
	Literal operator~()&& {
		if (name == TRUE) {
			return Literal{ FALSE };
		}
		else if (name == FALSE) {
			return Literal{ TRUE };
		}
		return { std::move(name), !negation };
	}

	bool operator<(const Literal& lit) const {
		return name < lit.name;
	}
	bool operator==(const Literal& lit) const {
		return name == lit.name;
	}
	bool operator!=(const Literal& lit) const {
		return !(*this == lit);
	}

	bool is_complementary(const Literal& lit) const {
		return name == lit.name && negation != lit.negation;
	}
	bool is_tautology() const {
		return name == TRUE;
	}
	bool is_unsatisfiable() const {
		return name == FALSE;
	}

	friend std::ostream& operator<<(std::ostream& os, const Literal& lit) {
		if (lit.negation) {
			os << '~';
		}
		return os << lit.name;
	}
private:
	std::string name;
	bool negation = false;
};

using Clause = Literal::Clause;
using CNFSentence = Literal::CNFSentence;