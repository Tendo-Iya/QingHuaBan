from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")

# helpers after copyBitmapFull
anchor = "static void presentBitmap(HDC hdc, Bitmap* bmp, int w, int h) {"
helpers = r'''static bool copyBitmapRect(Bitmap* dst, Bitmap* src, RECT rc, int w, int h) {
    if (!dst || !src) return false;
    rc.left = std::max(0L, rc.left);
    rc.top = std::max(0L, rc.top);
    rc.right = std::min((LONG)w, rc.right);
    rc.bottom = std::min((LONG)h, rc.bottom);
    if (rc.right <= rc.left || rc.bottom <= rc.top) return false;
    Rect full(0, 0, w, h);
    BitmapData sd{}, dd{};
    if (src->LockBits(&full, ImageLockModeRead, PixelFormat32bppPARGB, &sd) != Ok) return false;
    if (dst->LockBits(&full, ImageLockModeWrite, PixelFormat32bppPARGB, &dd) != Ok) {
        src->UnlockBits(&sd);
        return false;
    }
    size_t rowBytes = (size_t)(rc.right - rc.left) * 4;
    for (int y = rc.top; y < rc.bottom; ++y) {
        memcpy((BYTE*)dd.Scan0 + (size_t)y * dd.Stride + (size_t)rc.left * 4,
               (BYTE*)sd.Scan0 + (size_t)y * sd.Stride + (size_t)rc.left * 4, rowBytes);
    }
    dst->UnlockBits(&dd);
    src->UnlockBits(&sd);
    return true;
}

static void presentBitmapRect(HDC hdc, Bitmap* bmp, RECT rc, int w, int h) {
    if (!bmp) return;
    rc.left = std::max(0L, rc.left);
    rc.top = std::max(0L, rc.top);
    rc.right = std::min((LONG)w, rc.right);
    rc.bottom = std::min((LONG)h, rc.bottom);
    if (rc.right <= rc.left || rc.bottom <= rc.top) return;
    Rect full(0, 0, w, h);
    BitmapData bd{};
    if (bmp->LockBits(&full, ImageLockModeRead, PixelFormat32bppPARGB, &bd) != Ok) return;
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = rc.right - rc.left;
    bi.bmiHeader.biHeight = -(rc.bottom - rc.top);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    BYTE* bits = (BYTE*)bd.Scan0 + (size_t)rc.top * bd.Stride + (size_t)rc.left * 4;
    SetDIBitsToDevice(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                      0, 0, 0, rc.bottom - rc.top, bits, &bi, DIB_RGB_COLORS);
    bmp->UnlockBits(&bd);
}

'''
s = s.replace(anchor, helpers + anchor, 1)

# signature and partial branch
s = s.replace("void App::paint(HDC hdc) {", "void App::paint(HDC hdc, const RECT* dirty) {")
s = s.replace("""    if (drag_ == DragMode::Move && sel_ >= 0 && sel_ < (int)page().items.size()) {""",
"""    if (drag_ == DragMode::Draw && dirty && !baseDirty_ && base_ &&
        base_->GetWidth() == W_ && base_->GetHeight() == H_) {
        RECT rc = *dirty;
        rc.left = std::max(0L, rc.left);
        rc.top = std::max(0L, rc.top);
        rc.right = std::min((LONG)W_, rc.right);
        rc.bottom = std::min((LONG)H_, rc.bottom);
        bool fullRect = rc.left <= 0 && rc.top <= 0 && rc.right >= W_ && rc.bottom >= H_;
        if (!fullRect && rc.right > rc.left && rc.bottom > rc.top) {
            copyBitmapRect(back_.get(), base_.get(), rc, W_, H_);
            {
                Graphics g(back_.get());
                setupGraphics(g);
                g.SetCompositingQuality(CompositingQualityHighSpeed);
                g.SetInterpolationMode(InterpolationModeNearestNeighbor);
                g.SetPixelOffsetMode(PixelOffsetModeHalf);
                g.SetClip(RectF((float)rc.left, (float)rc.top, (float)(rc.right - rc.left),
                                (float)(rc.bottom - rc.top)), CombineModeIntersect);
                drawLive(g);
            }
            presentBitmapRect(hdc, back_.get(), rc, W_, H_);
            liveDirtyValid_ = false;
            return;
        }
    }
    if (drag_ == DragMode::Move && sel_ >= 0 && sel_ < (int)page().items.size()) {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("partial dirty paint")
