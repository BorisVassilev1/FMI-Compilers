#include <exception>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include "DPDA/cfg.h"
#include "DPDA/parser.h"
#include "DPDA/token.h"

#define DBG_LOG_LEVEL 2
#include "../../../cw1/src/coolLexer.hpp"

#include <lang/grammar_factory.hpp>

CREATE_TOKEN_CONSTEXPR(program, "_program")
CREATE_TOKEN_CONSTEXPR(class_, "_class")
CREATE_TOKEN_CONSTEXPR(feature, "_feature")
CREATE_TOKEN_CONSTEXPR(formal, "_formal")
CREATE_TOKEN_CONSTEXPR(expr, "_expr")

CREATE_TOKEN_CONSTEXPR(literal, "_literal")
CREATE_TOKEN_CONSTEXPR(memberexpr, "_memberexpr")
CREATE_TOKEN_CONSTEXPR(neighexpr, "_neighexpr")
CREATE_TOKEN_CONSTEXPR(isvoidexpr, "_isvoidexpr")
CREATE_TOKEN_CONSTEXPR(mulexpr, "_mulexpr")
CREATE_TOKEN_CONSTEXPR(arithexpr, "_arithexpr")
CREATE_TOKEN_CONSTEXPR(compexpr, "_compexpr")
CREATE_TOKEN_CONSTEXPR(notexpr, "_notexpr")
CREATE_TOKEN_CONSTEXPR(assignexpr, "_assignexpr")
CREATE_TOKEN_CONSTEXPR(condexpr, "_condexpr")
CREATE_TOKEN_CONSTEXPR(ID_expr, "ID_expr")
CREATE_TOKEN_CONSTEXPR(NONID_expr, "nonID_expr")
CREATE_TOKEN_CONSTEXPR(init, "init")
CREATE_TOKEN_CONSTEXPR(caseLine, "caseLine")
CREATE_TOKEN_CONSTEXPR(finish_expr, "finish_expr")
CREATE_TOKEN_CONSTEXPR(finish_aexpr, "finish_aexpr")
CREATE_TOKEN_CONSTEXPR(term, "term")
CREATE_TOKEN_CONSTEXPR(finish_term, "finish_term")

