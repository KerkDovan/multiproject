#include "project/config_project.h"

#if _COMPILE_WITHOUT_MULTIPROJECT

#include "project/main.h"

#else // _COMPILE_WITHOUT_MULTIPROJECT

#define _LOCAL_MULTIPROJECT_

#include "project/main.h"
#include "tools.h"
#include "random_test_generator.h"
//#include "project/checker.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#if defined _WIN32 || defined _WIN64
#include <conio.h>
#include <process.h>
#include <Windows.h>
#endif

#ifndef _MULTIPROJECT_TIME_LIMIT
#define _MULTIPROJECT_TIME_LIMIT 1000
#endif

#ifndef _MULTIPROJECT_STACK_SIZE_MB
#define _MULTIPROJECT_STACK_SIZE_MB 256
#endif

#  if _MULTIPROJECT_STACK_SIZE_MB == 1024
#pragma comment(linker, "/STACK:1073741824")
#elif _MULTIPROJECT_STACK_SIZE_MB == 512
#pragma comment(linker, "/STACK:536870912")
#elif _MULTIPROJECT_STACK_SIZE_MB == 256
#pragma comment(linker, "/STACK:268435456")
#elif _MULTIPROJECT_STACK_SIZE_MB == 128
#pragma comment(linker, "/STACK:134217728")
#elif _MULTIPROJECT_STACK_SIZE_MB == 64
#pragma comment(linker, "/STACK:67108864")
#elif _MULTIPROJECT_STACK_SIZE_MB == 32
#pragma comment(linker, "/STACK:33554432")
#elif _MULTIPROJECT_STACK_SIZE_MB == 16
#pragma comment(linker, "/STACK:16777216")
#elif _MULTIPROJECT_STACK_SIZE_MB == 8
#pragma comment(linker, "/STACK:8388608")
#elif _MULTIPROJECT_STACK_SIZE_MB == 4
#pragma comment(linker, "/STACK:4194304")
#elif _MULTIPROJECT_STACK_SIZE_MB == 2
#pragma comment(linker, "/STACK:2097152")
#elif _MULTIPROJECT_STACK_SIZE_MB == 1
#pragma comment(linker, "/STACK:1048576")
#endif

using namespace std;

using std::cin;
using std::cout;

using project::tools::MultiprojectProjectFiles;
using project::tools::MPF_big_output;
using project::tools::MPF_checker_verdict;
using project::tools::MPF_debug_output;
using project::tools::MPF_input;
using project::tools::MPF_output;
using project::tools::MPF_random_big_output;
using project::tools::MPF_random_output;
using project::tools::MPF_random_test_generator;
using project::tools::MPF_samplein;
using project::tools::MPF_sampleout;
using project::tools::MPF_tmp_input;
using project::tools::selectProjectFile;

// file:///D:\My%20Data\Coding\VS%20Community\multiproject\x64\Debug
// file:///D:\My%20Data\Coding\VS%20Community\multiproject\multiproject
// file:///D:\My%20Data\Coding\VS%20Community\multiproject\bin

const uint32_t time_limit = _MULTIPROJECT_TIME_LIMIT;

const tuple<unsigned char, unsigned char, unsigned char> UTF_BOM = { 0xEF, 0xBB, 0xBF };

size_t getFileSize(string filename) {
	ifstream is(filename);
	string str = string(istreambuf_iterator<char>(is), istreambuf_iterator<char>());
	is.close();
	size_t result = str.size();
	if (result > 2)
		result -= ((make_tuple(str[0], str[1], str[2]) == UTF_BOM) ? 3 : 0);
	return result;
}

bool is_file_empty(string filename) {
	return !getFileSize(filename);
}

pair<size_t, size_t> first_difference(string f1, string f2) {
	ifstream is1(f1), is2(f2);
	string s1 = string(istreambuf_iterator<char>(is1), istreambuf_iterator<char>());
	string s2 = string(istreambuf_iterator<char>(is2), istreambuf_iterator<char>());
	is1.close();
	is2.close();
	size_t row = 1, colon = 1;
	auto iter1 = s1.begin(), iter2 = s2.begin();
	size_t size1 = s1.size(), size2 = s2.size();
	if (s1.size() > 2 && make_tuple(s1[0], s1[1], s1[2]) == UTF_BOM)
		++ ++ ++iter1, size1 -= 3;
	if (s2.size() > 2 && make_tuple(s2[0], s2[1], s2[2]) == UTF_BOM)
		++ ++ ++iter2, size2 -= 3;
	for (; iter1 != s1.end() && iter2 != s2.end(); ++iter1, ++iter2) {
		if (*iter1 == '\n')
			++row, colon = 1;
		else
			++colon;
		if (*iter1 != *iter2)
			return make_pair(row, colon);
	}
	if (size1 != size2)
		return make_pair(row, colon);
	return make_pair(-1, -1);
}

unsigned __stdcall project_main(void*) {
	return (unsigned)project::main();
}

