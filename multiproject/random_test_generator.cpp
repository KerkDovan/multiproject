#include "random_test_generator.h"

#include <chrono>

using namespace std;

ifstream fin;
ofstream fout;

vector<string> code;

void parse_code() {
	code.clear();
	char c;
	while (fin >> c) {
		if (isspace(c)) {
			if (code.empty() || code.back() != "")
				code.push_back("");
			continue;
		}
		code.back() += c;
	}
}

unsigned int seed;

void set_seed(unsigned int new_seed) {
	srand(seed = new_seed);
}

void set_seed(void) {
	srand(seed = (unsigned int)time(NULL));
}



int random_test_generator() {
	fin.open("random_test_generator.txt");
	fout.open("tmp_input.txt");

	return 1; // TODO: create RTG language, make compiler

	//return 0;
}
