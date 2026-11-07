from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")

# Replace fastBlit/blitBase helpers with DIB copy/present
start = s.index("static bool fastBlit(")
end = s.index("void App::renderBase(Graphics& g) {", start)
new_helpers = r'''static bool copyBitmapFull(Bitmap* dst, Bitmap* src, int w, int h) {
    if (!dst || !src || dst->GetWidth() != w || dst->GetHeight() != h ||
        src->GetWidth() != w || src->GetHeight() != h) return false;
    Rect rect(0, 0, w, h);
    BitmapData sd{}, dd{};
    if (src->LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &sd) != Ok) return false;
    if (dst->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppPARGB, &dd) != Ok) {
        src->UnlockBits(&sd);
        return false;
    }
    bool ok = true;
    if (sd.Stride == dd.Stride) {
        memcpy(dd.Scan0, sd.Scan0, (size_t)sd.Stride * h);
    } else {
        for (int y = 0; y < h; ++y)
            memcpy((BYTE*)dd.Scan0 + (size_t)y * dd.Stride,
                   (BYTE*)sd.Scan0 + (size_t)y * sd.Stride, (size_t)w * 4);
    }
    dst->UnlockBits(&dd);
    src->UnlockBits(&sd);
    return ok;
}

static void presentBitmap(HDC hdc, Bitmap* bmp, int w, int h) {
    if (!bmp) return;
    Rect rect(0, 0, w, h);
    BitmapData bd{};
    if (bmp->LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &bd) != Ok) {
        Graphics out(hdc);
        out.DrawImage(bmp, 0, 0, w, h);
        return;
    }
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    SetDIBitsToDevice(hdc, 0, 0, w, h, 0, 0, 0, h, bd.Scan0, &bi, DIB_RGB_COLORS);
    bmp->UnlockBits(&bd);
}

'''
s = s[:start] + new_helpers + s[end:]

# Rewrite paint body core
old = s[s.index("void App::paint(HDC hdc) {"):s.index("void App::render(Graphics& g) {")]
new = r'''void App::paint(HDC hdc) {
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
        copyBitmapFull(back_.get(), base_.get(), W_, H_);
        {
            Graphics g(back_.get());
            setupGraphics(g);
            g.SetCompositingQuality(CompositingQualityHighSpeed);
            g.SetInterpolationMode(InterpolationModeNearestNeighbor);
            g.SetPixelOffsetMode(PixelOffsetModeHalf);
            drawLive(g);
            drawToast(g);
        }
    } else {
        {
            Graphics g(back_.get());
            setupGraphics(g);
            render(g);
        }
        ULONGLONG animNow = GetTickCount64();
        bool animating = splash_ || splashLeaving_ || overlay_ == 3 ||
                         (boardIntroStart_ != 0 && animNow - boardIntroStart_ < 260) ||
                         (pageFadeStart_ != 0 && animNow - pageFadeStart_ < 220) ||
                         (toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180);
        if (!animating) {
            if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_)
                base_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
            copyBitmapFull(base_.get(), back_.get(), W_, H_);
            baseDirty_ = false;
        } else {
            baseDirty_ = true;
        }
    }
    {
        Graphics ov(back_.get());
        setupGraphics(ov);
        ULONGLONG now = GetTickCount64();
        if (boardIntroStart_ != 0 && now - boardIntroStart_ < 260) {
            float t = easeOutCubic((float)(now - boardIntroStart_) / 260.0f);
            SolidBrush veil(RGBA(0xFFFFFF, (BYTE)(190 * (1.0f - t))));
            ov.FillRectangle(&veil, RectF(0, 0, (float)W_, (float)H_));
        }
        if (pageFadeStart_ != 0 && now - pageFadeStart_ < 220) {
            float t = easeOutCubic((float)(now - pageFadeStart_) / 220.0f);
            SolidBrush veil(RGBA(0xFFFFFF, (BYTE)(105 * (1.0f - t))));
            ov.FillRectangle(&veil, RectF(0, 0, (float)W_, (float)H_));
        }
    }
    presentBitmap(hdc, back_.get(), W_, H_);
}

'''
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("DIB copy/present patched")
