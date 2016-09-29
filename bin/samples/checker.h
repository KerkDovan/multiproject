#pragma once
#ifndef _MULTIPROJECT_CHECKER_H_
#define _MULTIPROJECT_CHECKER_H_

#include "random_test_generator.h"

#include <fstream>

extern int random_test_generator(void);



int checker() {
	std::ifstream input("tmp_input.txt");
	std::ifstream output("big_output.txt");
	std::ofstream verdict("checker_verdict.txt");

	

	return 0;
}

#endif // _MULTIPROJECT_CHECKER_H_