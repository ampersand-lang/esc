#include <tuple>
#include "esc.h"

#define with(__name, __parser, ...) \
    auto __name##maybe = __parser(__VA_ARGS__); \
    if (std::holds_alternative<esc::Error>(__name##maybe)) { \
        return std::get<esc::Error>(__name##maybe); \
    } \
    auto __name = std::move(std::get<0>(__name##maybe));

static std::variant<std::pair<esc::Node<esc::Ast>, esc::AstClass>, esc::Error> parse_stmt(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::Number) {
        return esc::Error();
    }

    with (result, parse, ctx, tokens) {
        if (tokens.size() > 0 && tokens[0].kind == esc::TokenKind::Semicolon) {
            tokens.pop_front();
            return std::make_pair(std::move(result), esc::AstClass::Statement);
        } else {
            return std::make_pair(std::move(result), esc::AstClass::Expression);
        }
    }
}

static std::variant<esc::Node<esc::Word>, esc::Error> parse_array(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    auto span = ctx.location(tok.loc);
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::BracketLeft) {
        return esc::Error();
    }

    std::vector<esc::Node<esc::Word>> array;
    std::optional<std::unique_ptr<esc::Node<esc::Word>>> last;
    while (tokens.size() > 0 && tokens[0].kind != esc::TokenKind::BracketRight) {
        with (stmt, parse_stmt, ctx, tokens) {
            if (stmt.second == esc::AstClass::Statement) {
                array.push_back(stmt.first.map<esc::Word>([](auto ast) { return esc::Word(std::move(ast)); }));
            } else {
                last = std::make_unique<esc::Node<esc::Word>>(stmt.first.map<esc::Word>([](auto ast) { return esc::Word(std::move(ast)); }));
                break;
            }
        }
    }

    tok = tokens[0];
    span += ctx.location(tok.loc);
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::BracketRight) {
        return esc::Error();
    }

    auto loc = ctx.add_location(span);
    return esc::Node(esc::Metadata(loc), esc::Word(esc::Array(std::move(array), std::move(last))));
}

static std::variant<esc::Node<esc::Word>, esc::Error> parse_block(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    auto span = ctx.location(tok.loc);
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::CurlyLeft) {
        return esc::Error();
    }

    std::vector<esc::Node<esc::Word>> block;
    std::optional<std::unique_ptr<esc::Node<esc::Word>>> result;
    while (tokens.size() > 0 && tokens[0].kind != esc::TokenKind::CurlyRight) {
        with (stmt, parse_stmt, ctx, tokens) {
            if (stmt.second == esc::AstClass::Statement) {
                block.push_back(stmt.first.map<esc::Word>([](auto ast) { return esc::Word(std::move(ast)); }));
            } else {
                result = std::make_unique<esc::Node<esc::Word>>(stmt.first.map<esc::Word>([](auto ast) { return esc::Word(std::move(ast)); }));
                break;
            }
        }
    }

    tok = tokens[0];
    span += ctx.location(tok.loc);
    tokens.pop_front();
    if (tokens.size() == 0 && tok.kind != esc::TokenKind::CurlyRight) {
        return esc::Error();
    }

    auto loc = ctx.add_location(span);
    return esc::Node(esc::Metadata(loc), esc::Word(esc::Block(std::move(block), std::move(result))));
}

static std::variant<esc::Node<esc::Word>, esc::Error> parse_number(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::Number) {
        return esc::Error();
    }

    auto loc = ctx.location(tok.loc);
    return esc::Node(esc::Metadata(tok.loc), esc::Word(esc::Number(atol(ctx.src().substr(loc.offset, loc.len).c_str()))));
}

static std::variant<esc::Node<esc::Word>, esc::Error> parse_string(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::String) {
        return esc::Error();
    }

    auto loc = ctx.location(tok.loc);
    return esc::Node(esc::Metadata(tok.loc), esc::Word(esc::String(ctx.src().substr(loc.offset + 1, loc.len - 2))));
}

static std::variant<esc::Node<esc::Word>, esc::Error> parse_ident(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::Ident) {
        return esc::Error();
    }

    auto loc = ctx.location(tok.loc);
    return esc::Node(esc::Metadata(tok.loc), esc::Word(esc::Identifier(ctx.src().substr(loc.offset, loc.len))));
}

static std::variant<esc::Node<esc::Word>, esc::Error> parse_word(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    switch (tokens[0].kind) {
    case esc::TokenKind::ParenLeft: {
        with (word, parse, ctx, tokens) {
            return word.map<esc::Word>([](auto ast) { return esc::Word(std::move(ast)); });
        }
    }
    case esc::TokenKind::BracketLeft: {
        with (word, parse_array, ctx, tokens) {
            return word;
        }
    }
    case esc::TokenKind::CurlyLeft: {
        with (word, parse_block, ctx, tokens) {
            return word;
        }
    }
    case esc::TokenKind::Ident: {
        with (word, parse_ident, ctx, tokens) {
            return word;
        }
    }
    case esc::TokenKind::String: {
        with (word, parse_string, ctx, tokens) {
            return word;
        }
    }
    case esc::TokenKind::Number: {
        with (word, parse_number, ctx, tokens) {
            return word;
        }
    }
    default:
        return esc::Error();
    }
}

esc::ParseResult esc::parse(esc::Context& ctx, std::deque<esc::Token>& tokens) {
    auto tok = tokens[0];
    auto span = ctx.location(tok.loc);
    tokens.pop_front();
    if (tok.kind != esc::TokenKind::ParenLeft) {
        return esc::Error();
    }

    with (function, parse_word, ctx, tokens) {
        std::vector<esc::Node<esc::Word>> arguments;

        while (tokens.size() > 0 && tokens[0].kind != esc::TokenKind::ParenRight) {
            with (word, parse_word, ctx, tokens) {
                arguments.push_back(std::move(word));
            }
        }

        auto tok = tokens[0];
        span += ctx.location(tok.loc);
        tokens.pop_front();
        if (tokens.size() == 0 && tok.kind != esc::TokenKind::ParenRight) {
            return esc::Error();
        }

        auto loc = ctx.add_location(span);
        return esc::Node(esc::Metadata(loc), esc::Ast(std::make_unique<esc::Node<esc::Word>>(std::move(function)), std::move(arguments)));
    }
}

esc::String::String(std::string str): m_str(str) {}
esc::Identifier::Identifier(std::string sym): m_sym(sym) { }
esc::Parenth::Parenth(std::unique_ptr<esc::Node<Ast>> ast): m_ast(std::move(ast)) { }

esc::Array::Array(std::vector<esc::Node<Word>> array, std::optional<std::unique_ptr<esc::Node<Word>>> last):
    m_array(std::move(array)),
    m_last(std::move(last)) { }

esc::Block::Block(std::vector<esc::Node<Word>> block, std::optional<std::unique_ptr<esc::Node<Word>>> result):
    m_block(std::move(block)),
    m_result(std::move(result)) { }

esc::Ast::Ast(std::unique_ptr<esc::Node<Word>> function, std::vector<esc::Node<Word>> arguments):
    m_function(std::move(function)),
    m_arguments(std::move(arguments)) { }

esc::Word::Word(Number word): m_word(std::move(word)) { }
esc::Word::Word(String word): m_word(std::move(word)) { }
esc::Word::Word(Identifier word): m_word(std::move(word)) { }
esc::Word::Word(Array word): m_word(std::move(word)) { }
esc::Word::Word(Block word): m_word(std::move(word)) { }
esc::Word::Word(Ast word): m_word(std::move(word)) { }
