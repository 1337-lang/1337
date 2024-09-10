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

    fn tokenize_number(&mut self) -> TokenKind {
        let mut number_str = String::from(self.current());
        let mut dot_count = 0;

        while let Some(c) = self.advance() {
            if !c.is_ascii_digit() && c != '_' && c != '.' {
                break;
            }

            if c == '.' {
                dot_count += 1;
            }

            number_str.push(c);
        }

        if dot_count > 1 {
            TokenKind::Invalid(number_str)
        } else {
            TokenKind::Number(number_str)
        }
    }

    fn tokenize_identifier(&mut self) -> TokenKind {
        let mut ident_str = String::from(self.current());

        while let Some(c) = self.advance() {
            if !c.is_ascii_alphanumeric() && c != '_' {
                break;
            }

            ident_str.push(c);
        }

        match ident_str.as_str() {
            "fn" => TokenKind::Fn,
            "mut" => TokenKind::Mut,
            _ => TokenKind::Identifier(ident_str),
        }
    }

    fn tokenize_string(&mut self) -> TokenKind {
        let mut string = String::new();
        while let Some(c) = self.advance() {
            if c == '"' {
                self.advance();
                return TokenKind::String(string);
            }

            string.push(c);
        }

        TokenKind::Invalid(string)
    }

    fn tokenize_character(&mut self) -> TokenKind {
        let mut character = String::new();
        while let Some(c) = self.advance() {
            if c == '\'' {
                self.advance();

                // TODO: Support special characters by resolving
                //       the character string in a separate function
                //       which will also be reused by `tokenize_string`
                if character.len() != 1 {
                    break;
                }

                return TokenKind::Char(character);
            }

            character.push(c);
        }

        TokenKind::Invalid(character)
    }

    pub fn next(&mut self) -> Option<Token> {
        let mut c: char;
        loop {
            c = self.advance()?;

            if !c.is_whitespace() {
                break;
            }
        }

        let context = self.token_context();

        if c.is_ascii_digit() {
            return Some(Token {
                kind: self.tokenize_number(),
                context,
            });
        }

        if c.is_ascii_alphabetic() || c == '_' {
            return Some(Token {
                kind: self.tokenize_identifier(),
                context,
            });
        }

        if c == '"' {
            return Some(Token {
                kind: self.tokenize_string(),
                context,
            });
        }

        if c == '\'' {
            return Some(Token {
                kind: self.tokenize_character(),
                context,
            });
        }

        Some(Token {
            kind: TokenKind::Invalid(c.into()),
            context: TokenContext {
                source_file: self.source_file.clone(),
                line: self.line,
                column: self.column,
            },
        })
    }
}
