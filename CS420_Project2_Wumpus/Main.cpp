#include "Literal.h"
#include <chrono>


int main() {
	Literal a{ "A" };
	Literal b{ "B" };
	Literal c{ "C" };
	Literal d{ "D" };
	Literal e{ "E" };
	Literal f{ "F" };
	CNFSentence s1{ (a & b & c & d) | (a & b & e & f) };
	/*std::cout << s1 << "\n\n";
	std::cout << ~s1 << "\n\n";*/
	CNFSentence s2{ (b & f & d) | (b & c & a) };
	std::cout << s1 << "\n\n";
	CNFSentence o{ ~s1 };
	std::cout << o << "\n\n";
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < 1000; ++i) {
		o = std::move(o) | ~s1;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cin.get();
	return 0;
}