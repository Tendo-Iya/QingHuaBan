from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""        } else {
            std::vector<PointF> pts;
            pts.reserve(it.points.size());
            for (auto& p : it.points) {
                Pt q = xf.to(p);
                pts.push_back(PointF(q.x, q.y));
            }
            GraphicsPath path;
            path.AddCurve(pts.data(), (INT)pts.size(), 0.45f);
            g.DrawPath(&pen, &path);
        }""",
"""        } else {
            std::vector<PointF> pts;
            pts.reserve(it.points.size());
            for (auto& p : it.points) {
                Pt q = xf.to(p);
                pts.push_back(PointF(q.x, q.y));
            }
            g.DrawLines(&pen, pts.data(), (INT)pts.size());
        }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("stroke final now uses raw polyline (no lift-time curve)")
