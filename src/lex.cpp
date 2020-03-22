#include "esc.h"

esc::Token::Token(TokenKind kind, LocationId loc):
    kind(kind),
    loc(loc) {}

static bool IDENT_BEGIN[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static bool IDENT_CONT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

bool is_ident_begin(unsigned char c) {
    return IDENT_BEGIN[c];
}

bool is_ident_cont(unsigned char c) {
    return IDENT_CONT[c];
}

esc::LexResult lex(esc::Context& ctx) {
    std::vector<esc::Token> tokens;

    size_t offset = 0;
    auto iter = ctx.src().begin();
    auto end = ctx.src().end();
    for (char c = *iter; iter != end; iter++, offset++) {
        switch (c) {
        case '\n':
            continue;
        case '\t':
            continue;
        case ' ':
            continue;
        case '#':
            while (*iter != '\n' && iter != end) {
                ++offset;
                ++iter;
            }
            continue;
        case ',':
            tokens.push_back(esc::Token(esc::TokenKind::Comma, ctx.add_location(offset, 1)));
            break;
        case ';':
            tokens.push_back(esc::Token(esc::TokenKind::Semicolon, ctx.add_location(offset, 1)));
            break;
        case '(':
            tokens.push_back(esc::Token(esc::TokenKind::ParenLeft, ctx.add_location(offset, 1)));
            break;
        case ')':
            tokens.push_back(esc::Token(esc::TokenKind::ParenRight, ctx.add_location(offset, 1)));
            break;
        case '[':
            tokens.push_back(esc::Token(esc::TokenKind::BracketLeft, ctx.add_location(offset, 1)));
            break;
        case ']':
            tokens.push_back(esc::Token(esc::TokenKind::BracketRight, ctx.add_location(offset, 1)));
            break;
        case '{':
            tokens.push_back(esc::Token(esc::TokenKind::CurlyLeft, ctx.add_location(offset, 1)));
            break;
        case '}':
            tokens.push_back(esc::Token(esc::TokenKind::CurlyRight, ctx.add_location(offset, 1)));
            break;
        case '"': {
            // TODO(walterpi): escape codes
            size_t pos = offset;
            size_t len = 1;
            if (c != '"') {
                do {
                    if (iter == end) {
                        return esc::Error();
                    }
                    ++len;
                    ++offset;
                    c = *++iter;
                } while (c != '"');
            }
            tokens.push_back(esc::Token(esc::TokenKind::String, ctx.add_location(pos, len)));
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            // TODO(walterpi): escape codes
            size_t pos = offset;
            size_t len = 1;
            do {
                if (iter == end) {
                    return esc::Error();
                }
                if (is_ident_begin(c)) {
                    return esc::Error();
                }
                ++len;
                ++offset;
                c = *++iter;
            } while (c >= '0' && c <= '9');
            tokens.push_back(esc::Token(esc::TokenKind::Number, ctx.add_location(pos, len)));
        } break;
        default:
            if (is_ident_begin(*iter)) {
                size_t pos = offset;
                size_t len = 0;
                while (is_ident_cont(*iter)) {
                    ++iter;
                    ++offset;
                    ++len;
                }
                tokens.push_back(esc::Token(esc::TokenKind::Ident, ctx.add_location(pos, len)));
                continue;
            } else {
                return esc::Error();
            }
            break;
        }
    }

    return tokens;
}