bool tryExecute(bool sayAboutBigOutput) {
	(void)sayAboutBigOutput;

	HANDLE thread = (HANDLE)_beginthreadex(
		NULL,
		0,
		&project_main,
		NULL,
		0,
		NULL
	);

	DWORD verdict;

	clock_t time_start = clock();
	clock_t time_end;

	verdict = WaitForSingleObject(thread, time_limit);

	if (verdict == WAIT_TIMEOUT) {
		cout << "Time limit exceeded: " << time_limit << "ms. Do you want stop program? (y/n) ";
		char c = 0;
		while (cin >> c, c != 'y' && c != 'n');
		if (c == 'y') {
			TerminateThread(thread, 0);
			project::cin.close();
			project::cout.flush();
			project::cout.close();
			project::tools::debugging::debug.close();
		}
		else
			verdict = WaitForSingleObject(thread, INFINITE);
	}

	time_end = clock();

	if (verdict == WAIT_TIMEOUT)
		cout << "Time limit exceeded: " << time_limit << "ms\n";
	else if (verdict == WAIT_FAILED)
		cout << "Error occured. Error code: " << GetLastError() << '\n';
	else {
		cout << "Used time: " << time_end - time_start << "ms\n";
		return true;
	}

	return false;
}

void copy_file_if_small(const char* src, const char* dst) {
	if (getFileSize(src) > 1000 * 1000)
		ofstream(dst) << "Output is too big, see \"" << src << "\" file using NOT MS Visual Studio\n";
	else
		CopyFile(src, dst, FALSE);
}

int main() {

	cout << "Testing in ";

#ifdef _DEBUG
	cout << "Debug";
	DeleteFile("..\\x64\\Release\\multiproject.exe");
	DeleteFile("..\\Release\\multiproject.exe");
#else
	cout << "Release";
	DeleteFile("..\\x64\\Debug\\multiproject.exe");
	DeleteFile("..\\Debug\\multiproject.exe");
#endif

#ifdef _WIN64
	cout << " x64";
	DeleteFile("..\\Debug\\multiproject.exe");
	DeleteFile("..\\Release\\multiproject.exe");
#else
	cout << " x86";
	DeleteFile("..\\x64\\Debug\\multiproject.exe");
	DeleteFile("..\\x64\\Release\\multiproject.exe");
#endif

	cout << "\n\n";

	bool next_test = true, tested = false;



	if (!is_file_empty(selectProjectFile(MPF_samplein))) {
		tested = true;
		cout << "Testing samples\n";
		CopyFile(selectProjectFile(MPF_samplein), selectProjectFile(MPF_tmp_input), FALSE);
		if (tryExecute(false)) {
			pair<size_t, size_t> pos = first_difference(selectProjectFile(MPF_big_output), selectProjectFile(MPF_sampleout));
			if (pos == make_pair((size_t)-1, (size_t)-1))
				cout << "Samples: Accepted\n";
			else {
				cout << "Samples: Wrong Answer, see line " << pos.first << " colon " << pos.second - 1 << '\n';
				next_test = false;
			}
			copy_file_if_small(selectProjectFile(MPF_big_output), selectProjectFile(MPF_output));
		}
		else {
			cout << "Failed testing samples\n";
			next_test = false;
		}
	}
	else
		cout << "There are no samples, passing by\n";

	if (next_test) {
		if (!is_file_empty(selectProjectFile(MPF_random_test_generator))) {
			tested = true;
			cout << '\n';
			int code = 0;// = random_test_generator();
			if (!code) {
				cout << "Random tests generated\n";
				cout << "Testing random tests\n";
				tryExecute(false);
				//checker();
				CopyFile(selectProjectFile(MPF_big_output), selectProjectFile(MPF_random_big_output), FALSE);
				copy_file_if_small(selectProjectFile(MPF_random_big_output), selectProjectFile(MPF_random_output));
			}
			else
				cout << "Random test generating failure. Exit code: " << code << '\n';
		}
		else
			cout << "\nRandom test generator not found, passing by\n";

		if (!is_file_empty(selectProjectFile(MPF_input))) {
			tested = true;
			cout << "\nTesting other inputs\n";
			CopyFile(selectProjectFile(MPF_input), selectProjectFile(MPF_tmp_input), FALSE);
			tryExecute(true);
			copy_file_if_small(selectProjectFile(MPF_big_output), selectProjectFile(MPF_output));
		}
		else
			cout << "\nOther inputs not found, passing by\n";

		if (!tested) {
#if !_RUN_WITHOUT_INPUT
			char c;
			cout << "\nThere are no input files for testing. Do you want to "
				<< "start testing with out any input? (y/n)\n";
			while (cin >> c, c != 'y' && c != 'n')
				;
			if (c == 'y') {
#endif
				cout << "\nTesting with out any input\n";
				tryExecute(true);
				copy_file_if_small(selectProjectFile(MPF_big_output), selectProjectFile(MPF_output));
#if !_RUN_WITHOUT_INPUT
			}
#endif
		}
	}
	else
		cout << "\nSubsequent testing has been canceled\n";

	cout << '\n' << flush;
	std::system("pause");

	return 0;
}

#endif // _COMPILE_WITHOUT_MULTIPROJECT
