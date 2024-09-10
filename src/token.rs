#[derive(Debug)]
pub enum TokenKind {
    Invalid,
    Identifier,

    Number(String),
    String(String),
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
