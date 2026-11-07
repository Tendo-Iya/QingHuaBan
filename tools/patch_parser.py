from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("void drawIcon(Graphics& g, Icon icon, RectF box, uint32_t color, float width, BYTE alpha = 255);",
"""void drawIcon(Graphics& g, Icon icon, RectF box, uint32_t color, float width, BYTE alpha = 255);
bool sampleExpression(const std::string& expr, double xmin, double xmax, int count,
                      std::vector<std::pair<double, double>>& out, std::string& error);""")
s = s.replace("    Cursor, Pen, Highlighter, Eraser, Shape, Text, Laser, Image, Background, Hand,",
              "    Cursor, Pen, Highlighter, Eraser, Shape, Text, Laser, Image, Background, Function, Hand,")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\util.cpp")
s = p.read_text(encoding="utf-8")
if "#include <cmath>" not in s:
    s = s.replace("#include <cstring>", "#include <cmath>\n#include <cctype>\n#include <cstdlib>\n#include <memory>\n#include <cstring>")
impl = r'''
// ---------------------------------------------------------------------------
// tiny math expression parser for the function plotter
// ---------------------------------------------------------------------------
namespace {

struct FNode {
    enum Type { Num, Var, Bin, Unary, Call } type = Num;
    double v = 0.0;
    char op = 0;
    int fn = 0;
    std::unique_ptr<FNode> a, b;
};

enum FnId { FnSin, FnCos, FnTan, FnAsin, FnAcos, FnAtan, FnSqrt, FnAbs,
            FnLn, FnLog, FnExp, FnFloor, FnCeil, FnRound };

class FParser {
public:
    explicit FParser(const std::string& text) : s_(text) {}
    std::unique_ptr<FNode> parse(std::string& err) {
        skip();
        auto n = expr();
        skip();
        if (!err_.empty()) { err = err_; return nullptr; }
        if (p_ != s_.size()) { err = "无法识别的字符: " + s_.substr(p_, 1); return nullptr; }
        return n;
    }
private:
    const std::string& s_;
    size_t p_ = 0;
    std::string err_;

    void skip() { while (p_ < s_.size() && std::isspace((unsigned char)s_[p_])) ++p_; }
    bool eat(char c) { skip(); if (p_ < s_.size() && s_[p_] == c) { ++p_; return true; } return false; }
    char peek() { skip(); return p_ < s_.size() ? s_[p_] : '\0'; }
    bool startsPrimary(char c) const {
        return std::isdigit((unsigned char)c) || std::isalpha((unsigned char)c) || c == '(' || c == '.';
    }
    std::unique_ptr<FNode> binary(char op, std::unique_ptr<FNode> a, std::unique_ptr<FNode> b) {
        auto n = std::make_unique<FNode>();
        n->type = FNode::Bin; n->op = op; n->a = std::move(a); n->b = std::move(b);
        return n;
    }
    std::unique_ptr<FNode> expr() {
        auto a = term();
        while (err_.empty()) {
            char c = peek();
            if (c != '+' && c != '-') break;
            ++p_;
            auto b = term();
            a = binary(c, std::move(a), std::move(b));
        }
        return a;
    }
    std::unique_ptr<FNode> term() {
        auto a = unary();
        while (err_.empty()) {
            char c = peek();
            if (c == '*' || c == '/') {
                ++p_;
                auto b = unary();
                a = binary(c, std::move(a), std::move(b));
            } else if (startsPrimary(c)) {
                auto b = unary();
                a = binary('*', std::move(a), std::move(b));
            } else {
                break;
            }
        }
        return a;
    }
    std::unique_ptr<FNode> unary() {
        char c = peek();
        if (c == '+' || c == '-') {
            ++p_;
            auto n = std::make_unique<FNode>();
            n->type = FNode::Unary; n->op = c; n->a = unary();
            return n;
        }
        return power();
    }
    std::unique_ptr<FNode> power() {
        auto a = primary();
        if (err_.empty() && eat('^')) {
            auto b = unary();
            a = binary('^', std::move(a), std::move(b));
        }
        return a;
    }
    std::unique_ptr<FNode> primary() {
        skip();
        if (p_ >= s_.size()) { err_ = "表达式不完整"; return nullptr; }
        char c = s_[p_];
        if (c == '(') {
            ++p_;
            auto n = expr();
            if (!eat(')')) err_ = "缺少右括号 )";
            return n;
        }
        if (std::isdigit((unsigned char)c) || c == '.') {
            const char* begin = s_.c_str() + p_;
            char* end = nullptr;
            double v = std::strtod(begin, &end);
            if (end == begin) { err_ = "数字格式错误"; return nullptr; }
            p_ += (size_t)(end - begin);
            auto n = std::make_unique<FNode>();
            n->type = FNode::Num; n->v = v;
            return n;
        }
        if (std::isalpha((unsigned char)c)) {
            size_t start = p_;
            while (p_ < s_.size() && (std::isalnum((unsigned char)s_[p_]) || s_[p_] == '_')) ++p_;
            std::string id = s_.substr(start, p_ - start);
            for (auto& ch : id) ch = (char)std::tolower((unsigned char)ch);
            auto n = std::make_unique<FNode>();
            if (id == "x") { n->type = FNode::Var; return n; }
            if (id == "pi") { n->type = FNode::Num; n->v = 3.14159265358979323846; return n; }
            if (id == "e") { n->type = FNode::Num; n->v = 2.71828182845904523536; return n; }
            int fn = -1;
            if (id == "sin") fn = FnSin; else if (id == "cos") fn = FnCos;
            else if (id == "tan") fn = FnTan; else if (id == "asin") fn = FnAsin;
            else if (id == "acos") fn = FnAcos; else if (id == "atan") fn = FnAtan;
            else if (id == "sqrt") fn = FnSqrt; else if (id == "abs") fn = FnAbs;
            else if (id == "ln") fn = FnLn; else if (id == "log") fn = FnLog;
            else if (id == "exp") fn = FnExp; else if (id == "floor") fn = FnFloor;
            else if (id == "ceil") fn = FnCeil; else if (id == "round") fn = FnRound;
            if (fn < 0) { err_ = "未知函数或变量: " + id; return nullptr; }
            if (!eat('(')) { err_ = "函数 " + id + " 需要括号"; return nullptr; }
            auto arg = expr();
            if (!eat(')')) err_ = "函数 " + id + " 缺少右括号";
            n->type = FNode::Call; n->fn = fn; n->a = std::move(arg);
            return n;
        }
        err_ = std::string("无法识别的字符: ") + c;
        return nullptr;
    }
};

double evalF(const FNode* n, double x) {
    if (!n) return std::numeric_limits<double>::quiet_NaN();
    switch (n->type) {
        case FNode::Num: return n->v;
        case FNode::Var: return x;
        case FNode::Unary: {
            double v = evalF(n->a.get(), x);
            return n->op == '-' ? -v : v;
        }
        case FNode::Bin: {
            double a = evalF(n->a.get(), x), b = evalF(n->b.get(), x);
            switch (n->op) {
                case '+': return a + b;
                case '-': return a - b;
                case '*': return a * b;
                case '/': return b == 0.0 ? std::numeric_limits<double>::quiet_NaN() : a / b;
                case '^': return std::pow(a, b);
            }
            return std::numeric_limits<double>::quiet_NaN();
        }
        case FNode::Call: {
            double v = evalF(n->a.get(), x);
            switch (n->fn) {
                case FnSin: return std::sin(v);
                case FnCos: return std::cos(v);
                case FnTan: return std::tan(v);
                case FnAsin: return (v < -1 || v > 1) ? std::numeric_limits<double>::quiet_NaN() : std::asin(v);
                case FnAcos: return (v < -1 || v > 1) ? std::numeric_limits<double>::quiet_NaN() : std::acos(v);
                case FnAtan: return std::atan(v);
                case FnSqrt: return v < 0 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(v);
                case FnAbs: return std::fabs(v);
                case FnLn: return v <= 0 ? std::numeric_limits<double>::quiet_NaN() : std::log(v);
                case FnLog: return v <= 0 ? std::numeric_limits<double>::quiet_NaN() : std::log10(v);
                case FnExp: return std::exp(v);
                case FnFloor: return std::floor(v);
                case FnCeil: return std::ceil(v);
                case FnRound: return std::round(v);
            }
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

}  // namespace

bool sampleExpression(const std::string& expr, double xmin, double xmax, int count,
                      std::vector<std::pair<double, double>>& out, std::string& error) {
    out.clear();
    error.clear();
    std::string text = expr;
    size_t a = text.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) { error = "请输入函数解析式"; return false; }
    text = text.substr(a);
    if (text.rfind("y=", 0) == 0) text = text.substr(2);
    else if (text.rfind("f(x)=", 0) == 0) text = text.substr(5);
    FParser parser(text);
    auto root = parser.parse(error);
    if (!root) return false;
    count = std::max(2, std::min(count, 5000));
    double step = (xmax - xmin) / count;
    out.reserve((size_t)count + 1);
    for (int i = 0; i <= count; ++i) {
        double x = xmin + step * i;
        double y = evalF(root.get(), x);
        if (std::isfinite(y) && std::fabs(y) < 1.0e12) out.push_back({x, y});
    }
    if (out.size() < 2) { error = "这个解析式在范围内没有可绘制的点"; out.clear(); return false; }
    return true;
}

'''
s = s.replace("}  // namespace qhb", impl + "}  // namespace qhb")
p.write_text(s, encoding="utf-8", newline="\n")
print("expression parser added")
