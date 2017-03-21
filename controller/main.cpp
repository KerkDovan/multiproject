#include "main.h"

Console console;

inline void showIntro() {
	cout << clBrightWhite;
	cout << "Multiproject console v.1.0\n";
	cout << "\nCurrent path: " << console.path();
	cout << "\nCurrent project: " << console.name();
}

int main() {

	auto window = GetConsoleWindow();
	tagWINDOWINFO windowInfo;

	find_bin();
	console = Console();
	console.setpath(bin + "projects\\");

	cout << clRed;

	showIntro();

	vector<pair<string, string>> reserved_paths;
	
	string line;
	bool to_exit = false;
	while (cout << clGreen << "\n\n>>> " << clWhite, cin >> ws, getline(cin, line)) {

		cout << clAqua;

		load_reserved_paths(reserved_paths);
		
		console.read(line);

		Command cmd = [&](auto iter) { 
			if (iter != CMD_ASSOCIATION.end())
				return iter->second;
			else
				return cmd_Unknown;
		} (CMD_ASSOCIATION.find(console.command()));

		string help = "";
		string oldname, to_open;
		size_t width;
		char c = 0;

		switch (cmd) {

		case cmd_Exit:
			if (console.size() > 1 && !(console.param(0) == "y" || console.param(0) == "n"))
				c = console.param(0)[0];
			if (c == 0) {
				cout << "Do you want to save the project? All unsaved data may be deleted "
					<< "when you load the project next time. (y/n) ";
				while (cin >> c, c != 'y' && c != 'n')
					;
			}
			if (c == 'n')
				return 0;
			if (console.name() == "")
				cout << "Choose project path and name using appropriate commands";
			else {
				if (!console.save(c == 'y'))
					cout << "Already existing project was not overwritten";
				else {
					cout << console.name();
					return 0;
				}
			}
			to_exit = true;
			break;

		case cmd_Help:
			if (console.size() == 1) {
				cout << "  Available commands:\n";
				help.clear();
				for (auto s : HELP)
					help += s.first + "\n";
				cout << splitToColumns(help);
			}
			else {
				string cmnd = console.param(0);
				for (size_t i = 1; i < console.size() - 1; ++i)
					cmnd += ' ' + console.param(i);
				auto result = HELP.find(cmnd);
				if (result != HELP.end())
					cout << result->second;
				else
					cout << "Unknown command: " << cmnd;
			}
			break;

		case cmd_Reserved:
			cout << "  Reserved paths:\n";
			for (auto p : reserved_paths)
				cout << "    " << p.first << " - " << p.second << '\n';
			break;

		case cmd_Path:
			if (console.size() != 1) {
				auto path = find_if(reserved_paths.begin(), reserved_paths.end(),
					[&](auto iter) { 
						return iter.first == console.param(0); 
					}
				);
				if (path == reserved_paths.end())
					cout << "Unknown path ";
				else
					console.setpath(path->second);
				console.setname("");
			}
			cout << console.path();
			break;

		case cmd_Dir:
			if (console.size() == 1 || console.size() == 2 && console.param(0) == ".") {
				cout << console.path();
				break;
			}
			if (console.param(0) == "..")
				console.setpath(dirUp(console.path()));
			else
				console.setpath(console.path() + console.param(0) + '\\');
			console.setname("");
			if (CreateDirectory(console.path().c_str(), NULL))
				cout << "Created new directory ";
			else {
				cout << "Opened existing directory ";
			}
			cout << console.path();
			break;

		case cmd_Project:
			console.setname(console.param(0));

		case cmd_Current:

		case cmd_Load:
			if (console.name() != "") {
				if (cmd == cmd_Load)
					console.load();
				cout << console.name() << '\n';
				cout << (isProject(console.name()) ?
					"Project exists" : exist(console.name()) ? 
					"Directory exists" : "Neither project nor directory exists");
			}
			else
				cout << "There is no project selected now";
			break;

		case cmd_Push:

			break;

		case cmd_Generate:
			console.generate();
			break;

		case cmd_SaveAs:
			oldname = console.name();
			console.setname(console.param(0));

		case cmd_Save:
			if (console.name() != "") {
				if (!console.save(console.size() > 1 && console.param(0) == "y"))
					cout << "Already existing project was not overwritten";
				else {
					cout << console.name();
					if (to_exit)
						return 0;
				}
			}
			else
				cout << "Cannot save project";
			break;

		case cmd_Explorer:
			to_open = "explorer \"";
			if (console.size() == 1)
				to_open += console.path();
			else if (console.param(0) == "current")
				to_open += console.name();
			else {
				auto path = find_if(reserved_paths.begin(), reserved_paths.end(),
					[&](auto iter) {
						return iter.first == console.param(0);
					}
				);
				if (path != reserved_paths.end())
					to_open += path->second;
				else {
					cout << "Unknown parameter: " << console.param(0);
					break;
				}
			}
			to_open += "\"";
			cout << to_open.c_str();
			system(to_open);
			break;

		case cmd_List:
			to_open = console.path();
			if (console.size() > 1)
				to_open += console.param(0);
			if (to_open.back() != '\\')
				to_open += '\\';
			if (!exist(to_open)) {
				cout << "Such directory not found";
				break;
			}
			GetWindowInfo(window, &windowInfo);
			width = windowInfo.rcClient.right - windowInfo.rcClient.left;
			cout << "Displayed the content of: " << to_open << "\n\n";
			cout << "----Projects----\n";
			cout << splitToColumns(exec("@echo off && for /f \"delims=\\\" %x in ('dir /A:D /B \"" + to_open +
				"\"') do (if exist \"" + to_open + "%x\\main.h\" echo %x)"), width);
			cout << "\n\n";
			cout << "----Directories----\n";
			cout << splitToColumns(exec("@echo off && dir /A:D /B \"" + to_open + "\" 2>nul"), width);
			cout << "\n\n";
			cout << "----Files----\n";
			cout << splitToColumns(exec("@echo off && dir /A:-D /B \"" + to_open + "\" 2>nul"), width);
			break;

		case cmd_Clear:
			system("cls");
			showIntro();
			break;

		case cmd_Unknown:
			cout << "Unknown command: " << console.command();

		case cmd_Empty:
			break;

		default:
			cout << "Command is not supported yet";

		}
	}
	
	return 0;
}