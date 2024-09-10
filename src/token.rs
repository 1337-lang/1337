#[derive(Debug)]
pub enum TokenKind {
    Invalid(String),
    Identifier(String),

    // Values
    Number(String),
    String(String),
    Char(String),

    // Keywords
    Fn,
    Mut,
    If,
    Else,
    Elif,
    For,
    In,
    While,

    // Symbols
    LeftCurly,
    RightCurly,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    Colon,

    // * Boolean Operators
    Not,
    And,
    Or,
    Equals,
    NotEquals,
    LowerThan,
    LowerEquals,
    GreaterThan,
    GreaterEquals,

    // * Bitwise Operators
    BitNot,
    BitAnd,
    BitOr,
    BitXor,

    // * Binary Operators
    Declaratation,
    Assignment,
    Plus,
    Minus,
    Times,
    DividedBy,
    Modulus,
}

#[derive(Debug)]
pub struct TokenContext {
    pub source_file: String,
    pub line: usize,
    pub column: usize,
}

#[derive(Debug)]
pub struct Token {
    pub kind: TokenKind,
    pub context: TokenContext,
}
