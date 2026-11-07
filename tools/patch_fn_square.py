from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\util.cpp")
s = p.read_text(encoding="utf-8")
# normalize unicode operators/superscripts before parser
s = s.replace("""    std::string text = expr;
    size_t a = text.find_first_not_of(" \\t\\r\\n");""",
"""    std::string text = expr;
    auto replaceAll = [&](const std::string& from, const std::string& to) {
        size_t at = 0;
        while ((at = text.find(from, at)) != std::string::npos) {
            text.replace(at, from.size(), to);
            at += to.size();
        }
    };
    replaceAll("\\xC2\\xB2", "^2");
    replaceAll("\\xC2\\xB3", "^3");
    replaceAll("\\xC3\\x97", "*");
    replaceAll("\\xC3\\xB7", "/");
    replaceAll("\\xE2\\x88\\x92", "-");
    replaceAll("\\xCF\\x80", "pi");
    size_t a = text.find_first_not_of(" \\t\\r\\n");""")
# x2 shorthand
s = s.replace("""            if (id == "x") { n->type = FNode::Var; return n; }""",
"""            if (id == "x") {
                n->type = FNode::Var;
                if (p_ < s_.size() && (s_[p_] == '2' || s_[p_] == '3')) {
                    char d = s_[p_++];
                    auto e = std::make_unique<FNode>();
                    e->type = FNode::Num;
                    e->v = d == '2' ? 2.0 : 3.0;
                    return binary('^', std::move(n), std::move(e));
                }
                return n;
            }""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""bool App::onChar(wchar_t ch) {
    if (overlay_ != 4) return false;
    if (ch >= 32 && ch < 127 && fnExpr_.size() < 120) {
        fnExpr_.push_back((char)ch);
        fnError_.clear();
        invalidate();
        return true;
    }
    return false;
}""",
"""bool App::onChar(wchar_t ch) {
    if (overlay_ != 4) return false;
    std::string add;
    if (ch >= 32 && ch < 127) add.push_back((char)ch);
    else if (ch == 0x00B2) add = "^2";
    else if (ch == 0x00B3) add = "^3";
    else if (ch == 0x00D7) add = "*";
    else if (ch == 0x00F7) add = "/";
    else if (ch == 0x2212) add = "-";
    else if (ch == 0x03C0) add = "pi";
    if (!add.empty() && fnExpr_.size() + add.size() < 120) {
        fnExpr_ += add;
        fnError_.clear();
        invalidate();
        return true;
    }
    return false;
}""")
p.write_text(s, encoding="utf-8", newline="\n")
print("square/unicode input support")
