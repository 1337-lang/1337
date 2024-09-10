use lexer::Lexer;

mod lexer;
mod token;

fn main() {
    let path = "examples/first.1337";
    let content = std::fs::read_to_string(path).expect("Failed to read source file");
    let mut lexer = Lexer::new(path.to_string(), content);

    while let Some(token) = lexer.next() {
        println!("TOKEN: {:?}", token);
    }
}
