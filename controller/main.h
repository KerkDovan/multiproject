#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <Windows.h>
#include <shellapi.h>

#undef max
#undef min

using namespace std;

const LONG CHAR_WIDTH = 8;
const LONG CHAR_HEIGHT = 16;

const map<string, string> HELP = {
	{ "help", "Displays helpful information" },
	{ "help COMMAND", "Displays helpful information about selected command" },
	{ "exit", "Exits console" },
	{ "current", "Displays information about currently selected project" },
	{ "reserved", "Shows reserved paths" },
	{ "path", "Shows currently selected path" },
	{ "path RESERVED_PATH", "Selecting reserved path" },
	{ "cd", "Shows current directory" },
	{ "cd NAME", "Changes current directory to selected" },
	{ "md NAME", "Tries to create new directory" },
	{ "dir NAME", "Creates new or opens already existing directory by the current path" },
	{ "project NAME", "Selecting project in current path, even if project does not exist" },
	{ "load", "Loading existing selected project otherwise creating new project" },
	{ "push", "Pushes current project to buffer stack" },
	{ "push ALIAS", "Assigns alias to current project and adds it's working copy to buffer heap" },
	{ "get ALIAS", "Loads working copy from buffer heap" },
	{ "pop", "Loads working copy from buffer stack" },
	{ "pop ALIAS", "Calls \"get ALIAS\", deassigns alias to project and removes it's copy from buffer" },
	{ "buffer", "Calls \"buffer stack\" and \"buffer heap\"" },
	{ "buffer heap", "Shows all active aliases and projects they assigned to" },
	{ "buffer stack", "Shows stack of project, stored in buffer" },
	{ "buffer current", "Shows all active aliases assigned to current project" },
	{ "save", "Saving loaded project" },
	{ "saveas NAME", "Saving loaded project with new name" },
	{ "explorer", "Opens current directory in explorer" },
	{ "explorer RESERVED_PATH", "Opens reserved path in explorer without selecting it" },
	{ "explorer current", "Opens current project directory" },
	{ "list", "Displays a list of files and subdirectories in a directory" },
	{ "list NAME", "Displays a list of files and subdirectories in a directory" },
	{ "clear", "Clears the screen" },
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
	cmd_Cd,
	cmd_Md,
	cmd_Dir,
	cmd_Project,
	cmd_Load,
	cmd_Push,
	cmd_Get,
	cmd_Pop,
	cmd_Buffer,
	cmd_Save,
	cmd_SaveAs,
	cmd_Explorer,
	cmd_List,
	cmd_Clear,
	cmd_Generate,
};

const map<string, Command> CMD_ASSOCIATION = {
	{ "", cmd_Empty },
	{ "help", cmd_Help },
	{ "exit", cmd_Exit },
	{ "current", cmd_Current },
	{ "reserved", cmd_Reserved },
	{ "path", cmd_Path },
	{ "cd", cmd_Cd },
	{ "md", cmd_Md },
	{ "dir", cmd_Dir },
	{ "project", cmd_Project },
	{ "load", cmd_Load },
	{ "push", cmd_Push },
	{ "get", cmd_Get },
	{ "pop", cmd_Pop },
	{ "buffer", cmd_Buffer },
	{ "save", cmd_Save },
	{ "saveas", cmd_SaveAs },
	{ "explorer", cmd_Explorer },
	{ "list", cmd_List },
	{ "clear", cmd_Clear },
	{ "generate", cmd_Generate },
};

string exec(const char* cmd) {
	char buffer[128];
	string result = "";
	shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe)
		throw runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer, 128, pipe.get()) != NULL)
			result += buffer;
	}
	return result;
}

inline string exec(const string& cmd) {
	return exec(cmd.c_str());
}

inline int _cdecl system(const string& cmd) {
	return system(cmd.c_str());
}

inline bool exist(const string& name) {
	return exec("if exist \"" + name + "\" echo exists") == "exists\n";
}

