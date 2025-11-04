#include <iostream>
#include <string>
#include <ranges>
#include <filesystem>

#define DBG_LOG_LEVEL 2
#include "Regex/OutputFSA.hpp"
#include "lang/lex_traverser.hpp"

#include "../escaping.hpp"
#include "../tokens.hpp"

enum ErrorType {
	_UNUSED,
	UNMATCHED_OPEN_COMMENT,
	UNMATCHED_CLOSE_COMMENT,
	STRING_TOO_LONG,
	STRING_CONTAINS_NULL,
	STRING_CONTAINS_ESCAPED_NULL
};

std::string caseIns(const std::string &s) {
	std::string res;
	for (auto [i, c] : std::views::enumerate(s)) {
		if (std::isalpha(c)) {
			res += std::format("('{}'+'{}')", (char)std::tolower(c), (char)std::toupper(c));
		} else {
			res += std::format("'{}'", c);
		}
		if (i + 1 < (long)s.size()) res += ".";
	}
	return res;
}

auto createLexer() {
	// грозотата на този код е породена от опити за constexpr оптимизации
	constexpr std::size_t NUM_STATES	  = 157;
	constexpr std::size_t NUM_TRANSITIONS = 8582;
	constexpr const char* pack_path	  = ROOT_DIR "/src/lexer_packed.bin";
	if (std::filesystem::exists(pack_path)) {
		SSFT<Token>::PackedSSFTInputOnly<NUM_STATES, NUM_TRANSITIONS> packed;
		{
			std::ifstream in(pack_path, std::ios::binary);
			in.read(reinterpret_cast<char *>(&packed), sizeof(packed));
		}
		return SSFT<Token>::loadPackedInputOnly(packed);
	}

	auto		time_start = std::chrono::high_resolution_clock::now();
	std::string _09		   = "('0'+'1'+'2'+'3'+'4'+'5'+'6'+'7'+'8'+'9')";
	std::string _az =
		"('a'+'b'+'c'+'d'+'e'+'f'+'g'+'h'+'i'+'j'+'k'+'l'+'m'+'n'+'o'+'p'+'q'+'r'+'s'+'t'+'u'+'v'+'w'+'x'+'y'+'z')";
	std::string _AZ =
		"('A'+'B'+'C'+'D'+'E'+'F'+'G'+'H'+'I'+'J'+'K'+'L'+'M'+'N'+'O'+'P'+'Q'+'R'+'S'+'T'+'U'+'V'+'W'+'X'+'Y'+'Z')";
	std::string all_chars =
		"'\r'+'\t'+"
		"' '+'!'+'#'+'$'+'%'+'&'+'\\''+"
		"'('+')'+'*'+'+'+','+'-'+'.'+'/'"
		"+'0'+'1'+'2'+'3'+'4'+'5'+'6'+'7'+'8'+'9'+':'+';'+'<'+'='+'>'+'?'+'@'+'A'+'B'+'C'+'D'+'E'+'F'+'G'+'H'+'I'+'J'+"
		"'K'+'L'+'M'+'N'+'O'+'P'+'Q'+'R'+'S'+'T'+'U'+'V'+'W'+'X'+'Y'+'Z'+'['+']'+'^'+'_'+'`'+'a'+'b'+'c'+'d'"
		"+'e'+'f'+'g'+'h'+'i'+'j'+'k'+'l'+'m'+'n'+'o'+'p'+'q'+'r'+'s'+'t'+'u'+'v'+'w'+'x'+'y'+'z'+'{'+'|'+'}'+'~'+"
		"'\v'+'\a'+'\b'+'\f'+'\x0E'+'\x0F'+'\x10'+'\x11'+'\x12'+'\x13'+'\x14'+'\x15'+'\x16'+'\x17'+'\x18'+'\x19'+'\x1A'"
		"+'\x1B'+'\x1C'+'\x1D'+'\x1E'+'\x1F'";
	std::string escaped_chars		= "'\\\\'+'\"'+'\n'+null";
	std::string escaped_not_newline = "'\\\\'+'\"'+null";

	OutputFSA<Token> assign{"'<-'", ASSIGN};
	OutputFSA<Token> bool_const_true{std::format("'t'.({})", caseIns("rue")), BOOL_CONST_TRUE};
	OutputFSA<Token> bool_const_false{std::format("'f'.({})", caseIns("alse")), BOOL_CONST_FALSE};
	OutputFSA<Token> case_{caseIns("case"), CASE};
	OutputFSA<Token> class_{caseIns("class"), CLASS};
	OutputFSA<Token> darrow{"'=>'", DARROW};
	OutputFSA<Token> else_{caseIns("else"), ELSE};
	OutputFSA<Token> esac{caseIns("esac"), ESAC};
	OutputFSA<Token> fi{caseIns("fi"), FI};
	OutputFSA<Token> if_{caseIns("if"), IF};
	OutputFSA<Token> in{caseIns("in"), IN};
	OutputFSA<Token> inherits{caseIns("inherits"), INHERITS};
	OutputFSA<Token> int_const{_09 + "!", INT_CONST};
	OutputFSA<Token> isvoid{caseIns("isvoid"), ISVOID};
	OutputFSA<Token> le{"'<='", LE};
	OutputFSA<Token> let{caseIns("let"), LET};
	OutputFSA<Token> loop{caseIns("loop"), LOOP};
	OutputFSA<Token> new_{caseIns("new"), NEW};
	OutputFSA<Token> not_{caseIns("not"), NOT};
	OutputFSA<Token> objectid{std::format("{}.({}+{}+{}+'_')*", _az, _az, _AZ, _09), OBJECTID};
	OutputFSA<Token> of{caseIns("of"), OF};
	OutputFSA<Token> pool{caseIns("pool"), POOL};
	OutputFSA<Token> string_const{
		std::format("'\"'.(null+{}+'\\\\'.({} + {}))*.'\"'", all_chars, all_chars, escaped_chars), STR_CONST};

	OutputFSA<Token> then{caseIns("then"), THEN};
	OutputFSA<Token> typeid_{std::format("{}.({}+{}+{}+'_')*", _AZ, _az, _AZ, _09), TYPEID};
	OutputFSA<Token> while_{caseIns("while"), WHILE};
	OutputFSA<Token> whitespace{"(' '+'\n'+'\t'+'\r')!", WS};
	OutputFSA<Token> commentSingle{std::format("'--'.({}+{})*.('\n'+'')", all_chars, escaped_not_newline), WS};
	OutputFSA<Token> commentMulti{"'(*'", COMMENT_MULTI};
	OutputFSA<Token> commentMultiEnd{"'*)'", Token(ERROR).setData(UNMATCHED_CLOSE_COMMENT)};

	OutputFSA<Token> semicolon{"';'", ';'};
	OutputFSA<Token> colon{"':'", ':'};
	OutputFSA<Token> comma{"','", ','};
	OutputFSA<Token> dot{"'.'", '.'};
	OutputFSA<Token> plus{"'+'", '+'};
	OutputFSA<Token> minus{"'-'", '-'};
	OutputFSA<Token> multiply{"'*'", '*'};
	OutputFSA<Token> divide{"'/'", '/'};
	OutputFSA<Token> equal{"'='", '='};
	OutputFSA<Token> lt{"'<'", '<'};
	OutputFSA<Token> at{"'@'", '@'};
	OutputFSA<Token> tilde{"'~'", '~'};
	OutputFSA<Token> braceOpen{"'{'", '{'};
	OutputFSA<Token> braceClose{"'}'", '}'};
	OutputFSA<Token> parenthOpen{"'('", '('};
	OutputFSA<Token> parenthClose{"')'", ')'};
	OutputFSA<Token> eof{realtimeFST(BS_WordFSA<Token>({Token::eof}, {})), WS};

	dbLog(dbg::LOG_INFO, "Compiling individual FSAs...");
	auto tokenizer = UnionOutputFSA<Token>(
		whitespace, commentSingle, commentMulti, commentMultiEnd, assign, bool_const_true, bool_const_false, case_,
		class_, darrow, else_, esac, fi, if_, in, inherits, int_const, isvoid, le, let, loop, new_, not_, of, pool,
		string_const, then, while_, objectid, typeid_, semicolon, colon, comma, dot, plus, minus, multiply, divide,
		equal, lt, at, tilde, braceOpen, braceClose, parenthOpen, parenthClose, eof);
	dbLog(dbg::LOG_INFO, "Determinizing FSA...");
	auto Tokenizer = tokenizer.determinizeToSSFT();

	dbLog(dbg::LOG_INFO, "Done");
	auto time_end = std::chrono::high_resolution_clock::now();
	dbLog(dbg::LOG_DEBUG, "Lexer FSA construction took ",
		  std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count(), "ms");

	auto pack = Tokenizer.packInputOnly<NUM_STATES, NUM_TRANSITIONS>();
	{
		std::ofstream out(pack_path, std::ios::binary);
		out.write((char *)&pack, sizeof(pack));
	}

	return Tokenizer;
}

struct lexComment {
	auto operator()(auto &it) {
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
	auto operator()(auto &stream_it) {
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
	auto	   input = CharInputStream<Token>(std::cin);
	auto	   lexer = createLexer();
	LexerRange lex(input, std::move(lexer), ERROR);
	lex.attachSkipper(COMMENT_MULTI, lexComment{});
	lex.attachSkipper(STR_CONST, lexString{});
	for (auto [token, from, to, line, str] : lex) {
		formatToken(std::cout, token, from, to, line, str);
	}
	return 0;
}
