from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")

# dock drag fast frame
s = s.replace("""    if (isLiveDrawing()) {
        if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_ || baseDirty_) {""",
"""    if (drag_ == DragMode::Dock) {
        if (!dockDragBase_ || dockDragBase_->GetWidth() != W_ || dockDragBase_->GetHeight() != H_) {
            dockDragBase_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
            regions_.clear();
            Graphics bg(dockDragBase_.get());
            setupGraphics(bg);
            drawBackdrop(bg);
            drawPage(bg);
            drawHeader(bg);
            drawSelection(bg);
        }
        copyBitmapFull(back_.get(), dockDragBase_.get(), W_, H_);
        {
            Graphics g(back_.get());
            setupGraphics(g);
            regions_.clear();
            drawDock(g);
            drawToast(g);
        }
        presentBitmap(hdc, back_.get(), W_, H_);
        return;
    }
    if (isLiveDrawing()) {
        if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_ || baseDirty_) {""")

# tooltip animation in animating
s = s.replace("""                         (toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180) || toolAnimStart_ != 0;""",
"""                         (toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180) || toolAnimStart_ != 0 ||
                         (!hoverId_.empty() && mouseInside_ && animNow - hoverSince_ >= 45 &&
                          animNow - hoverSince_ < 190);""")

# replace drawTooltip
start = s.index("void App::drawTooltip(Graphics& g) {")
end = s.index("// ---------------------------------------------------------------------------\n// overlays", start)
new = r'''void App::drawTooltip(Graphics& g) {
    if (hoverId_.empty() || !mouseInside_ || drag_ != DragMode::None || hEdit_) return;
    ULONGLONG elapsed = GetTickCount64() - hoverSince_;
    if (elapsed < 45) return;
    Region* reg = findRegion(hoverId_);
    if (!reg || reg->tip.empty()) return;
    RectF measured;
    g.MeasureString(reg->tip.c_str(), -1, fMini_.get(), PointF(0, 0), &measured);
    float w = measured.Width + S(22);
    float h = S(28);
    float fx = (float)mouse_.x + S(14);
    float fy = (float)mouse_.y + S(20);
    if (fx + w > W_ - S(8)) fx = (float)mouse_.x - w - S(10);
    if (fy + h > H_ - S(8)) fy = (float)mouse_.y - h - S(10);
    fx = std::max(S(8), fx);
    fy = std::max(headerH() + S(4), fy);

    float t = std::min(1.0f, (float)(elapsed - 45) / 130.0f);
    float e = easeOutCubic(t);
    float ax = reg->rect.X + reg->rect.Width / 2.0f;
    float ay = reg->rect.Y + reg->rect.Height / 2.0f;
    float fcx = fx + w / 2.0f, fcy = fy + h / 2.0f;
    float cx = ax + (fcx - ax) * e;
    float cy = ay + (fcy - ay) * e;
    float sc = 0.82f + 0.18f * e;
    float x = cx - w / 2.0f, y = cy - h / 2.0f;

    GraphicsState st = g.Save();
    g.TranslateTransform(cx, cy);
    g.ScaleTransform(sc, sc);
    g.TranslateTransform(-cx, -cy);
    RectF r(x, y, w, h);
    drawShadow(g, r, S(8), S(5), S(2));
    fillRoundRect(g, r, S(8), 0x202633, 245);
    drawTextAligned(g, reg->tip, RectF(x + S(11), y, w - S(18), h), fMini_, COL_WHITE,
                    StringAlignmentNear, StringAlignmentCenter, false);
    g.Restore(st);
    if (t < 1.0f) SetTimer(hwnd_, 5, 16, nullptr);
}

'''
s = s[:start] + new + s[end:]
p.write_text(s, encoding="utf-8", newline="\n")
print("dock drag frame + pop tooltip")
