from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
# Reset move cache for resize and start timer
s = s.replace("""                drag_ = DragMode::Resize;
                dragOrig_ = page().items[sel_];""",
"""                drag_ = DragMode::Resize;
                moveBase_.reset();
                SetTimer(hwnd_, 6, 8, nullptr);
                dragOrig_ = page().items[sel_];""")
s = s.replace("""                drag_ = DragMode::Move;
                moveBase_.reset();
                dragOrig_ = page().items[idx];""",
"""                drag_ = DragMode::Move;
                moveBase_.reset();
                SetTimer(hwnd_, 6, 8, nullptr);
                dragOrig_ = page().items[idx];""")
# Timer6 supports move/resize; sampling only draw/erase
s = s.replace("""        bool draw = drag_ == DragMode::Draw;
        bool erase = drag_ == DragMode::Erase;
        if ((draw || erase) && hwnd_ && IsWindowVisible(hwnd_)) {""",
"""        bool draw = drag_ == DragMode::Draw;
        bool erase = drag_ == DragMode::Erase;
        bool moving = drag_ == DragMode::Move || drag_ == DragMode::Resize;
        if ((draw || erase || moving) && hwnd_ && IsWindowVisible(hwnd_)) {""")
s = s.replace("""            if (got) {
                ScreenToClient(hwnd_, &pt);
                Pt w = screenToWorld({(float)pt.x, (float)pt.y});
                if (draw) addLiveSample(w);
                else addEraserSample(w);
            }
            UpdateWindow(hwnd_);""",
"""            if (got && (draw || erase)) {
                ScreenToClient(hwnd_, &pt);
                Pt w = screenToWorld({(float)pt.x, (float)pt.y});
                if (draw) addLiveSample(w);
                else addEraserSample(w);
            }
            UpdateWindow(hwnd_);""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("if (drag_ == DragMode::Move && sel_ >= 0 && sel_ < (int)page().items.size()) {",
              "if ((drag_ == DragMode::Move || drag_ == DragMode::Resize) && sel_ >= 0 && sel_ < (int)page().items.size()) {")
p.write_text(s, encoding="utf-8", newline="\n")
print("fast move/resize cache and timer")
