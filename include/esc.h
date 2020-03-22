#ifndef ESC_H
#define ESC_H 1

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <variant>
#include <optional>

namespace esc {
    class Ast;
    class Word;

    struct Location {
        size_t offset;
        size_t len;

        constexpr Location operator +(const Location& other) const {
            return Location { offset, other.offset - offset + other.len };
        }

        constexpr Location& operator +=(const Location& other) {
            *this = Location { offset, other.offset - offset + other.len };
            return *this;
        }
    };

    typedef size_t LocationId;

    class Context {
    private:
        const std::string m_src;
        std::vector<Location> m_locs;

    public:
        Context(std::string src);
        Context(const Context &) = delete;
        Context(Context &&) = default;
        ~Context() = default;

        const std::string& src() const;
        LocationId add_location(size_t offset, size_t len);
        LocationId add_location(Location loc);
        const Location& location(LocationId id);
    };

    enum class TokenKind {
        ParenLeft,
        ParenRight,
        BracketLeft,
        BracketRight,
        CurlyLeft,
        CurlyRight,
        Comma,
        Semicolon,
        Ident,
        String,
        Number,
    };

    struct Token {
        TokenKind kind;
        LocationId loc;

        Token(TokenKind kind, LocationId loc);
        Token(const Token &) = default;
        ~Token() = default;
    };

    class Metadata {
    private:
        LocationId m_loc;

    public:
        Metadata() = default;
        constexpr explicit Metadata(LocationId loc): m_loc(loc) { }

        constexpr LocationId loc() const {
            return m_loc;
        }
    };

    template<typename T>
    class Node {
    private:
        Metadata m_meta;
        T m_t;

    public:
        Node(Metadata meta, T t):
            m_meta(meta),
            m_t(std::move(t)) { }

        Node(T t):
            m_t(std::move(t)) { }

        T& get() {
            return m_t;
        }

        const T& get() const {
            return m_t;
        }

        Metadata& meta() {
            return m_meta;
        }

        const Metadata& meta() const {
            return m_meta;
        }

        template<typename U, typename F>
        Node<U> map(F f) {
            return Node<U>(m_meta, f(std::move(m_t)));
        }
    };

    typedef int64_t Number;

    class String {
    private:
        std::string m_str;

    public:
        explicit String(std::string str);
        String(const String &) = delete;
        String(String &&) = default;
        ~String() = default;
    };

    class Identifier {
    private:
        std::string m_sym;

    public:
        explicit Identifier(std::string sym);
        Identifier(const Identifier &) = delete;
        Identifier(Identifier &&) = default;
        ~Identifier() = default;
    };

    class Parenth {
    private:
        std::unique_ptr<Node<Ast>> m_ast;

    public:
        explicit Parenth(std::unique_ptr<Node<Ast>> ast);
        Parenth(const Parenth &) = delete;
        Parenth(Parenth &&) = default;
        ~Parenth() = default;
    };

    class Array {
    private:
        std::vector<Node<Word>> m_array;
        std::optional<std::unique_ptr<Node<Word>>> m_last;

    public:
        explicit Array(std::vector<Node<Word>> array, std::optional<std::unique_ptr<Node<Word>>> last = std::nullopt);
        Array(const Array &) = delete;
        Array(Array &&) = default;
        ~Array() = default;
    };

    class Block {
    private:
        std::vector<Node<Word>> m_block;
        std::optional<std::unique_ptr<Node<Word>>> m_result;

    public:
        explicit Block(std::vector<Node<Word>> block, std::optional<std::unique_ptr<Node<Word>>> result = std::nullopt);
        Block(const Block &) = delete;
        Block(Block &&) = default;
        ~Block() = default;
    };

    enum class AstClass {
        Statement,
        Expression,
    };

    class Ast {
    private:
        std::unique_ptr<Node<Word>> m_function;
        std::vector<Node<Word>> m_arguments;

    public:
        Ast(std::unique_ptr<Node<Word>> function, std::vector<Node<Word>> arguments);
        Ast(const Ast &) = delete;
        Ast(Ast &&) = default;
        ~Ast() = default;
    };

    class Word {
    private:
        using Variant = std::variant<Number, String, Identifier, Array, Block, Ast>;
        Variant m_word;

    public:
        Word(Number word);
        Word(String word);
        Word(Identifier word);
        Word(Array word);
        Word(Block word);
        Word(Ast word);
        Word(const Word &) = delete;
        Word(Word &&) = default;
        ~Word() = default;
    };

    class Error { };

    using LexResult = std::variant<std::vector<Token>, Error>;
    using ParseResult = std::variant<Node<Ast>, Error>;

    LexResult lex(Context& ctx);
    ParseResult parse(Context& ctx, std::deque<Token>& tokens);
}

#endif /* ifndef ESC_H */
