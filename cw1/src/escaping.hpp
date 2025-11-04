#pragma once

#include <string>
#include <ranges>
#include <format>

template <class Token, std::ranges::input_range R>
auto escapeString(R &&r) {
	std::string res;
	for (Token c : r) {
		switch (char(c)) {
			case '\n': res += "\\n"; break;
			case '\b': res += "\\b"; break;
			case '\f': res += "\\f"; break;
			case '\t': res += "\\t"; break;
			case '\"': res += "\\\""; break;
			case '\\': res += "\\\\"; break;
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 11:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			case 31: res += std::format("<0x{:02x}>", unsigned(char(c))); break;
			default: res += char(c); break;
		}
	}
	return res;
}

template <std::ranges::input_range Range>
auto unescapeString(Range &&r) {
	std::string res;
	auto		it	= std::ranges::begin(r);
	auto		end = std::ranges::end(r);
	while (it != end) {
		char c = char(*it);
		if (c == '\\' && ++it != end) {
			char next = char(*it);
			switch (next) {
				case 'n': res += '\n'; break;
				case 'b': res += '\b'; break;
				case 'f': res += '\f'; break;
				case 't': res += '\t'; break;
				case 'r': res += '\r'; break;
				case '"': res += '\"'; break;
				case '\\': res += '\\'; break;
				default: res += next; break;
			}
			++it;
		} else {
			res += c;
			++it;
		}
	}
	return res;
}
