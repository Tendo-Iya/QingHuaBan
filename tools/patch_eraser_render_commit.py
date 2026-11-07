from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
anchor = "    if (drag_ == DragMode::Move && sel_ >= 0 && sel_ < (int)page().items.size()) {"
branch = r'''    if (drag_ == DragMode::Erase && back_ && back_->GetWidth() == W_ && back_->GetHeight() == H_) {
        Graphics g(back_.get());
        setupGraphics(g);
        SolidBrush paper(RGBA(page().paper));
        float radius = std::max(1.0f, eraserSize_ * viewScale() / 2.0f);
        Xf xf = pageXf();
        for (size_t i = eraserDrawn_; i < eraserPath_.size(); ++i) {
            Pt sp = xf.to(eraserPath_[i]);
            g.FillEllipse(&paper, sp.x - radius, sp.y - radius, radius * 2, radius * 2);
        }
        eraserDrawn_ = eraserPath_.size();
        regions_.clear();
        drawHeader(g);
        drawDock(g);
        drawToast(g);
        if (mouseInside_ && !overlay_) {
            Pt sp = xf.to(screenToWorld(mouse_));
            Pen ring(RGBA(0x5A6A60, 180), std::max(1.0f, S(1.2f)));
            g.DrawEllipse(&ring, sp.x - radius, sp.y - radius, radius * 2, radius * 2);
        }
        presentBitmap(hdc, back_.get(), W_, H_);
        return;
    }
'''
s = s.replace(anchor, branch + anchor, 1)
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    for (const Item& item : page().items) {
        if (item.kind == ItemKind::Stroke && item.points.size() > 1) {
            std::vector<Pt> run;
            std::vector<std::vector<Pt>> pieces;
            bool cut = false;
            for (const Pt& p : item.points) {""",
"""    for (const Item& item : page().items) {
        RectF ib = item.bounds();
        if (world.x < ib.X - tol || world.x > ib.GetRight() + tol ||
            world.y < ib.Y - tol || world.y > ib.GetBottom() + tol) {
            out.push_back(item);
            continue;
        }
        if (item.kind == ItemKind::Stroke && item.points.size() > 1) {
            std::vector<Pt> dense;
            if (item.points.size() >= 2) {
                float step = std::max(1.2f, tol * 0.4f);
                dense.push_back(item.points[0]);
                for (size_t si = 1; si < item.points.size(); ++si) {
                    Pt a = item.points[si - 1], b = item.points[si];
                    float d = std::hypot(b.x - a.x, b.y - a.y);
                    int n = std::max(1, std::min(24, (int)std::ceil(d / step)));
                    for (int k = 1; k <= n; ++k) {
                        float t = (float)k / n;
                        dense.push_back({a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t});
                    }
                }
            }
            std::vector<Pt> run;
            std::vector<std::vector<Pt>> pieces;
            bool cut = false;
            for (const Pt& p : dense) {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("eraser overlay and dense geometry commit")
