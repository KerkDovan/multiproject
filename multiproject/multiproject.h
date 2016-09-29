#pragma once
#ifndef _MULTIPROJECT_MULTIPROJECT_H_
#define _MULTIPROJECT_MULTIPROJECT_H_

#include "tools.h"

#if true & _DEBUG // enable/disable debug
#define breakpoint __asm { int 3 }
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
#define debug_output(name, ...) { debug << "Debug output " << name << (string(name).empty() ? "" : " ") \
	<< "on line " << __LINE__ << '\n'; \
	output_variadic_arguments(\
	get_names_of_variables(stringify(__VA_ARGS__)), \
	0, \
	__VA_ARGS__); debug << '\n'; }
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
#define freopen_in(...) project::cin.open("tmp_input.txt"); \
	project::std_in = fopen("tmp_input.txt", "r");
#define freopen_out(...) project::cout.open("big_output.txt"); \
	project::tools::debugging::debug.open("debug_output.txt"); \
	project::std_out = fopen("big_output.txt", "w");
#define close_files project::cin.close(); project::cout.close(); \
	project::tools::debugging::debug.close(); fclose(std_in); fclose(std_out);

#define scanf(...) fscanf(project::std_in, __VA_ARGS__)
#define printf(...) fprintf(project::std_out, __VA_ARGS__)
#define getchar() getc(std_in)

namespace project {

	std::ifstream cin;
	std::ofstream cout;
	FILE* std_in;
	FILE* std_out;

	static unsigned long long _m_counter_ = 0;
	
	namespace tools {

		namespace debugging {

			std::map<std::string, long long> BreakpointCounters;

			std::ofstream debug;

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

			template<typename _Head, typename... _Tail>
			void output_variadic_arguments(
				const std::vector<std::string>& names,
				size_t id,
				const _Head& head,
				const _Tail&... tail
			) {
				debug << names[id] << " == " << boolalpha << head
					<< " (" << typeid(head).name() << ")\n";
				output_variadic_arguments(names, id + 1, tail...);
			}

			template<typename _Head>
			void output_variadic_arguments (
				const std::vector<std::string>& names,
				size_t id,
				const _Head& head
				) {
				debug << names[id] << " == " << boolalpha << head
					<< " (" << typeid(head).name() << ")\n";
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