auto createParser() {
	using namespace ll1g;
	Token current = Token::eps;
	auto  NT	  = [&]() {
		  assert(current != Token::eps && "Cannot create dependent token from eps");
		  return current = Token::createDependentToken(current);
	};

	// clang-format off
	current		 = program;
	auto Program = Repeat<Token>(program, Production<Token>({class_}, {false}), -1);

	current = class_;
	auto Class = Seq<Token>(class_,
		CLASS,
		TYPEID,
		Optional<Token>(NT(), Production<Token>({INHERITS, TYPEID})),
		Token('{'),
		Repeat<Token>(NT(), Production<Token>({feature, Token(';')}), -1),
		Token('}'),
		Token(';')
	);

	current = init;
	auto Init = Seq<Token>(NT(),
		OBJECTID, Token(':'), TYPEID, Optional<Token>(NT(), Production<Token>({ASSIGN, expr})));

	current = feature;
	auto Feature = Seq<Token>(feature,
		OBJECTID,
		Choice<Token>(NT(),
			Seq<Token>(NT(),
				Token('('), Optional<Token>(NT(), Seq<Token>(NT(),
					formal,
					Repeat<Token>(NT(), Seq<Token>(NT(), Token(','), formal), -1))
				),
				Token(')'), Token(':'), TYPEID, Token('{'), expr, Token('}')
			),
			Seq<Token>(NT(), Token(':'), TYPEID, Optional<Token>(NT(), Production<Token>({ASSIGN, expr})))
		)
	);

	current = formal;
	auto Formal = Seq<Token>(formal,
		OBJECTID, Token(':'), TYPEID
	);

	current = caseLine;
	auto CaseLine = Seq<Token>(caseLine,
		OBJECTID, Token(':'), TYPEID, DARROW, expr, Token(';')
	);

	current = NONID_expr;
	auto NONID_Expr = Choice<Token>(NONID_expr,
		Production<Token>({IF, expr, THEN, expr, ELSE, expr, FI}),
		Production<Token>({WHILE, expr, LOOP, expr, POOL}),
		Seq<Token>(NT(),
			Token('{'),
			expr,
			Token(';'),
			Repeat<Token>(NT(), Production<Token>({expr, Token(';')})),
			Token('}')
		),
		Seq<Token>(NT(),
			LET, init, Repeat<Token>(NT(), Production<Token>({Token(','), init})), IN, expr, Token(';') // this ; is important?
		),
		Seq<Token>(NT(),
			CASE, expr, OF, caseLine, Repeat<Token>(NT(), caseLine), ESAC
		),
		Production<Token>({NEW, TYPEID}),
		Production<Token>({Token('('), expr, Token(')')}),
		INT_CONST,
		STR_CONST,
		BOOL_CONST_FALSE,
		BOOL_CONST_TRUE
	);

	current = assignexpr;
	auto AssignExpr = Seq<Token>(assignexpr,
			OBJECTID,
			Choice<Token>(NT(),
				Production<Token>({ASSIGN, assignexpr}),
				finish_expr
			)
		);

	current = finish_expr;
	auto FinishExpr = Choice<Token>(finish_expr,
		Production<Token>({LE, arithexpr}),
		Production<Token>({Token('<'), arithexpr}),
		Production<Token>({Token('='), arithexpr}),
		finish_aexpr
	);

	current = expr;
	auto Expr = Choice<Token>(expr,
		assignexpr,
		NONID_expr
	);

	current = finish_aexpr;
	auto FinishAExpr = Choice<Token>(finish_aexpr,
		Seq<Token>(NT(),
			Choice<Token>(NT(),
				Production<Token>({Token('+'), term}),
				Production<Token>({Token('-'), term})
			),
			RepeatChoice<Token>(NT(),
				Production<Token>({Token('+'), term}),
				Production<Token>({Token('-'), term})
			)
		),
		finish_term
	);

	current = arithexpr;
	auto ArithExpr = Seq<Token>(arithexpr, term, finish_aexpr);

	current = finish_term;
	auto FinishTerm = Choice(finish_term,
		Production<Token>({Token('*'), isvoidexpr}),
		Production<Token>({Token('/'), isvoidexpr})
	);

	current = term;
	auto Term = Seq<Token>(term, isvoidexpr, finish_term);

	current = isvoidexpr;
	auto IsVoidExpr = Choice<Token>(isvoidexpr,
		Production<Token>({ISVOID, neighexpr}),
		neighexpr
	);

	current = neighexpr;
	auto NeighExpr = Choice<Token>(neighexpr,
		Production<Token>({Token('~'), NONID_expr}),
		NONID_expr,
		OBJECTID
	);

	// clang-format on

	auto g = Combine<Token>(Program, Class, Feature, Formal, FinishExpr, Expr, AssignExpr, ArithExpr, Term, IsVoidExpr,
							NeighExpr, FinishTerm, FinishAExpr, CaseLine, NONID_Expr, Init);
	// g.printRules();
	// g.printParseTable();
	auto p = Parser<Token>(g);
	//std::cout << p.getFirst() << std::endl;
	std::cout << p.getFollow() << std::endl;
	//std::cout << p.getNullable() << std::endl;

	return p;
}

namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
	std::unique_ptr<Parser<Token>> p;
	try {
		p = std::make_unique<Parser<Token>>(createParser());
	} catch (std::exception &e) {
		std::cerr << "Error creating parser: " << e << std::endl;
		return 1;
	}

	if (argc != 2) {
		std::cerr << "Expecting exactly one argument: name of input file" << std::endl;
		return 1;
	}
	auto		  file_path = argv[1];
	std::ifstream fin(file_path);

	auto file_name = fs::path(file_path).filename().string();

	auto			   lex = createCoolLexer(fin);
	std::vector<Token> tokens;
	for (auto [token, from, to, line, str] : lex) {
		std::cout << token << " " << from << " " << to << " " << line << " " << std::endl;
		if (token != WS && token != COMMENT_MULTI) tokens.push_back(token);
	}
	tokens.push_back(Token::eof);

	auto t = p->parse(tokens);
	std::cout << t << std::endl;

	return 0;
}
