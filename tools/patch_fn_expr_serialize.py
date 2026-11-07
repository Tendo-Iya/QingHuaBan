from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""                       << nf(it.unitY);
                }
                ss << "\\n";""",
"""                       << nf(it.unitY);
                }
                if (!it.funcExpr.empty()) ss << " EXPR " << b64encode(it.funcExpr);
                ss << "\\n";""")
s = s.replace("""                ss << "SHAPE " << sk << " #" << hexColor(it.color) << " " << nf(it.width) << " "
                   << (it.filled ? 1 : 0) << " " << nf(it.x1) << " " << nf(it.y1) << " "
                   << nf(it.x2) << " " << nf(it.y2) << "\\n";""",
"""                ss << "SHAPE " << sk << " #" << hexColor(it.color) << " " << nf(it.width) << " "
                   << (it.filled ? 1 : 0) << " " << nf(it.x1) << " " << nf(it.y1) << " "
                   << nf(it.x2) << " " << nf(it.y2);
                if (it.isPoint) ss << " POINT " << b64encode(it.pointText);
                ss << "\\n";""")
s = s.replace("""                std::string extra;
                if (is >> extra) {
                    if (extra == "FUNC") {
                        int axes = 1;
                        if (!(is >> axes >> it.originX >> it.originY >> it.gLeft >> it.gTop
                              >> it.gRight >> it.gBottom >> it.unitX >> it.unitY)) {
                            err = L"函数图像数据损坏。";
                            return false;
                        }
                        it.functionGraph = true;
                        it.showAxes = axes != 0;
                    }
                }""",
"""                std::string extra;
                while (is >> extra) {
                    if (extra == "FUNC") {
                        int axes = 1;
                        if (!(is >> axes >> it.originX >> it.originY >> it.gLeft >> it.gTop
                              >> it.gRight >> it.gBottom >> it.unitX >> it.unitY)) {
                            err = L"函数图像数据损坏。";
                            return false;
                        }
                        it.functionGraph = true;
                        it.showAxes = axes != 0;
                    } else if (extra == "EXPR") {
                        std::string e;
                        if (is >> e) it.funcExpr = b64decode(e);
                    }
                }""")
s = s.replace("""                else if (sk == "ellipse") it.shape = ShapeKind::Ellipse;
                else it.shape = ShapeKind::Rect;
            } else if (kind == "TEXT") {""",
"""                else if (sk == "ellipse") it.shape = ShapeKind::Ellipse;
                else it.shape = ShapeKind::Rect;
                std::string extra;
                if (is >> extra) {
                    if (extra == "POINT") {
                        std::string pt;
                        if (is >> pt) {
                            it.isPoint = true;
                            it.pointText = b64decode(pt);
                        }
                    }
                }
            } else if (kind == "TEXT") {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("serialize expr/point")
