use crate::token::{Token, TokenContext, TokenKind};

pub struct Lexer {
    source_file: String,
    content: String,
    offset: usize,
    line: usize,
    column: usize,
}

impl Lexer {
    pub fn new(source_file: String, content: String) -> Self {
        Self {
            source_file,
            content,
            offset: 0,
            line: 1,
            column: 0,
        }
    }

    fn current(&self) -> char {
        // Advance will always increase the offset by one,
        // so to get the current char we need to get the
        // `offset - 1` character.
        self.content.chars().nth(self.offset - 1).unwrap()
    }

    fn advance(&mut self) -> Option<char> {
        let c = self.content.chars().nth(self.offset)?;

        if c == '\n' {
            self.line += 1;
            self.column = 0;
        } else {
            self.column += 1;
        }

        self.offset += 1;

        Some(c)
    }

    fn token_context(&self) -> TokenContext {
        TokenContext {
            source_file: self.source_file.clone(),
            line: self.line,
            column: self.column,
        }
    }

    fn tokenize_number(&mut self) -> Token {
        let mut number_str = String::from(self.current());
        let context = self.token_context();

        while let Some(c) = self.advance() {
            if !c.is_digit(10) {
                break;
            }

            number_str.push(c);
        }

        Token {
            kind: TokenKind::Number(number_str),
            context,
        }
    }

    pub fn next(&mut self) -> Option<Token> {
        let mut c: char;
        loop {
            c = self.advance()?;

            if !c.is_whitespace() {
                break;
            }
        }

        if c.is_digit(10) {
            return Some(self.tokenize_number());
        }

        Some(Token {
            kind: TokenKind::Invalid,
            context: TokenContext {
                source_file: self.source_file.clone(),
                line: self.line,
                column: self.column,
            },
        })
    }
}
