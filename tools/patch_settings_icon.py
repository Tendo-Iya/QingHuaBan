from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\util.cpp")
s = p.read_text(encoding="utf-8")
old = """        case Icon::Page: {
            GraphicsPath p;
            p.AddLine(P(5, 3), P(15, 3));
            p.AddLine(P(15, 3), P(20, 8));
            p.AddLine(P(20, 8), P(20, 21));
            p.AddLine(P(20, 21), P(5, 21));
            p.CloseFigure();
            g.DrawPath(&pen, &p);
            g.DrawLine(&pen, P(15, 3), P(15, 8));
            g.DrawLine(&pen, P(15, 8), P(20, 8));
            g.DrawLine(&pen, P(8.5f, 13), P(16, 13));
            g.DrawLine(&pen, P(8.5f, 17), P(14, 17));
            break;
        }
    }
}"""
new = """        case Icon::Page: {
            GraphicsPath p;
            p.AddLine(P(5, 3), P(15, 3));
            p.AddLine(P(15, 3), P(20, 8));
            p.AddLine(P(20, 8), P(20, 21));
            p.AddLine(P(20, 21), P(5, 21));
            p.CloseFigure();
            g.DrawPath(&pen, &p);
            g.DrawLine(&pen, P(15, 3), P(15, 8));
            g.DrawLine(&pen, P(15, 8), P(20, 8));
            g.DrawLine(&pen, P(8.5f, 13), P(16, 13));
            g.DrawLine(&pen, P(8.5f, 17), P(14, 17));
            break;
        }
        case Icon::Settings: {
            g.DrawLine(&pen, P(4, 7), P(20, 7));
            g.DrawLine(&pen, P(4, 12), P(20, 12));
            g.DrawLine(&pen, P(4, 17), P(20, 17));
            SolidBrush knob(RGBA(color, alpha));
            g.FillEllipse(&knob, P(7, 5), P(4, 4));
            g.FillEllipse(&knob, P(14, 10), P(4, 4));
            g.FillEllipse(&knob, P(9, 15), P(4, 4));
            break;
        }
    }
}"""
if old not in s:
    raise SystemExit("settings icon anchor missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("settings icon added")