inline bool isProject(const string& name) {
	return exist(name) && exist(name + "main.h");
}

const struct Color {
	Color(WORD clr) : color(clr) {}
	WORD color;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	friend ostream& operator << (ostream& os, const Color& color) {
		SetConsoleTextAttribute(color.handle, color.color);
		return os;
	}
}
clBlack			(0x0),
clBlue			(0x1),
clGreen			(0x2),
clAqua			(0x3),
clRed			(0x4),
clPurple		(0x5),
clYellow		(0x6),
clWhite			(0x7),
clGray			(0x8),
clLightBlue		(0x9),
clLightGreen	(0xA),
clLightAqua		(0xB),
clLightRed		(0xC),
clLightPurple	(0xD),
clLightYellow	(0xE),
clBrightWhite	(0xF);

vector<string> split(const string& str, const string& separators = "\n\r") {
	vector<string> result(1, "");
	for (char c : str) {
		if (find(separators.begin(), separators.end(), c) != separators.end()) {
			if (result.back() != "")
				result.emplace_back("");
			continue;
		}
		result.back() += c;
	}
	if (result.back() == "")
		result.pop_back();
	return result;
}

string splitToColumns(const string& str, size_t width = 800) {
	if (str == "")
		return "";
	const size_t cutting = width / CHAR_WIDTH - 4;
	vector<string> names = split(str);
	size_t max_size = 0;
	for (string& name : names) {
		if (name.size() > cutting) {
			name.resize(cutting - 3 - 1);
			name += "...";
		}
		name = "    " + name;
		max_size = max(max_size, name.size());
	}
	sort(names.begin(), names.end());
	size_t rows_cnt;
	vector<size_t> cols_widths;
	for (rows_cnt = 1; rows_cnt <= names.size(); ++rows_cnt) {
		vector<size_t> cw;
		for (size_t i = 0; i < names.size(); ++i) {
			if (i % rows_cnt == 0) {
				cw.emplace_back(0);
			}
			cw.back() = max(cw.back(), names[i].size());
		}
		size_t sum_width = 0;
		for (size_t w : cw)
			sum_width += w;
		if (sum_width * CHAR_WIDTH <= width) {
			swap(cols_widths, cw);
			break;
		}
	}
	string result;
	result.reserve(rows_cnt * width);
	size_t cols_cnt = cols_widths.size();
	for (size_t r = 0; r < rows_cnt; ++r) {
		for (size_t c = 0; c < cols_cnt; ++c) {
			size_t id = r + c * rows_cnt;
			if (id < names.size()) {
				names[id].resize(cols_widths[c]);
				result += names[id];
			}
		}
		result += '\n';
	}
	return result;
}

inline void copyFile(const string& source, const string& destination) {
	CopyFile(source.c_str(), destination.c_str(), FALSE);
}

void copyDirectory(const string& source, const string& destination) {
	if (!exist(destination))
		CreateDirectory(destination.c_str(), NULL);
	vector<string> dirs, files;
	dirs = split(exec("dir /A:D /B \"" + source + "\" 2>nul"));
	files = split(exec("dir /A:-D /B \"" + source + "\" 2>nul"));
	for (const string& dir : dirs) {
		CreateDirectory((destination + dir + '\\').c_str(), NULL);
		copyDirectory(source + dir + '\\', destination + dir + '\\');
	}
	for (const string& file : files)
		copyFile(source + file, destination + file);
}

inline void deleteDirectory(const string& directory, const string& args = "/s /q") {
	system("rmdir " + directory + args);
}

string bin = "..\\bin\\";

