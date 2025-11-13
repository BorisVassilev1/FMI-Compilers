#include <exception>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "DPDA/token.h"

#define DBG_LOG_LEVEL 2
#include "../../../cw1/src/coolLexer.hpp"

#include <lang/grammar_factory.hpp>

CREATE_TOKEN_CONSTEXPR(program, "_program");
CREATE_TOKEN_CONSTEXPR(class_, "_class");
CREATE_TOKEN_CONSTEXPR(feature, "_feature");
CREATE_TOKEN_CONSTEXPR(formal, "_formal");
CREATE_TOKEN_CONSTEXPR(expr, "_expr");

CREATE_TOKEN_CONSTEXPR(literal, "_literal");
CREATE_TOKEN_CONSTEXPR(memberexpr, "_memberexpr");
CREATE_TOKEN_CONSTEXPR(neighexpr, "_neighexpr");
CREATE_TOKEN_CONSTEXPR(isvoidexpr, "_isvoidexpr");
CREATE_TOKEN_CONSTEXPR(mulexpr, "_mulexpr");
CREATE_TOKEN_CONSTEXPR(arithexpr, "_arithexpr");
CREATE_TOKEN_CONSTEXPR(compexpr, "_compexpr");
CREATE_TOKEN_CONSTEXPR(notexpr, "_notexpr");
CREATE_TOKEN_CONSTEXPR(assignexpr, "_assignexpr");
CREATE_TOKEN_CONSTEXPR(condexpr, "_condexpr");

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
		Optional<Token>(NT(), Word<Token>(NT(), {INHERITS, TYPEID})),
		Token('{'), 
		Repeat<Token>(NT(), feature, -1), 
		Token('}'), 
		Token(';')
	);

	current = feature;
	auto Feature = Seq<Token>(feature, 
		OBJECTID,
		Choice<Token>(NT(),
			Seq<Token>(NT(), 
				Token('('), formal, 
					Repeat<Token>(NT(), Seq<Token>(NT(), Token(','), formal), -1), 
				Token(')'), Token(':'), TYPEID, Token('{'), expr, Token('}')
			),
			Seq<Token>(NT(), Token(':'), TYPEID, Optional<Token>(NT(), Seq<Token>(NT(), Token('<'), Token('-'), expr)))
		)
	);

	current = formal;
	auto Formal = Seq<Token>(formal, 
		OBJECTID, Token(':'), TYPEID
	);

	current = literal;
	auto Literal = Choice<Token>(literal,
		INT_CONST,
		STR_CONST,
		BOOL_CONST_TRUE,
		BOOL_CONST_FALSE
	);

	current = expr;
	auto Expr = Choice<Token>(expr,
		Seq<Token>(NT(), IF, expr, THEN, expr, ELSE, expr, FI),
		Seq<Token>(NT(), WHILE, expr, LOOP, expr, POOL),
		Seq<Token>(NT(), LET, formal, Repeat<Token>(NT(), Production<Token>({Token(','), formal})), IN, expr),
		Seq<Token>(NT(), Token('{'), expr, Repeat<Token>(NT(), Production<Token>({Token(';'), expr})), Token('}')),
		Seq<Token>(NT(),
			CASE, expr, Token(':'), OF, OBJECTID, Token(':'), TYPEID, DARROW, expr,
			Repeat<Token>(NT(), Production<Token>({Token(';'), OBJECTID, Token(':'), TYPEID, DARROW, expr})),
			ESAC
		),
		Production<Token>({NEW, TYPEID}),
		assignexpr,
		literal
	);

	current = assignexpr;
	auto AssignExpr = Seq<Token>(assignexpr, 
		OBJECTID, 
		ASSIGN,
		Choice<Token>(NT(),
			assignexpr,
			notexpr
		)
	);

	current = notexpr;
	auto NotExpr = Choice<Token>(notexpr,
		Seq<Token>(NT(), NOT, notexpr),
		compexpr
	);

	current = compexpr;
	auto CompExpr = Choice<Token>(compexpr,
		Production<Token>({arithexpr, LE, arithexpr}),
		Seq<Token>(NT(), arithexpr, Token('='), arithexpr),
		Seq<Token>(NT(), arithexpr, Token('<'), arithexpr),
		arithexpr
	);

	current = arithexpr;
	auto ArithExpr = Seq<Token>(arithexpr,
		mulexpr,
		RepeatChoice<Token>(NT(),
				Seq<Token>(NT(), Token('+'), mulexpr),
				Seq<Token>(NT(), Token('-'), mulexpr)
			)
		);

	current = mulexpr;
	auto MulExpr = Seq<Token>(mulexpr,
		isvoidexpr,
		RepeatChoice<Token>(NT(),
				Seq<Token>(NT(), Token('*'), isvoidexpr),
				Seq<Token>(NT(), Token('/'), isvoidexpr)
			)
		);

	current = isvoidexpr;
	auto IsVoidExpr = Choice<Token>(isvoidexpr,
		Seq<Token>(NT(), ISVOID, isvoidexpr),
		neighexpr
	);

	current = neighexpr;
	auto NeighExpr = Choice<Token>(neighexpr,
		Seq<Token>(NT(), Token('~'), neighexpr),
		memberexpr
	);

	current = memberexpr;
	auto MemberExpr = Seq<Token>(memberexpr,
		expr, 
		Optional<Token>(NT(),
			Production<Token>({Token('@'), TYPEID})
		),
		Token('.'),
		OBJECTID,
		Token('('),
		Seq<Token>(NT(),
			expr,
			Repeat<Token>(NT(), Production<Token>({Token(','), expr}))
		),
		Token(')')
	);

	// clang-format on

	auto g = Combine<Token>(Program, Class, Feature, Formal, Expr, AssignExpr, NotExpr, CompExpr, ArithExpr, MulExpr,
							IsVoidExpr, NeighExpr, MemberExpr, Literal);

	return Parser<Token>(g);
}

namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		std::cerr << "Expecting exactly one argument: name of input file" << std::endl;
		return 1;
	}

	auto		  file_path = argv[1];
	std::ifstream fin(file_path);

	auto file_name = fs::path(file_path).filename().string();

	try {
		auto g = createParser();
	} catch (std::exception &e) {
		std::cerr << "Error creating parser: " << e << std::endl;
		return 1;
	}

	auto lex = createCoolLexer(fin);
	for (auto [token, from, to, line, str] : lex) {
		std::cout << token << " " << from << " " << to << " " << line << " " << std::endl;
	}

	return 0;
}
