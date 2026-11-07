from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")

old = """void App::paint(HDC hdc) {
    if (W_ <= 0 || H_ <= 0) return;
    if (!back_ || back_->GetWidth() != W_ || back_->GetHeight() != H_) {
        back_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
    }
    Graphics g(back_.get());
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetCompositingMode(CompositingModeSourceOver);
    render(g);
    Graphics dst(hdc);
    dst.DrawImage(back_.get(), 0, 0, W_, H_);
}"""
new = """static void setupGraphics(Graphics& g) {
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetCompositingMode(CompositingModeSourceOver);
}

void App::renderBase(Graphics& g) {
    regions_.clear();
    drawBackdrop(g);
    if (splash_) {
        drawSplash(g);
        return;
    }
    drawPage(g);
    drawHeader(g);
    drawDock(g);
}

void App::paint(HDC hdc) {
    if (W_ <= 0 || H_ <= 0) return;
    if (!back_ || back_->GetWidth() != W_ || back_->GetHeight() != H_) {
        back_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
        base_.reset();
        baseDirty_ = true;
    }
    if (isLiveDrawing()) {
        if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_ || baseDirty_) {
            if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_)
                base_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
            Graphics bg(base_.get());
            setupGraphics(bg);
            renderBase(bg);
            baseDirty_ = false;
        }
        Graphics g(back_.get());
        setupGraphics(g);
        g.DrawImage(base_.get(), 0, 0, W_, H_);
        drawLive(g);
        drawToast(g);
    } else {
        Graphics g(back_.get());
        setupGraphics(g);
        render(g);
        baseDirty_ = true;
    }
    Graphics dst(hdc);
    dst.DrawImage(back_.get(), 0, 0, W_, H_);
}"""
if old not in s:
    raise SystemExit("paint anchor missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("base-cache paint patched")
