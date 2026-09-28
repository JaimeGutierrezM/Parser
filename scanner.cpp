#include "scanner.hpp"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <cctype>

static const std::unordered_map<std::string, std::string> keywords = {
    {"False", "FALSE"},       {"None", "NONE"},         {"True", "TRUE"},
    {"and", "AND"},           {"as", "AS"},             {"assert", "ASSERT"},
    {"async", "ASYNC"},       {"await", "AWAIT"},       {"break", "BREAK"},
    {"class", "CLASS"},       {"continue", "CONTINUE"}, {"def", "DEF"},
    {"del", "DEL"},           {"elif", "ELIF"},         {"else", "ELSE"},
    {"except", "EXCEPT"},     {"finally", "FINALLY"},   {"for", "FOR"},
    {"from", "FROM"},         {"global", "GLOBAL"},     {"if", "IF"},
    {"import", "IMPORT"},     {"in", "IN"},             {"is", "IS"},
    {"lambda", "LAMBDA"},     {"nonlocal", "NONLOCAL"}, {"not", "NOT"},
    {"or", "OR"},             {"pass", "PASS"},         {"raise", "RAISE"},
    {"return", "RETURN"},     {"try", "TRY"},           {"while", "WHILE"},
    {"with", "WITH"},         {"yield", "YIELD"},       {"print", "PRINT"},
    {"int", "INT_TYPE"},      {"bool", "BOOL_TYPE"},    {"string", "STRING_TYPE"}
};

Scanner::Scanner(const std::string& filename)
    : filename(filename), text_Arr(nullptr), index_pos(0), sizee(0),
      line_global(1), col_global(1), error_count(0), indent_stack({0}),
      bracket_depth(0), at_line_start(true) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        sizee = file.tellg();
        file.seekg(0, std::ios::beg);
        text_Arr = new char[sizee + 1];
        file.read(text_Arr, sizee);
        text_Arr[sizee] = '\0';
        file.close();
    } else {
        std::cerr << "Error: No se pudo abrir el archivo '" << filename << "'" << std::endl;
        error_count++;
    }
}

Scanner::~Scanner() {
    if (text_Arr) {
        delete[] text_Arr;
    }
}

bool Scanner::hasError() const {
    return error_count > 0;
}

char Scanner::get_char() {
    if (index_pos < sizee) {
        char ch = text_Arr[index_pos++];
        col_global++;
        return ch;
    }
    return '\0';
}

char Scanner::peek_actual_char() {
    if (index_pos < sizee) {
        return text_Arr[index_pos];
    }
    return '\0';
}

void Scanner::report_error(const std::string& message, int line, int col) {
    std::cerr << "ERROR SCANNER - " << message << " (" << line << ":" << col << ")" << std::endl;
    error_count++;
}

void Scanner::process_indentation(std::vector<Token>& tokens) {
    while (index_pos < sizee && at_line_start) {
        int temp_index = index_pos;
        while (temp_index < sizee && (text_Arr[temp_index] == ' ' || text_Arr[temp_index] == '\t')) {
            temp_index++;
        }

        char next_ch = (temp_index < sizee) ? text_Arr[temp_index] : '\0';

        if (next_ch == '\n' || next_ch == '\r' || next_ch == '\0') {
            while (index_pos < temp_index) get_char();
            if (next_ch == '\r') get_char();
            if (peek_actual_char() == '\n') get_char();
            line_global++;
            col_global = 1;
            continue;
        }

        if (next_ch == '#') {
            while (index_pos < sizee && peek_actual_char() != '\n' && peek_actual_char() != '\r') {
                get_char();
            }
            if (peek_actual_char() == '\r') get_char();
            if (peek_actual_char() == '\n') get_char();
            line_global++;
            col_global = 1;
            continue;
        }

        while (index_pos < temp_index) get_char();
        at_line_start = false;
    }
}

