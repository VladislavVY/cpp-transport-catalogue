#include "json.h"

using namespace std;

namespace json {

namespace {
using namespace std::literals;

using Number = std::variant<int, double>;

Node LoadNode(istream& input);
Node LoadString(std::istream& input);
    
std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}    

Node LoadArray(std::istream& input) {
    std::vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict dict;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (dict.find(key) != dict.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            } else {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        } else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}    

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}
    
Node LoadBool(std::istream& input) {
    const auto s = LoadLiteral(input);
    if (s == "true"sv) {
        return Node{true};
    } else if (s == "false"sv) {
        return Node{false};
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}

Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node{nullptr};
    } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}    

Node LoadNumber(std::istream& input) {
    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
            input.putback(c);
            return LoadBool(input);
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}
    
} 

bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}

int Node::AsInt() const {
        using namespace std::literals;
        if (!IsInt()) {
            throw std::logic_error("Not an int"s);
        }
   return std::get<int>(value_);
}    
   
bool Node::IsPureDouble() const {
    return holds_alternative<double>(value_);
}    

bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
}        

double Node::AsDouble() const {
        using namespace std::literals;
        if (!IsDouble()) {
            throw std::logic_error("Not a double"s);
        }
    return IsPureDouble() ? std::get<double>(value_) : AsInt();
}     
    
bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}

 bool Node::AsBool() const {
        using namespace std::literals;
        if (!IsBool()) {
            throw std::logic_error("Not a bool"s);
        }

    return std::get<bool>(value_);
}    

bool Node::IsNull() const {
    return holds_alternative<std::nullptr_t>(value_);
}    
  
bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}    

const Array& Node::AsArray() const {
        using namespace std::literals;
        if (!IsArray()) {
            throw std::logic_error("Not an array"s);
        }

        return std::get<Array>(value_);
    }    
    
bool Node::IsString() const {
    return holds_alternative<std::string>(value_);
}

const std::string& Node::AsString() const {
        using namespace std::literals;
        if (!IsString()) {
            throw std::logic_error("Not a string"s);
        }

      return std::get<std::string>(value_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}

const Dict& Node::AsMap() const {
        using namespace std::literals;
        if (!IsMap()) {
            throw std::logic_error("Not a map"s);
        }

     return std::get<Dict>(value_);
}    
    
bool Node::operator==(const Node& rhs) const {
    return value_ == rhs.value_;
}
 
bool Node::operator!=(const Node& rhs) const {
    return !(value_ == rhs.value_);
}    
    
const Node::Value& Node::GetValue() const {
    return value_;
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& rhs) const {
    return root_ == rhs.root_;
}

bool Document::operator!=(const Document& rhs) const {
    return !(root_ == rhs.root_);
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(std::string value, const PrintContext& ctx) {
    ctx.out << "\""sv;
    for (const char& c : value) {
        switch (c) {
            case '\r':
                ctx.out << "\\r"sv;
                continue;
            case '\n':
                ctx.out << "\\n"sv;
                continue;
            case '\t':
                ctx.out << "\\t"sv;
                continue;
            case '\"':
                 [[fallthrough]];
            case '\\':
                ctx.out << "\\"sv;
                 [[fallthrough]];
            default:
                ctx.out << c;
                break;
        }
    }
     ctx.out << "\""sv;
}    
    
void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << std::boolalpha << value;
}

void PrintValue(Array array, const PrintContext& ctx) {
    ctx.out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& node : array) {
        if (first) {
            first = false;
        } else {
            ctx.out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "]"sv;
}

void PrintValue(const Dict& nodes, const PrintContext& ctx) {
    ctx.out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            ctx.out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintValue(key, ctx);
        ctx.out << ": ";
        PrintNode(node, inner_ctx);
    }
    ctx.out << "\n"s;
    ctx.PrintIndent();
    ctx.out << "}"sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) { PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext ctx{ output };
    PrintNode(doc.GetRoot(), ctx);
}

}  // namespace json
