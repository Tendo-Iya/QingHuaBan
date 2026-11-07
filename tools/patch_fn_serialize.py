from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""                for (auto& pt : it.points) ss << " " << nf(pt.x) << "," << nf(pt.y);
                ss << "\\n";""",
"""                for (auto& pt : it.points) ss << " " << nf(pt.x) << "," << nf(pt.y);
                if (it.functionGraph) {
                    ss << " FUNC " << (it.showAxes ? 1 : 0) << " " << nf(it.originX) << " "
                       << nf(it.originY) << " " << nf(it.gLeft) << " " << nf(it.gTop) << " "
                       << nf(it.gRight) << " " << nf(it.gBottom) << " " << nf(it.unitX) << " "
                       << nf(it.unitY);
                }
                ss << "\\n";""")
s = s.replace("""                    it.points.push_back({std::stof(xy.substr(0, comma)), std::stof(xy.substr(comma + 1))});
                }
            } else if (kind == "SHAPE") {""",
"""                    it.points.push_back({std::stof(xy.substr(0, comma)), std::stof(xy.substr(comma + 1))});
                }
                std::string extra;
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
                }
            } else if (kind == "SHAPE") {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("function graph serialization")