void Scanner::scan_next_tokens(std::vector<Token>& tokens) {
    if (at_line_start) {
        process_indentation(tokens);
    }

    if (index_pos >= sizee) {
        tokens.push_back(Token("EOF", "", line_global, col_global));
        return;
    }

    char ch = peek_actual_char();

    if (ch == ' ' || ch == '\t') {
        get_char();
        return;
    }

    if (ch == '#') {
        while (index_pos < sizee && peek_actual_char() != '\n' && peek_actual_char() != '\r') {
            get_char();
        }
        return;
    }

    if (ch == '\n' || ch == '\r') {
        if (ch == '\r') get_char();
        if (peek_actual_char() == '\n') get_char();

        line_global++;
        col_global = 1;
        at_line_start = true;
        return;
    }

    int start_line = line_global;
    int start_col = col_global;

    if (isalpha(ch) || ch == '_') {
        std::string word = "";
        while (index_pos < sizee && (isalnum(peek_actual_char()) || peek_actual_char() == '_')) {
            word += get_char();
        }
        if (keywords.find(word) != keywords.end()) {
            tokens.push_back(Token(keywords.at(word), word, start_line, start_col));
        } else {
            tokens.push_back(Token("ID", word, start_line, start_col));
        }
        return;
    }

    if (isdigit(ch)) {
        std::string word = "";
        while (index_pos < sizee && isdigit(peek_actual_char())) {
            word += get_char();
        }
        tokens.push_back(Token("INT", word, start_line, start_col));
        return;
    }

    if (ch == '"') {
        get_char();
        std::string str_val = "";
        bool closed = false;

        while (index_pos < sizee) {
            char curr = get_char();
            if (curr == '"') {
                closed = true;
                break;
            }
            if (curr == '\n' || curr == '\r') break;
            if (curr == '\\') {
                if (index_pos >= sizee) break;
                char esc = get_char();
                if (esc == 'n') str_val += '\n';
                else if (esc == 't') str_val += '\t';
                else if (esc == '"') str_val += '"';
                else if (esc == '\\') str_val += '\\';
                else report_error(std::string("Escape invalido '\\") + esc + "'", start_line, start_col);
            } else {
                str_val += curr;
            }
        }

        if (!closed) {
            report_error("Cadena sin cerrar", start_line, start_col);
            return;
        }

        tokens.push_back(Token("STRING", str_val, start_line, start_col));
        return;
    }

    get_char();

    if (ch == '+') tokens.push_back(Token("PLUS", "+", start_line, start_col));
    else if (ch == '-') tokens.push_back(Token("MINUS", "-", start_line, start_col));
    else if (ch == '*') tokens.push_back(Token("STAR", "*", start_line, start_col));
    else if (ch == '/') tokens.push_back(Token("SLASH", "/", start_line, start_col));
    else if (ch == '%') tokens.push_back(Token("PERCENT", "%", start_line, start_col));
    else if (ch == '<') {
        if (peek_actual_char() == '=') {
            get_char();
            tokens.push_back(Token("LESSEQ", "<=", start_line, start_col));
        } else {
            tokens.push_back(Token("LESS", "<", start_line, start_col));
        }
    } else if (ch == '>') {
        if (peek_actual_char() == '=') {
            get_char();
            tokens.push_back(Token("GREATEREQ", ">=", start_line, start_col));
        } else {
            tokens.push_back(Token("GREATER", ">", start_line, start_col));
        }
    } else if (ch == '=') {
        if (peek_actual_char() == '=') {
            get_char();
            tokens.push_back(Token("EQEQ", "==", start_line, start_col));
        } else {
            tokens.push_back(Token("ASSIGN", "=", start_line, start_col));
        }
    } else if (ch == '!') {
        if (peek_actual_char() == '=') {
            get_char();
            tokens.push_back(Token("NEQ", "!=", start_line, start_col));
        } else {
            tokens.push_back(Token("NOT", "!", start_line, start_col));
        }
    } else if (ch == '&') {
        if (peek_actual_char() == '&') {
            get_char();
            tokens.push_back(Token("AND", "&&", start_line, start_col));
        }
    } else if (ch == '|') {
        if (peek_actual_char() == '|') {
            get_char();
            tokens.push_back(Token("OR", "||", start_line, start_col));
        }
    } else if (ch == '(') tokens.push_back(Token("LPAREN", "(", start_line, start_col));
    else if (ch == ')') tokens.push_back(Token("RPAREN", ")", start_line, start_col));
    else if (ch == '{') tokens.push_back(Token("LBRACE", "{", start_line, start_col));
    else if (ch == '}') tokens.push_back(Token("RBRACE", "}", start_line, start_col));
    else if (ch == '[') tokens.push_back(Token("LBRACKET", "[", start_line, start_col));
    else if (ch == ']') tokens.push_back(Token("RBRACKET", "]", start_line, start_col));
    else if (ch == ';') tokens.push_back(Token("SEMICOLON", ";", start_line, start_col));
    else if (ch == ',') tokens.push_back(Token("COMMA", ",", start_line, start_col));
    else if (ch == ':') tokens.push_back(Token("COLON", ":", start_line, start_col));
    else if (ch == '.') tokens.push_back(Token("DOT", ".", start_line, start_col));
    else {
        report_error(std::string("Simbolo invalido '") + ch + "'", start_line, start_col);
    }
}

std::vector<Token> Scanner::scan() {
    std::vector<Token> tokens;
    if (!text_Arr) return tokens;

    while (tokens.empty() || tokens.back().token_name != "EOF") {
        scan_next_tokens(tokens);
    }
    return tokens;
}
