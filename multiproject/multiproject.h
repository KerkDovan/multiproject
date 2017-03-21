#pragma once
#ifndef _MULTIPROJECT_MULTIPROJECT_H_
#define _MULTIPROJECT_MULTIPROJECT_H_

#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include "tools.h"

#ifndef _USE_DEBUG
#define _USE_DEBUG (true && _DEBUG)
#endif

#if defined (_WIN32) || defined (__WIN32__) || defined (_WIN64) || defined (__WIN64__)
#define popen _popen
#define pclose _pclose
#endif

#if _USE_DEBUG // enable/disable debug
#ifdef __GNUC__
#define breakpoint __asm("int $3")
#elif defined _MSC_VER
#define breakpoint __asm { int 3 }
#else
#warning "Cannot define 'breakpoint'"
#define breakpoint
#endif
#define breakpoint_if(condition) if (condition) breakpoint;
#define breakpoint_counters project::tools::debugging::BreakpointCounters
#define breakpoint_counter(name) breakpoint_counters[stringify(name)]
#define breakpoint_counter_init(name, initial) \
	breakpoint_counter(name) = static_cast<long long>(initial);
#define breakpoint_counter_offset(name, offset) \
	breakpoint_counter(name) += static_cast<long long>(offset);
#define breakpoint_when(counter_name, equal_to) breakpoint_if( \
	breakpoint_counter(counter_name) == static_cast<long long>(equal_to) );
#define breakpoint_when_offset(counter_name, equal_to, offset) \
	breakpoint_when(counter_name, equal_to); breakpoint_counter_offset(counter_name, offset);
#define breakpoint_counters_clear breakpoint_counters.clear();
#define debug_output(name, ...) { debug << "Debug output" << (name[0] ? ": " : "") << name \
	<< "\n    in file " << __FILE__ << "\n    on line " << __LINE__ \
	<< "\n    in function " << __func__ << '\n'; \
	output_variadic_arguments( \
	get_names_of_variables(stringify(__VA_ARGS__)), \
	0, __VA_ARGS__); debug << std::endl; }
#else
#define breakpoint
#define breakpoint_if(...)
#define breakpoint_counters
#define breakpoint_counter(...)
#define breakpoint_counter_init(...)
#define breakpoint_counter_offset(...)
#define breakpoint_when(...)
#define breakpoint_when_offset(...)
#define breakpoint_counters_clear
#define debug_output(...)
#endif

#define only_multiproject(...) __VA_ARGS__
#define freopen_in(...) project::cin.open(project::tools::selectProjectFile(MPF_tmp_input)); \
	project::std_in = fopen(project::tools::selectProjectFile(MPF_tmp_input), "r");
#define freopen_out(...) project::cout.open(project::tools::selectProjectFile(MPF_big_output)); \
	project::tools::debugging::debug.open(project::tools::selectProjectFile(MPF_debug_output)); \
	project::std_out = fopen(project::tools::selectProjectFile(MPF_big_output), "w");
// TODO: Check std_in and std_out;
#define close_files project::cin.close(); project::cout.close(); \
	project::tools::debugging::debug.close(); if (project::std_in) fclose(project::std_in); \
	if (project::std_out) fclose(project::std_out);

#define scanf(...) fscanf(project::std_in, __VA_ARGS__)
#define printf(...) fprintf(project::std_out, __VA_ARGS__)
#define getchar() getc(std_in)

namespace project {

	std::ifstream cin;
	std::ofstream cout;
	FILE* std_in;
	FILE* std_out;

	//static unsigned long long _m_counter_ = 0;

	namespace tools {

		std::string exec(const char* cmd) {
			char buffer[128];
			std::string result = "";
			std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
			if (!pipe)
				throw std::runtime_error("popen() failed!");
			while (!feof(pipe.get())) {
				if (fgets(buffer, 128, pipe.get()) != NULL)
					result += buffer;
			}
			return result;
		}

		std::string exec(const std::string& cmd) {
			return exec(cmd.c_str());
		}

		bool exist(const std::string& name) {
			return exec("if exist \"" + name + "\" echo exists") == "exists\n";
		}