bool isNameValid(const string& name) {
	static const set<string::value_type> forbidden = []() { 
		set<string::value_type> result;
		for (char i = 0; i < 32; ++i)
			result.emplace(i);
		string tmp = "<>:\"/\\|?*";
		for (char c : tmp)
			result.emplace(c);
		return result;
	}();
	for (char c : name)
		if (forbidden.find(c) != forbidden.end())
			return false;
	return true;
}

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
			cout << "Loaded existing project ";
		}
		else {
			copyFile(bin + "samples\\main_sample.h", fname + "main.h");
			copyFile(bin + "samples\\config_project.h", fname + "config_project.h");
			copyFile(bin + "samples\\empty.txt", fname + "input.txt");
			copyFile(bin + "samples\\empty.txt", fname + "tmp_input.txt");
			copyFile(bin + "samples\\empty.txt", fname + "output.txt");
			copyFile(bin + "samples\\empty.txt", fname + "big_output.txt");
			copyFile(bin + "samples\\empty.txt", fname + "checker_verdict.txt");
			copyFile(bin + "samples\\empty.txt", fname + "debug_output.txt");
			copyFile(bin + "samples\\empty.txt", fname + "sample.in");
			copyFile(bin + "samples\\empty.txt", fname + "sample.out");
			copyFile(bin + "samples\\empty.txt", fname + "random_test_generator.txt");
			copyFile(bin + "samples\\checker.h", fname + "checker.h");
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
		// Copying project files from it's directory
		deleteDirectory(bin + "..\\multiproject\\project\\");
		copyDirectory(name, bin + "..\\multiproject\\project\\");
		write_current();
	}
	bool save(bool overwrite) {
		if (exist(name) && exist(name + "main.h") && !overwrite) {
			cout << "Project already exists. Do you really want to overwrite it? (y/n) ";
			char c;
			while (cin >> c, c != 'y' && c != 'n')
				;
			if (c == 'n')
				return false;
		}
		// Copying project files to it's directory
		deleteDirectory(name);
		copyDirectory(bin + "..\\multiproject\\project\\", name);
		// Copying executable to project directory
		copyFile(bin + "..\\Debug\\multiproject.exe", name + onlyProjectName(name) + "_x86Debug.exe");
		copyFile(bin + "..\\Release\\multiproject.exe", name + onlyProjectName(name) + "_x86Release.exe");
		copyFile(bin + "..\\x64\\Debug\\multiproject.exe", name + onlyProjectName(name) + "_x64Debug.exe");
		copyFile(bin + "..\\x64\\Release\\multiproject.exe", name + onlyProjectName(name) + "_x64Release.exe");
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
	string path() const {
		return bin + cutTheBin(_path);
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

/***************************************************************************

***************************************************************************/

string make_slashes(const string& line, char slash) {
	string result = line;
	for (char& c : result)
		if (c == '\\' || c == '/')
			if (c != slash)
				c = slash;
	return result;
}

int tosystem(const string& cmd) {
	return system(make_slashes(cmd, '\\'));
}

vector<pair<string, string>> getFilenames(const string& path) { // [] < name, path >
	const string DIR_FILE = "!!!___!!!___!!!___dir.txt";
	const string DIRS = path + DIR_FILE;
	const char* DIR = DIRS.c_str();
	const string CMD = "DIR \"" + path + "\" > \"" + DIR + "\"";
	system(CMD);
	ifstream fin(DIR);
	string line;
	for (int i = 0; i < 7; ++i)
		getline(fin, line);
	vector<string> lines;
	while (getline(fin, line))
		lines.emplace_back(line);
	if (lines.size() > 1)
		lines.resize(lines.size() - 2);
	vector<pair<string, string>> result;
	for (const string& l : lines) {
		if (l[35] <= 32 || l[35] > 127)
			continue;
		const string& PATH = path;
		string name;
		name.reserve(l.size() - 35);
		for (size_t i = 35; i != l.size(); ++i)
			name += l[i];
		if (name == DIR_FILE)
			continue;
		decltype(result) tmp;
		if (l[20] == '<')
			tmp = getFilenames(PATH + name + '/');
		else
			result.emplace_back(name, PATH);
		for (auto& p : tmp)
			result.emplace_back(p);
	}
	fin.close();
	tosystem("DEL \"" + DIRS + "\"");
	return result;
}
