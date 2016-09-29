#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <Windows.h>

using namespace std;

const map<string, string> HELP = {
	{ "help", "Displays helpful information" },
	{ "help COMMAND", "Displays helpful information about selected command" },
	{ "exit", "Exits console" },
	{ "current", "Displays information about currently selected project" },
	{ "reserved", "Shows reserved paths" },
	{ "path", "Shows currently selected path" },
	{ "path RESERVED_PATH", "Selecting reserved path" },
	{ "dir NAME", "Creates new or opens already existing directory by the current path" },
	{ "project NAME", "Selecting project in current path, even if project does not exist" },
	{ "load", "Loading existing selected project otherwise creating new project" },
	{ "save", "Saving loaded project" },
	{ "saveas NAME", "Saving loaded project with new name" },
	{ "explorer", "Opens current directory in explorer" },
	{ "explorer RESERVED_PATH", "Opens reserved path in explorer without selecting it" },
	{ "explorer current", "Opens current project directory" },
	{ "generate NAME1 NAME2 ... NAMEN", "Generating projects with listed names in current directory" },
};

enum Command {
	cmd_Unknown,
	cmd_Empty,
	cmd_Help,
	cmd_Exit,
	cmd_Current,
	cmd_Reserved,
	cmd_Path,
	cmd_Dir,
	cmd_Project,
	cmd_Load,
	cmd_Save,
	cmd_SaveAs,
	cmd_Explorer,
	cmd_Generate,
};

const map<string, Command> CMD_ASSOCIATION = {
	{ "", cmd_Empty },
	{ "help", cmd_Help },
	{ "exit", cmd_Exit },
	{ "current", cmd_Current },
	{ "reserved", cmd_Reserved },
	{ "path", cmd_Path },
	{ "dir", cmd_Dir },
	{ "project", cmd_Project },
	{ "load", cmd_Load },
	{ "save", cmd_Save },
	{ "saveas", cmd_SaveAs },
	{ "explorer", cmd_Explorer },
	{ "generate", cmd_Generate },
};

string bin = "..\\bin\\";

string toString(int n) {
	string result;
	do
		result += (n % 10) + '0';
	while (n /= 10);
	reverse(result.begin(), result.end());
	return result;
}

string dirUp(const string& path) {
	string result = path;
	size_t i = 1;
	while (i < path.size() && path[path.size() - i - 1] != '\\')
		++i;
	result.resize(result.size() - i);
	return result;
}

string onlyProjectName(const string& fullname) {
	string result;
	size_t i = fullname.size() - 1;
	if (fullname[i] == '\\')
		--i;
	while (~i && fullname[i] != '\\')
		result += fullname[i--];
	reverse(result.begin(), result.end());
	return result;
}

string cutTheBin(const string& fullname) {
	for (size_t i = 0; i < bin.size(); ++i)
		if (bin[i] != fullname[i])
			return fullname;
	string result;
	result.reserve(fullname.size() - bin.size());
	for (size_t i = bin.size(); i < fullname.size(); ++i)
		result += fullname[i];
	return result;
}

class Project {
public:
	string name;

