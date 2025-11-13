#pragma once

#include <tuple>
#include <lang/lex_traverser.hpp>

#include "escaping.hpp"
#include "tokens.hpp"

enum ErrorType {
	_UNUSED,
	UNMATCHED_OPEN_COMMENT,
	UNMATCHED_CLOSE_COMMENT,
	STRING_TOO_LONG,
	STRING_CONTAINS_NULL,
	STRING_CONTAINS_ESCAPED_NULL
};

struct lexComment {
	std::tuple<std::size_t, Token> operator()(auto &it) {
		// comments will be discarded, so we don't need to store them in the buffer
		int			counter = 1;
		std::size_t len		= 0;
		Token		prev	= Token::eps;
		Token		current = Token::eps;
		while (counter > 0 && it.current != it.end) {
			prev	= current;
			current = *it.current;
			if (prev == '(' && current == '*') {
				++counter;
			} else if (prev == '*' && current == ')') {
				--counter;
			}
			if (current == '\n') ++it.line_number;
			++it.current;
			++len;
		}
		if (counter > 0) {
			it.buffer.push_back(prev);
			return std::tuple{len, Token(ERROR).setData(ErrorType::UNMATCHED_OPEN_COMMENT)};
		}
		return std::tuple{len, COMMENT_MULTI};
	}
};

struct lexString {
	std::tuple<std::size_t, Token> operator()(auto &stream_it) {
		auto str	   = std::span<const Token>(stream_it.buffer.begin(), stream_it.buffer.end());
		auto unescaped = unescapeString(str.subspan(1, str.size() - 2));
		auto it		   = std::find(str.begin(), str.end(), '\0');
		if (unescaped.size() > 1024) return std::tuple{0ull, Token(ERROR).setData(STRING_TOO_LONG)};
		if (it != str.end()) {
			if (it != str.begin() && *(it - 1) == '\\')
				return std::tuple{0ull, Token(ERROR).setData(STRING_CONTAINS_ESCAPED_NULL)};
			return std::tuple{0ull, Token(ERROR).setData(STRING_CONTAINS_NULL)};
		}
		return std::tuple{0ull, STR_CONST};
	}
};

SSFT<Token> createLexerAut();

inline auto createCoolLexer(std::istream &in) {
	auto	   input = CharInputStream<Token>(in);
	auto	   lexer = createLexerAut();
	LexerRange lex(input, std::move(lexer), ERROR);
	lex.attachSkipper(COMMENT_MULTI, lexComment{});
	lex.attachSkipper(STR_CONST, lexString{});
	return lex;
}
