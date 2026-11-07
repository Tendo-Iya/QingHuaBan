from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    std::unique_ptr<Bitmap> dockDragBase_;",
              "    std::unique_ptr<Bitmap> dockDragBase_;\n    std::unique_ptr<Bitmap> moveBase_;\n    int hideItemForDrag_ = -1;")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""                sel_ = idx;
                drag_ = DragMode::Move;
                dragOrig_ = page().items[idx];""",
"""                sel_ = idx;
                drag_ = DragMode::Move;
                moveBase_.reset();
                dragOrig_ = page().items[idx];""")
s = s.replace("""    if (hEdit_) commitText();
    if (hNameEdit_) commitAppName();
    dockDragBase_.reset();""",
"""    if (hEdit_) commitText();
    if (hNameEdit_) commitAppName();
    dockDragBase_.reset();
    moveBase_.reset();""")
# Skip hidden item in drawPage loop and drawPageContent
p2 = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
r = p2.read_text(encoding="utf-8")
r = r.replace("""    for (int i = 0; i < (int)page().items.size(); ++i) {
        bool hide = hideEditing && i == editIndex_;
        drawItem(g, page().items[i], pageXf(), hide, false);
    }""",
"""    for (int i = 0; i < (int)page().items.size(); ++i) {
        if (i == hideItemForDrag_) continue;
        bool hide = hideEditing && i == editIndex_;
        drawItem(g, page().items[i], pageXf(), hide, false);
    }""")
r = r.replace("""    for (int i = 0; i < (int)p.items.size(); ++i) {
        bool hide = hideEditing && hEdit_ && i == editIndex_;
        drawItem(g, p.items[i], xf, hide, true);
    }""",
"""    for (int i = 0; i < (int)p.items.size(); ++i) {
        if (i == hideItemForDrag_) continue;
        bool hide = hideEditing && hEdit_ && i == editIndex_;
        drawItem(g, p.items[i], xf, hide, true);
    }""")
# Move drag fast frame before dock branch
r = r.replace("""    if (drag_ == DragMode::Dock) {""",
"""    if (drag_ == DragMode::Move && sel_ >= 0 && sel_ < (int)page().items.size()) {
        if (!moveBase_ || moveBase_->GetWidth() != W_ || moveBase_->GetHeight() != H_) {
            moveBase_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
            int oldHide = hideItemForDrag_;
            hideItemForDrag_ = sel_;
            regions_.clear();
            Graphics bg(moveBase_.get());
            setupGraphics(bg);
            drawBackdrop(bg);
            drawPage(bg);
            drawHeader(bg);
            drawDock(bg);
            hideItemForDrag_ = oldHide;
        }
        copyBitmapFull(back_.get(), moveBase_.get(), W_, H_);
        {
            Graphics g(back_.get());
            setupGraphics(g);
            drawItem(g, page().items[sel_], pageXf(), false, false);
            drawSelection(g);
            drawToast(g);
        }
        presentBitmap(hdc, back_.get(), W_, H_);
        return;
    }
    if (drag_ == DragMode::Dock) {""")
p2.write_text(r, encoding="utf-8", newline="\n")
p.write_text(s, encoding="utf-8", newline="\n")
print("fast function/item drag cache")
