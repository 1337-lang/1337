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