		enum MultiprojectProjectFiles {
			MPF_checker_verdict = 0,
			MPF_debug_output = 1,
			MPF_tmp_input = 2,
			MPF_big_output = 3,
			MPF_input = 4,
			MPF_output = 5,
			MPF_random_big_output = 6,
			MPF_random_output = 7,
			MPF_random_test_generator = 8,
			MPF_samplein = 9,
			MPF_sampleout = 10,
		};

		const char* MULTIPROJECT_PROJECT_FILES[][2] = {
			{ "project\\checker_verdict.txt", "checker_verdict.txt" },
			{ "project\\debug_output.txt", "debug_output.txt" },
			{ "project\\tmp_input.txt", "tmp_input.txt" },
			{ "project\\big_output.txt", "big_output.txt" },
			{ "project\\input.txt", "input.txt" },
			{ "project\\output.txt", "output.txt" },
			{ "project\\random_big_output.txt", "random_big_output.txt" },
			{ "project\\random_output.txt", "random_output.txt" },
			{ "project\\random_test_generator.txt", "random_test_generator.txt" },
			{ "project\\sample.in", "sample.in" },
			{ "project\\sample.out", "sample.out" },
		};

		const char* selectProjectFile(MultiprojectProjectFiles file) {
			return exist(MULTIPROJECT_PROJECT_FILES[file][0]) ?
				MULTIPROJECT_PROJECT_FILES[file][0] :
				MULTIPROJECT_PROJECT_FILES[file][1];
		}

		namespace debugging {

			std::map<std::string, long long> BreakpointCounters;

#if _USE_DEBUG
			std::ofstream debug;
#else
			struct MultiprojectDebug {
				void open(const std::string&) {}
				void close() {}
				template<class _Ty> friend MultiprojectDebug&
					operator << (MultiprojectDebug& db, const _Ty&) { return db; }
			} debug;
#endif // _USE_DEBUG_

			std::vector<std::string> get_names_of_variables(const char* names) {
				std::vector<std::string> result;
				result.push_back("");
				bool comma = true;
				for (int i = 0; names[i]; ++i) {
					char c = names[i];
					if (comma) {
						if (c == ' ')
							continue;
						comma = false;
					}
					if (c == ',') {
						result.push_back("");
						comma = true;
					}
					else
						result.back().push_back(c);
				}
				return result;
			}

			template<typename _Head>
			void output_variadic_arguments (
				const std::vector<std::string>& names,
				size_t id,
				const _Head& head
			) {
				debug << names[id] << " == " << std::boolalpha << head
					<< " (" << typeid(head).name() << ")\n";
			}

			template<typename _Head, typename... _Tail>
			void output_variadic_arguments(
				const std::vector<std::string>& names,
				size_t id,
				const _Head& head,
				const _Tail&... tail
			) {
				debug << names[id] << " == " << std::boolalpha << head
					<< " (" << typeid(head).name() << ")\n";
				output_variadic_arguments(names, id + 1, tail...);
			}

			namespace format {

				template<class _Ty>
				using FormatterType = std::ostream&(*)(std::ostream&, const _Ty&);

				template<class _Ty, class _FTy>
				class FormatDebugOutput {
				public:
					FormatDebugOutput(_Ty& obj, const _FTy& format)
						: obj(obj), formatter(format) {}

					friend std::ostream& operator << (std::ostream& os, const FormatDebugOutput<_Ty, _FTy>& fdo) {
						return fdo.formatter(os, fdo.obj);
					}
				private:
					_Ty& obj;
					_FTy formatter;
				};

				template<class _Ty, class _FTy>
				FormatDebugOutput<_Ty, _FTy> format_debug(_Ty& obj, const _FTy& formatter) {
					return FormatDebugOutput<_Ty, _FTy>(obj, formatter);
				}

				template<class _Ty>
				struct FormatVector {
				public:
					FormatVector(const std::string& format) : format(format) {}
					std::ostream& operator () (std::ostream& os, const std::vector<_Ty>& vect) const {
						bool ctrl = false;
						for (size_t i = 0; i < vect.size(); ++i) {
							for (char c : format) {
								if (ctrl) {
									switch (c) {
									case 'i': os << i; break;
									case 'e': os << vect[i]; break;
									default: os << c;
									}
									ctrl = false;
								}
								else if (c == '%')
									ctrl = true;
								else
									os << c;
							}
						}
						return os;
					}
				private:
					std::string format;
				};

			}

		}

	}

} // namespace project

#endif // _MULTIPROJECT_MULTIPROJECT_H_
