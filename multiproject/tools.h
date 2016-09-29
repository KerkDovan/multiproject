#pragma once
#ifndef _MULTIPROJECT_TOOLS_H_
#define _MULTIPROJECT_TOOLS_H_

#include <chrono>

namespace tools {

	template<typename... _Types>
	inline size_t count_of_params(const _Types&...) {
		return sizeof...(_Types);
	}

	template<typename _First, typename... _Rest>
	auto select(int selected, const _First& first, const _Rest&... rest) {
		if (!selected)
			return first;
		return select(selected - 1, rest...);
	}

	template<typename _First>
	_First select(int, const _First& first) {
		return first;
	}

	namespace randomness {

		template<typename... _Types>
		inline auto random(const _Types&... list) {
			return select(std::rand() % count_of_params(list...), list...);
		}

		inline bool random_bool() {
			return std::rand() % 2 == 1;
		}

		inline char random_char(char from, char to) {
			return (char)(std::rand() % (to - from) + from);
		}

		inline char random_char() {
			return (char)std::rand();
		}

		inline unsigned char random_uchar(unsigned char from, unsigned char to) {
			return (unsigned char)(std::rand() % (to - from) + from);
		}

		inline unsigned char random_uchar() {
			return (unsigned char)std::rand();
		}

		inline int random_int(int from, int to) {
			return (int)(std::rand() % (to - from) + from);
		}

		inline int random_int() {
			return (int)std::rand();
		}

		inline unsigned int random_uint(unsigned int from, unsigned int to) {
			return (unsigned int)(std::rand() % (to - from) + from);
		}

		inline unsigned int random_uint() {
			return (unsigned int)std::rand();
		}

	}

}

#endif // _MULTIPROJECT_TOOLS_H_