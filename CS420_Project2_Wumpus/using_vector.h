#pragma once

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
			}

			//CNF OR Literal
			CNFSentence operator|(const Literal& lit)const& {
				return CNFSentence{ *this } | Clause{ lit };
			}
			CNFSentence operator|(Literal&& lit)const& {
				return CNFSentence{ *this } | Clause{ std::move(lit) };
			}
			CNFSentence operator|(const Literal& lit)&& {
				return CNFSentence{ std::move(*this) } | Clause{ lit };
			}
			CNFSentence operator|(Literal&& lit)&& {
				return std::move(*this) | Clause(std::move(lit));
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
				if (lit.is_true() || this->is_true()) {
					return CNFSentence(Literal(TRUE));
				}
				else if (lit.is_false()) {
					return std::move(*this);
				}
				else if (this->is_false()) {
					return CNFSentence(std::move(lit));
				}
				else if (!std::binary_search(clauses.begin(), clauses.end(), lit)) {
					for (size_t i = 0; i < size(); ++i) {
						auto temp = std::move(clauses[i]) | lit;
						if (temp.is_true()) return CNFSentence(Literal(TRUE));
						clauses[i] = std::move(temp);
					}
					return std::move(*this);
				}
				else return std::move(*this);
			}

			//CNF OR CNF
			CNFSentence operator|(const CNFSentence& lit)const& {
				return CNFSentence{ *this } | CNFSentence{ lit };
			}
			CNFSentence operator|(CNFSentence&& lit)const& {
				return CNFSentence{ *this } | CNFSentence{ std::move(lit) };
			}
			CNFSentence&& operator|(const CNFSentence& lit)&& {
				return CNFSentence{ std::move(*this) } | CNFSentence{ lit };
			}
			CNFSentence&& operator|(CNFSentence&& lit)&& {
				CNFSentence res;
				res.clauses.reserve(lit.size() * size());
				res.clauses.erase(res.clauses.begin());

				std::set_intersection(
					clauses.begin(),
					clauses.end(),
					lit.clauses.begin(),
					lit.clauses.end(),
					std::back_inserter(res.clauses)
				);

				std::vector<Clause> first;
				first.reserve(size());
				std::set_difference(
					clauses.begin(),
					clauses.end(),
					lit.clauses.begin(),
					lit.clauses.end(),
					std::back_inserter(first)
				);

				std::vector<Clause> second;
				second.reserve(lit.size());
				std::set_difference(
					lit.clauses.begin(),
					lit.clauses.end(),
					clauses.begin(),
					clauses.end(),
					std::back_inserter(second)
				);

				for (size_t i = 0; i < first.size(); ++i) {
					for (size_t j = 0; j < second.size(); ++j) {
						res = std::move(res) & (first[i] | second[j]);
					}
				}

				*this = std::move(res);
				return std::move(*this);
				/*std::vector<Clause> res;
				res.reserve(clauses.size() * lit.size());

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
				return res_;*/
			}

			//CNF AND Literal
			CNFSentence operator&(const Literal& lit)const& {
				return CNFSentence{ *this } &Literal{ lit };
			}
			CNFSentence operator&(Literal&& lit)const& {
				return CNFSentence{ *this } &Literal{ std::move(lit) };
			}
			CNFSentence operator&(const Literal& lit)&& {
				return CNFSentence{ std::move(*this) } &Literal{ lit };
			}
			CNFSentence operator&(Literal&& lit)&& {
				return std::move(*this) & Clause(std::move(lit));
			}

			//CNF AND Clause
			CNFSentence operator&(const Clause& clause)const& {
				return CNFSentence{ *this } &Clause{ clause };
			}
			CNFSentence operator&(Clause&& clause)const& {
				return CNFSentence{ *this } &Clause{ std::move(clause) };
			}
			CNFSentence operator&(const Clause& clause)&& {
				return CNFSentence{ std::move(*this) } &Clause{ clause };
			}
			CNFSentence operator&(Clause&& clause)&& {
				if (clause.is_false() || this->is_false()) {
					return CNFSentence(Literal(FALSE));
				}
				else if (this->is_true()) {
					return CNFSentence(std::move(clause));
				}
				else if (clause.is_true()) {
					return std::move(*this);
				}
				else {
					auto has_same = std::lower_bound(clauses.begin(), clauses.end(), clause);
					if (has_same == clauses.end()) {
						clauses.emplace_back(std::move(clause));
					}
					else {
						if (*has_same != clause) {
							clauses.emplace(has_same, std::move(clause));
						}
					}
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
				if (sentence.is_false() || this->is_false()) {
					return CNFSentence(Literal(FALSE));
				}
				else if (!sentence.is_true()) {
					if (this->is_true()) {
						return std::move(sentence);
					}
					else {
						clauses.reserve(size() + sentence.size());
						for (size_t i = 0; i < sentence.size() && !this->is_false() && !this->is_true(); ++i) {
							*this = std::move(*this) & std::move(sentence.clauses[i]);
							if (this->is_false()) return CNFSentence(Literal(FALSE));
						}
					}
					return std::move(*this);
				}
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
				if (this->is_true()) { return CNFSentence(Literal(FALSE)); }
				else if (this->is_false()) return CNFSentence(Literal{ TRUE });

				CNFSentence res{ Clause{} };
				for (size_t i = 0; i < clauses.size(); ++i) {
					res = std::move(res) | ~std::move(clauses[i]);
				}
				return res;
			}

			bool operator==(const CNFSentence& lhs) const {
				return clauses == lhs.clauses;
			}
			bool operator!=(const CNFSentence& lhs) const {
				return clauses != lhs.clauses;
			}
			operator bool() const {
				return is_true();
			}
			bool operator!()const {
				return is_false();
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

		private:			
			CNFSentence(const std::vector<Clause>& list) : CNFSentence(std::vector<Clause>(list)) {}
			CNFSentence(std::vector<Clause>&& list) : clauses(std::move(list)) {}
			bool is_true() const {
				return clauses.size() == 1 && clauses[0].is_true();
			}
			bool is_false() const {
				return clauses.size() == 1 && clauses[0].is_false();
			}
			size_t size() const {
				return clauses.size();
			}

			std::vector<Clause> clauses;
			friend Clause;
			friend Literal;
		};
	public:
		Clause() : Clause(Literal(FALSE)) {}

		Clause(const Literal& lit) : Clause(Literal(lit)) {}
		Clause(Literal&& lit) {
			literals.emplace_back(std::move(lit));
		}

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
			if (this->is_true() || clause.is_true()) {
				return Literal{ TRUE };
			}
			else if (clause.is_false()) {
				return std::move(*this);
			}
			else if (this->is_false()) {
				return { std::move(clause) };
			}
			else {
				if (std::binary_search(literals.begin(), literals.end(), ~clause)) {
					return Literal(TRUE);
				}
				const auto has_same = std::lower_bound(literals.begin(), literals.end(), clause);
				if (has_same == literals.end()) {
					literals.emplace_back(std::move(clause));
				}
				else {
					if (*has_same != clause) {
						literals.emplace(has_same, std::move(clause));
					}
				}
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
			if (this->is_true() || sentence.is_true()) {
				return Literal{ TRUE };
			}
			else if (sentence.is_false()) {
				return std::move(*this);
			}
			else if (this->is_false()) {
				return { std::move(sentence) };
			}
			else {
				for (size_t i = 0; i < sentence.size() && !this->is_true(); ++i) {
					*this = std::move(*this) | std::move(sentence.literals[i]);
				}
				return std::move(*this);
			}
		}

		//Clause OR CNF
		CNFSentence operator|(const CNFSentence& sentence)const& {
			return  CNFSentence{ sentence } | Clause{ *this };
		}
		CNFSentence operator|(CNFSentence&& sentence)const& {
			return CNFSentence{ std::move(sentence) } | Clause{ *this };
		}
		CNFSentence operator|(const CNFSentence& sentence)&& {
			return CNFSentence{ sentence } | Clause{ std::move(*this) };
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
			if (lit.is_false() || this->is_false() || lit.is_complementary(*this))
			{
				return CNFSentence(Literal(FALSE));
			}
			else if (lit.is_true()) {
				return CNFSentence(std::move(*this));
			}
			else if (this->is_true() || std::binary_search(literals.begin(), literals.end(), lit)) {
				return CNFSentence(std::move(lit));
			}
			else {
				std::vector<Clause> res;
				res.reserve(2);
				if (*this < lit) {
					res.emplace_back(std::move(*this));
					res.emplace_back(std::move(lit));
				}
				else {
					res.emplace_back(std::move(lit));
					res.emplace_back(std::move(*this));
				}
				return res;
			}
		}

		//Clause AND Clause
		CNFSentence operator&(const Clause& clause)const& {
			return Clause{ *this } &Clause{ clause };
		}
		CNFSentence operator&(Clause&& clause)const& {
			return Clause{ *this } &Clause{ std::move(clause) };
		}
		CNFSentence operator&(const Clause& clause)&& {
			return Clause{ std::move(*this) } &Clause{ clause };
		}
		CNFSentence operator&(Clause&& clause)&& {
			if ((*this).is_false() || clause.is_false()) {
				return CNFSentence(Literal(FALSE));
			}
			else if (this->is_true()) {
				return CNFSentence(std::move(clause));
			}
			else if (clause.is_true() || clause.includes_clause(*this)) {
				return CNFSentence(std::move(*this));
			}
			else if (*this == clause || this->includes_clause(clause)) {
				return CNFSentence(std::move(clause));
			}
			else {
				std::vector<Clause> res;
				res.reserve(2);
				if (*this < clause) {
					res.emplace_back(std::move(*this));
					res.emplace_back(std::move(clause));
				}
				else {
					res.emplace_back(std::move(clause));
					res.emplace_back(std::move(*this));
				}
				return std::move(res);
			}
		}

		//Clause AND CNF
		CNFSentence operator&(const CNFSentence& sentence)const& {
			return  Clause{ *this } &CNFSentence{ sentence };
		}
		CNFSentence operator&(CNFSentence&& sentence)const& {
			return CNFSentence{ std::move(sentence) } & Clause{ *this };
		}
		CNFSentence operator&(const CNFSentence& sentence)&& {
			return CNFSentence{ sentence } & Clause{ std::move(*this) };
		}
		CNFSentence operator&(CNFSentence&& sentence)&& {
			return CNFSentence{ std::move(sentence) } &Clause{ std::move(*this) };
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
			return ~Clause{ *this };
		}
		CNFSentence operator~()&& {
			if (this->is_true()) {
				return CNFSentence(Literal(FALSE));
			}
			else if (this->is_false()) {
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

		bool operator==(const Clause& clause) const {
			return literals == clause.literals;
		}
		bool operator!=(const Clause& lit) const {
			return !(*this == lit);
		}
		bool operator<(const Clause& clause) const {
			return literals < clause.literals;
		}
		bool operator!()const {
			return is_false();
		}
		operator bool()const {
			return is_true();
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

	private:
		Clause(const std::vector<Literal>& lits) :Clause(std::vector<Literal>(lits)) {}
		Clause(std::vector<Literal>&& lits) :literals(std::move(lits)) {}
		bool includes_clause(const Clause& clause) {
			return std::includes(literals.begin(), literals.end(), clause.literals.begin(), clause.literals.end());
		}
		bool is_true() const {
			return literals.size() == 1 && literals[0].is_true();
		}
		bool is_false() const {
			return literals.size() == 1 && literals.front().is_false();
		}
		size_t size() const {
			return literals.size();
		}
		std::vector<Literal> literals;
		friend Literal;
	};
public:
	using CNFSentence = Clause::CNFSentence;
	Literal(std::string name, bool negation = false) :
		name(
			[&]()
			{
				if (name[0] == '~') name.erase(name.begin());
				return name;
			}()),
		negation(negation)
	{}

	//Literal AND Literal
	CNFSentence operator&(const Literal& lit)const& {
		return Literal{ *this } &Literal{ lit };
	}
	CNFSentence operator&(Literal&& lit)const& {
		return Literal{ *this } &Literal{ std::move(lit) };
	}
	CNFSentence operator&(const Literal& lit)&& {
		return Literal{ std::move(*this) } &Literal{ lit };
	}
	CNFSentence operator&(Literal&& lit)&& {
		if (this->is_false() || lit.is_false() || this->is_complementary(lit)) {
			return CNFSentence(Literal(FALSE));
		}
		else if (lit.is_true()) {
			return CNFSentence(std::move(*this));
		}
		else if (this->is_true()) {
			return CNFSentence(std::move(lit));
		}
		else if (*this == lit) {
			return CNFSentence(std::move(*this));
		}
		else {
			std::vector<Clause> res;
			res.reserve(2);
			if (*this < lit) {
				res.emplace_back(*this);
				res.emplace_back(lit);
			}
			else {
				res.emplace_back(lit);
				res.emplace_back(*this);
			}
			return res;
		}
	}

	//Literal AND Clause
	CNFSentence operator&(const Clause& clause)const& {
		return Clause{ clause } & Literal{ *this };
	}
	CNFSentence operator&(Clause&& clause)const& {
		return Clause{ std::move(clause) } & Literal{ *this };
	}
	CNFSentence operator&(const Clause& clause)&& {
		return Clause{ clause } & Literal{ std::move(*this) };
	}
	CNFSentence operator&(Clause&& clause)&& {
		return Clause{ std::move(clause) } & Literal{ std::move(*this) };
	}

	//Literal AND CNF
	CNFSentence operator&(const CNFSentence& clause)const& {
		return CNFSentence{ clause } & Literal{ *this };
	}
	CNFSentence operator&(CNFSentence&& clause)const& {
		return CNFSentence{ std::move(clause) } & Literal{ *this };
	}
	CNFSentence operator&(const CNFSentence& clause)&& {
		return CNFSentence{ clause } & Literal{ std::move(*this) };
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
		if (this->is_complementary(lit) || this->is_true() || lit.is_true()) {
			return Literal(TRUE);
		}
		else if (*this == lit || lit.is_false()) {
			return std::move(*this);
		}
		else if (this->is_false()) {
			return std::move(lit);
		}
		else {
			std::vector<Literal> res;
			res.reserve(2);
			if (*this < lit) {
				res.emplace_back(std::move(*this));
				res.emplace_back(std::move(lit));
			}
			else {
				res.emplace_back(std::move(lit));
				res.emplace_back(std::move(*this));
			}
			return res;
		}
	}

	//Literal OR Clause
	Clause operator|(const Clause& lit)const& {
		return  Clause{ lit } | Literal{ *this };
	}
	Clause operator|(Clause&& lit)const& {
		return  Clause{ std::move(lit) } | Literal{ *this };
	}
	Clause operator|(const Clause& lit)&& {
		return  Clause{ lit } | Literal{ std::move(*this) };
	}
	Clause operator|(Clause&& lit)&& {
		return Clause(std::move(lit)) | Literal(std::move(*this));
	}

	//Literal OR CNF
	CNFSentence operator|(const CNFSentence& lit)const& {
		return CNFSentence{ lit } | Literal{ *this };
	}
	CNFSentence operator|(CNFSentence&& lit)const& {
		return CNFSentence{ std::move(lit) } | Literal{ *this };
	}
	CNFSentence operator|(const CNFSentence& lit)&& {
		return CNFSentence{ lit } | Literal{ std::move(*this) };
	}
	CNFSentence operator|(CNFSentence&& lit)&& {
		return CNFSentence(std::move(lit)) | Literal(std::move(*this));
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
			return Literal(FALSE);
		}
		else if (name == FALSE) {
			return Literal{ TRUE };
		}
		return { std::move(name), !negation };
	}

	bool operator==(const Literal& lit) const {
		return name == lit.name && negation == lit.negation;
	}
	bool operator!=(const Literal& lit) const {
		return !(*this == lit);
	}
	bool operator<(const Literal& lit) const {
		if (name == lit.name) {
			return negation < lit.negation;
		}
		return name < lit.name;
	}
	operator bool()const {
		return name == TRUE;
	}
	bool operator!()const {
		return !*this;
	}

	friend std::ostream& operator<<(std::ostream& os, const Literal& lit) {
		if (lit.negation) {
			os << '~';
		}
		return os << lit.name;
	}
private:
	bool is_complementary(const Literal& lit) const {
		return name == lit.name && negation != lit.negation
			|| name == TRUE && lit.name == FALSE
			|| name == FALSE && lit.name == TRUE;
	}
	bool is_complementary(const Clause& clause) const {
		return clause.size() == 1 && clause.literals[0].is_complementary(*this);
	}
	bool is_complementary(const CNFSentence& sentence) const {
		return sentence.size() == 1 && this->is_complementary(sentence.clauses[0]);
	}
	bool is_true() const {
		return name == TRUE;
	}
	bool is_false() const {
		return name == FALSE;
	}
	std::string name;
	bool negation = false;
};

using Clause = Literal::Clause;
using CNFSentence = Literal::CNFSentence;