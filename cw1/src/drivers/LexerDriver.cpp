#include <iostream>

#define DBG_LOG_LEVEL 2
#include "../coolLexer.hpp"

void formatError(std::ostream &out, Token token, std::size_t, std::size_t, std::size_t line,
				 std::span<const Token> str) {
	switch (ErrorType((uint64_t)token.data)) {
		case ErrorType::UNMATCHED_OPEN_COMMENT:
			out << std::format("#{} ERROR: Unmatched (*", line - (str.back() == '\n')) << std::endl;
			break;
		case ErrorType::UNMATCHED_CLOSE_COMMENT:
			out << std::format("#{} ERROR: Unmatched *)", line) << std::endl;
			break;
		case ErrorType::STRING_TOO_LONG:
			out << std::format("#{} ERROR: String constant too long", line) << std::endl;
			break;
		case ErrorType::STRING_CONTAINS_NULL:
			out << std::format("#{} ERROR: String contains null character", line) << std::endl;
			break;
		case ErrorType::STRING_CONTAINS_ESCAPED_NULL:
			out << std::format("#{} ERROR: String contains escaped null character", line) << std::endl;
			break;
		default:
			if (str[0] == Token::eof) out << std::format("#{} ERROR: Unterminated string at EOF", line) << std::endl;
			else if (char(str[0]) == '\n')
				out << std::format("#{} ERROR: String contains unescaped new line", line - 1) << std::endl;
			else out << std::format("#{} ERROR: Invalid symbol \"{}\"", line, escapeString<Token>(str)) << std::endl;
	}
}

void formatToken(std::ostream &out, Token token, std::size_t from, std::size_t to, std::size_t line,
				 std::span<const Token> str) {
	switch (uint64_t(token)) {
		case uint64_t(WS):
		case uint64_t(COMMENT_MULTI): break;
		case uint64_t(BOOL_CONST_TRUE): out << std::format("#{} {} {}", line, token, "true") << std::endl; break;
		case uint64_t(BOOL_CONST_FALSE): out << std::format("#{} {} {}", line, token, "false") << std::endl; break;
		case uint64_t(INT_CONST):
		case uint64_t(OBJECTID):
		case uint64_t(TYPEID):
			out << std::format("#{} {} {}", line, token, escapeString<Token>(str)) << std::endl;
			break;
		case uint64_t(ASSIGN):
		case uint64_t(CASE):
		case uint64_t(CLASS):
		case uint64_t(DARROW):
		case uint64_t(ELSE):
		case uint64_t(ESAC):
		case uint64_t(FI):
		case uint64_t(IF):
		case uint64_t(IN):
		case uint64_t(INHERITS):
		case uint64_t(ISVOID):
		case uint64_t(LE):
		case uint64_t(LET):
		case uint64_t(LOOP):
		case uint64_t(NEW):
		case uint64_t(NOT):
		case uint64_t(OF):
		case uint64_t(POOL):
		case uint64_t(THEN):
		case uint64_t(WHILE): out << std::format("#{} {}", line, token) << std::endl; break;
		case uint64_t(STR_CONST):
			out << std::format("#{} {} \"{}\"", line, token,
							   escapeString<Token>(unescapeString(str.subspan(1, str.size() - 2))))
				<< std::endl;
			break;
		case uint64_t(ERROR): formatError(out, token, from, to, line, str); break;
		default:
			assert(int(token) < 256);
			out << std::format("#{} '{}'", line, (char)token) << std::endl;
			break;
	};
}

int main() {
	auto lex = createCoolLexer(std::cin);
	for (auto [token, from, to, line, str] : lex) {
		formatToken(std::cout, token, from, to, line, str);
	}
	return 0;
}
