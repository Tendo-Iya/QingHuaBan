from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    bool baseDirty_ = true;", "    bool baseDirty_ = true;\n    ULONGLONG lastLivePaint_ = 0;")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("void App::invalidateLive() { InvalidateRect(hwnd_, nullptr, FALSE); }",
"""void App::invalidateLive() {
    InvalidateRect(hwnd_, nullptr, FALSE);
    ULONGLONG now = GetTickCount64();
    if (now - lastLivePaint_ >= 8) {
        lastLivePaint_ = now;
        if (hwnd_ && IsWindowVisible(hwnd_)) UpdateWindow(hwnd_);
    }
}""")
s = s.replace("""    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    SetCapture(hwnd_);
    invalidate();""",
"""    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    SetCapture(hwnd_);
    invalidateLive();""")
s = s.replace("""    previewShape_.y1 = previewShape_.y2 = clampf(world.y, 1, PAGE_H - 1);
    SetCapture(hwnd_);
    invalidate();""",
"""    previewShape_.y1 = previewShape_.y2 = clampf(world.y, 1, PAGE_H - 1);
    SetCapture(hwnd_);
    invalidateLive();""")
s = s.replace("""    lastLaserTime_ = GetTickCount64();
    SetTimer(hwnd_, 3, 30, nullptr);
    SetCapture(hwnd_);
    invalidate();""",
"""    lastLaserTime_ = GetTickCount64();
    SetTimer(hwnd_, 3, 30, nullptr);
    SetCapture(hwnd_);
    invalidateLive();""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
# fast blit helper before paint
s = s.replace("""void App::renderBase(Graphics& g) {""",
"""static void blitBase(Graphics& g, Bitmap* base, int w, int h) {
    g.SetCompositingMode(CompositingModeSourceCopy);
    g.SetInterpolationMode(InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);
    g.DrawImage(base, RectF(0, 0, (float)w, (float)h));
    g.SetCompositingMode(CompositingModeSourceOver);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
}

void App::renderBase(Graphics& g) {""")
# live base copy
s = s.replace("""        Graphics g(back_.get());
        setupGraphics(g);
        g.DrawImage(base_.get(), 0, 0, W_, H_);
        drawLive(g);
        drawToast(g);""",
"""        Graphics g(back_.get());
        setupGraphics(g);
        blitBase(g, base_.get(), W_, H_);
        drawLive(g);
        drawToast(g);""")
# full render -> cache base
s = s.replace("""        Graphics g(back_.get());
        setupGraphics(g);
        render(g);
        baseDirty_ = true;
    }""",
"""        Graphics g(back_.get());
        setupGraphics(g);
        render(g);
        if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_)
            base_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
        Graphics bg(base_.get());
        setupGraphics(bg);
        blitBase(bg, back_.get(), W_, H_);
        baseDirty_ = false;
    }""")
# fast live stroke helper and use
s = s.replace("""void App::drawLive(Graphics& g) {
    if (drag_ == DragMode::Draw && !liveStroke_.points.empty()) {
        drawItem(g, liveStroke_, pageXf(), false, false);
    }""",
"""static void drawFastStroke(Graphics& g, const Item& it, Xf xf) {
    if (it.points.empty()) return;
    BYTE alpha = (it.strokeTool == L"highlighter") ? 82 : 255;
    Color col = RGBA(it.color, alpha);
    float w = std::max(0.7f, it.width * xf.s);
    if (it.points.size() == 1) {
        SolidBrush b(col);
        float r = std::max(0.7f, w / 2);
        Pt p = xf.to(it.points[0]);
        g.FillEllipse(&b, p.x - r, p.y - r, r * 2, r * 2);
        return;
    }
    std::vector<PointF> pts;
    pts.reserve(it.points.size());
    for (auto& p : it.points) {
        Pt q = xf.to(p);
        pts.push_back(PointF(q.x, q.y));
    }
    Pen pen(col, w);
    pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    pen.SetLineJoin(LineJoinRound);
    g.DrawLines(&pen, pts.data(), (INT)pts.size());
}

void App::drawLive(Graphics& g) {
    if (drag_ == DragMode::Draw && !liveStroke_.points.empty()) {
        drawFastStroke(g, liveStroke_, pageXf());
    }""")
p.write_text(s, encoding="utf-8", newline="\n")
print("low-latency live rendering patched")
