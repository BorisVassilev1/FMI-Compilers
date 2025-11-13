#include "coolLexer.hpp"

#include <filesystem>
#include <Regex/OutputFSA.hpp>

static std::string caseIns(const std::string &s) {
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

SSFT<Token> createLexerAut() {
	// грозотата на този код е породена от опити за constexpr оптимизации
	//constexpr std::size_t NUM_STATES	  = 157;
	//constexpr std::size_t NUM_TRANSITIONS = 8582;
	//constexpr const char *pack_path		  = ROOT_DIR "/src/lexer_packed.bin";
	//if (std::filesystem::exists(pack_path)) {
	//	SSFT<Token>::PackedSSFTInputOnly<NUM_STATES, NUM_TRANSITIONS> packed;
	//	{
	//		std::ifstream in(pack_path, std::ios::binary);
	//		in.read(reinterpret_cast<char *>(&packed), sizeof(packed));
	//	}
	//	return SSFT<Token>::loadPackedInputOnly(packed);
	//}

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

	//auto pack = Tokenizer.packInputOnly<NUM_STATES, NUM_TRANSITIONS>();
	//{
	//	std::ofstream out(pack_path, std::ios::binary);
	//	out.write((char *)&pack, sizeof(pack));
	//}

	return Tokenizer;
}