	void read_current() {
		fstream(bin + "current_project.txt", ios::in) >> name;
		name = bin + name;
	}
	void write_current() {
		fstream(bin + "current_project.txt", ios::out) << cutTheBin(name);
	}
	void load_or_create(string fname) {
		if (!CreateDirectory(fname.c_str(), NULL) && GetLastError() == ERROR_ALREADY_EXISTS) {
			cout << "Loaded existed project ";
		}
		else {
			CopyFile((bin + "samples\\main_sample.h").c_str(), (fname + "main.h").c_str(), FALSE);
			CopyFile((bin + "samples\\limits.txt").c_str(), (fname + "limits.txt").c_str(), FALSE);
			CopyFile((bin + "samples\\empty.txt").c_str(), (fname + "input.txt").c_str(), FALSE);
			CopyFile((bin + "samples\\empty.txt").c_str(), (fname + "output.txt").c_str(), FALSE);
			CopyFile((bin + "samples\\empty.txt").c_str(), (fname + "big_output.txt").c_str(), FALSE);
			CopyFile((bin + "samples\\empty.txt").c_str(), (fname + "sample.in").c_str(), FALSE);
			CopyFile((bin + "samples\\empty.txt").c_str(), (fname + "sample.out").c_str(), FALSE);
			CopyFile((bin + "samples\\empty.txt").c_str(), (fname + "random_test_generator.txt").c_str(), FALSE);
			CopyFile((bin + "samples\\checker.h").c_str(), (fname + "checker.h").c_str(), FALSE);
			cout << "Created new project ";
		}
	}
	void generate(string path, vector<string>::iterator begin, vector<string>::iterator end) {
		auto iter = begin;
		while (iter != end) {
			string fname = path + *iter + "\\";
			load_or_create(fname);
			cout << fname;
			if (++iter != end)
				cout << '\n';
		}
	}
	void load() {
		load_or_create(name);
		CopyFile((name + "main.h").c_str(), (bin + "..\\multiproject\\main.h").c_str(), FALSE);
		CopyFile((name + "limits.txt").c_str(), (bin + "..\\multiproject\\limits.txt").c_str(), FALSE);
		CopyFile((name + "input.txt").c_str(), (bin + "..\\multiproject\\input.txt").c_str(), FALSE);
		CopyFile((name + "output.txt").c_str(), (bin + "..\\multiproject\\output.txt").c_str(), FALSE);
		CopyFile((name + "big_output.txt").c_str(), (bin + "..\\multiproject\\big_output.txt").c_str(), FALSE);
		CopyFile((name + "sample.in").c_str(), (bin + "..\\multiproject\\sample.in").c_str(), FALSE);
		CopyFile((name + "sample.out").c_str(), (bin + "..\\multiproject\\sample.out").c_str(), FALSE);
		CopyFile((name + "random_test_generator.txt").c_str(), (bin + "..\\multiproject\\random_test_generator.txt").c_str(), FALSE);
		CopyFile((name + "checker.h").c_str(), (bin + "..\\multiproject\\checker.h").c_str(), FALSE);
		write_current();
	}
	bool save(bool overwrite) {
		if (!CreateDirectory(name.c_str(), NULL) && !overwrite && GetLastError() == ERROR_ALREADY_EXISTS) {
			cout << "Project already exists. Do you really want to overwrite it? (y/n) ";
			char c;
			while (cin >> c, c != 'y' && c != 'n')
				;
			if (c == 'n')
				return false;
		}
		CopyFile((bin + "..\\multiproject\\main.h").c_str(), (name + "main.h").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\limits.txt").c_str(), (name + "limits.txt").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\input.txt").c_str(), (name + "input.txt").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\output.txt").c_str(), (name + "output.txt").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\big_output.txt").c_str(), (name + "big_output.txt").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\sample.in").c_str(), (name + "sample.in").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\sample.out").c_str(), (name + "sample.out").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\random_test_generator.txt").c_str(), (name + "random_test_generator.txt").c_str(), FALSE);
		CopyFile((bin + "..\\multiproject\\checker.h").c_str(), (name + "checker.h").c_str(), FALSE);
		CopyFile((bin + "..\\Debug\\multiproject.exe").c_str(), (name + onlyProjectName(name) + ".exe").c_str(), FALSE);
		CopyFile((bin + "..\\Release\\multiproject.exe").c_str(), (name + onlyProjectName(name) + ".exe").c_str(), FALSE);
		CopyFile((bin + "..\\x64\\Debug\\multiproject.exe").c_str(), (name + onlyProjectName(name) + ".exe").c_str(), FALSE);
		CopyFile((bin + "..\\x64\\Release\\multiproject.exe").c_str(), (name + onlyProjectName(name) + ".exe").c_str(), FALSE);
		write_current();
		cout << "Succesfully saved project ";
		return true;
	}
};

class Console {
public:
	Console() {
		project = Project();
		project.read_current();
	}
	void read(const string& line) {
		console.clear();
		console.push_back("");
		auto iter = line.begin();
		while (iter != line.end() && *iter == ' ')
			++iter;
		for (iter = line.begin(); iter != line.end(); ++iter) {
			if (*iter == ' ') {
				console.push_back("");
				while (iter != line.end() && *iter == ' ')
					++iter;
			}
			if (iter == line.end())
				break;
			console.back() += *iter;
		}
	}
	bool empty() const {
		return console.empty();
	}
	size_t size() const {
		return console.size();
	}
	const string& command() const {
		return console[0];
	}
	const string& param(size_t n) const {
		return console[n + 1];
	}
	const string& path() const {
		return _path;
	}
	void setpath(const string& path) {
		_path = bin + cutTheBin(path);
	}
	const string& name() const {
		return project.name;
	}
	void setname(const string& name) {
		if (name != "")
			project.name = _path + name + '\\';
		else
			project.name = "";
	}
	void load() {
		project.load();
	}
	void generate() {
		project.generate(_path, console.begin() + 1, console.end());
	}
	bool save(bool overwrite) {
		return project.save(overwrite);
	}
private:
	vector<string> console;
	string _path = bin + "projects\\";
	Project project;
};

void load_reserved_paths(vector<pair<string, string>>& paths) {
	paths.clear();
	ifstream is(bin + "reserved_paths.txt");
	string s1, s2;
	while (is >> s1 >> s2)
		paths.emplace_back(s1, s2);
}

void find_bin() {
	ifstream is("bin.txt");
	if (is.is_open())
		getline(is, bin);
	is.close();
}