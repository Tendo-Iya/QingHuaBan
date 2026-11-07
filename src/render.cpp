#include "qhb.h"

#include <cwchar>

namespace qhb {

static std::string strOf(int v) { return std::to_string(v); }

std::wstring toolName(Tool t) {
    switch (t) {
        case Tool::Select: return L"选择";
        case Tool::Pen: return L"画笔";
        case Tool::Highlighter: return L"荧光笔";
        case Tool::Eraser: return L"橡皮擦";
        case Tool::Shape: return L"形状";
        case Tool::Text: return L"文字";
        case Tool::Laser: return L"激光笔";
        case Tool::Background: return L"画布";
        case Tool::Hand: return L"移动画布";
    }
    return L"画笔";
}

const wchar_t* toolHint(Tool t) {
    switch (t) {
        case Tool::Select: return L"点选对象后拖动，双击文字可编辑";
        case Tool::Pen: return L"按住鼠标书写";
        case Tool::Highlighter: return L"半透明荧光标记";
        case Tool::Eraser: return L"整笔擦除";
        case Tool::Shape: return L"拖拽绘制，Shift 保持比例";
        case Tool::Text: return L"点击页面输入文字";
        case Tool::Laser: return L"按住拖动，松手渐隐";
        case Tool::Background: return L"选择画布颜色和底纹";
        case Tool::Hand: return L"拖动画布";
    }
    return L"";
}


static void addArrow(Graphics& g, PointF a, PointF b, float width, uint32_t color) {
    float dx = b.X - a.X, dy = b.Y - a.Y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.1f) return;
    float head = std::min(std::max(width * 4.0f, 10.0f), len * 0.5f);
    PointF u{dx / len, dy / len};
    PointF n{-u.Y, u.X};
    PointF p1{b.X - u.X * head + n.X * head * 0.55f, b.Y - u.Y * head + n.Y * head * 0.55f};
    PointF p2{b.X - u.X * head - n.X * head * 0.55f, b.Y - u.Y * head - n.Y * head * 0.55f};
    std::vector<PointF> pts = {b, p1, p2};
    GraphicsPath path;
    path.AddPolygon(pts.data(), (INT)pts.size());
    SolidBrush br(RGBA(color));
    g.FillPath(&br, &path);
}

static void setupGraphics(Graphics& g) {
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetCompositingMode(CompositingModeSourceOver);
}

static bool copyBitmapFull(Bitmap* dst, Bitmap* src, int w, int h) {
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

static bool copyBitmapRect(Bitmap* dst, Bitmap* src, RECT rc, int w, int h) {
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

static void drawFastStroke(Graphics& g, const Item& it, Xf xf);

void App::paint(HDC hdc, const RECT* dirty) {
    if (W_ <= 0 || H_ <= 0) return;
    if (!back_ || back_->GetWidth() != W_ || back_->GetHeight() != H_) {
        back_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
        base_.reset();
        baseDirty_ = true;
    }
    if (drag_ == DragMode::Draw && back_ &&
        back_->GetWidth() == W_ && back_->GetHeight() == H_) {
        Graphics g(back_.get());
        setupGraphics(g);
        g.SetCompositingQuality(CompositingQualityHighSpeed);
        g.SetInterpolationMode(InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(PixelOffsetModeHalf);
        size_t n = liveStroke_.points.size();
        if (liveInkDrawn_ == 0 || liveInkDrawn_ > n) {
            drawFastStroke(g, liveStroke_, pageXf());
            liveInkDrawn_ = n;
        } else if (n > liveInkDrawn_) {
            Item tail = liveStroke_;
            size_t begin = liveInkDrawn_ > 0 ? liveInkDrawn_ - 1 : 0;
            tail.points.assign(liveStroke_.points.begin() + begin, liveStroke_.points.end());
            drawFastStroke(g, tail, pageXf());
            liveInkDrawn_ = n;
        }
        presentBitmap(hdc, back_.get(), W_, H_);
        liveDirtyValid_ = false;
        return;
    }
    if (drag_ == DragMode::Erase && back_ && back_->GetWidth() == W_ && back_->GetHeight() == H_) {
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
            Pt sp = xf.to(screenToWorld({(float)mouse_.x, (float)mouse_.y}));
            Pen ring(RGBA(0x5A6A60, 180), std::max(1.0f, S(1.2f)));
            g.DrawEllipse(&ring, sp.x - radius, sp.y - radius, radius * 2, radius * 2);
        }
        presentBitmap(hdc, back_.get(), W_, H_);
        return;
    }
    if ((drag_ == DragMode::Move || drag_ == DragMode::Resize) && sel_ >= 0 && sel_ < (int)page().items.size()) {
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
    if (drag_ == DragMode::Dock) {
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
        liveInkDrawn_ = liveStroke_.points.size();
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
                         (toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180) || toolAnimStart_ != 0 ||
                         (!hoverId_.empty() && mouseInside_ && animNow - hoverSince_ >= 45 &&
                          animNow - hoverSince_ < 190);
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
    liveDirtyValid_ = false;
}

void App::render(Graphics& g) {
    regions_.clear();
    drawBackdrop(g);
    if (splash_) {
        drawSplash(g);
        return;
    }
    drawPage(g);
    drawHeader(g);
    drawSelection(g);
    drawLive(g);
    drawDock(g);
    drawTextEditorHint(g);
    drawOverlay(g);
    drawToast(g);
    drawTooltip(g);
}

// ---------------------------------------------------------------------------
// header
// ---------------------------------------------------------------------------

void App::drawHeaderButton(Graphics& g, const std::string& id, RectF r, Icon ic, const std::wstring& label,
                           bool primary, bool outline, bool enabled, const std::wstring& tip) {
    bool hover = enabled && hoverId_ == id;
    if (primary) {
        fillRoundRect(g, r, S(10), hover ? 0x2457D6 : COL_ACCENT);
    } else if (outline) {
        fillRoundRect(g, r, S(10), hover ? 0xF7F9FC : COL_WHITE);
        strokeRoundRect(g, r, S(10), COL_LINE, S(1));
    } else if (hover) {
        fillRoundRect(g, r, S(10), 0xF3F6FB);
    }
    uint32_t fg = primary ? COL_WHITE : (enabled ? 0x3B4555 : 0xB7C0CC);
    float iconSize = S(17);
    float ix = r.X + S(13);
    drawIcon(g, ic, RectF(ix, r.Y + (r.Height - iconSize) / 2, iconSize, iconSize), fg, S(1.6));
    float tx = ix + iconSize + S(7);
    RectF tr(tx, r.Y, r.GetRight() - tx - S(6), r.Height);
    drawTextAligned(g, label, tr, fUI_, fg, StringAlignmentNear, StringAlignmentCenter, true);
    if (id == "header:save" && dirty_) {
        SolidBrush dot(RGBA(COL_YELLOW));
        g.FillEllipse(&dot, r.GetRight() - S(10), r.Y + S(6), S(5), S(5));
    }
    if (enabled) addRegion(id, r, tip, CursorKind::Hand);
}

void App::drawIconButton(Graphics& g, const std::string& id, RectF r, Icon ic, bool enabled, const std::wstring& tip) {
    bool hover = enabled && hoverId_ == id;
    if (hover) fillRoundRect(g, r, S(9), 0xF1F4FA);
    uint32_t fg = enabled ? 0x414C5C : 0xBDC5D0;
    float isz = S(19);
    drawIcon(g, ic, RectF(r.X + (r.Width - isz) / 2, r.Y + (r.Height - isz) / 2, isz, isz), fg, S(1.7));
    if (enabled) addRegion(id, r, tip, CursorKind::Hand);
}

void App::drawHeader(Graphics& g) {
    drawBrand(g, false);
}

void App::drawPageNav(Graphics& g) {
    float pw = S(248), ph = S(40);
    float px = ((float)W_ - pw) / 2.0f;
    float py = (headerH() - ph) / 2.0f;
    RectF r(px, py, pw, ph);
    fillRoundRect(g, r, ph / 2, 0xF4F6FA);
    strokeRoundRect(g, r, ph / 2, 0xEDF0F6, S(1));

    auto hoverBox = [&](float x, float w, const std::string& id) {
        if (hoverId_ == id) fillRoundRect(g, RectF(x, py + S(4), w, ph - S(8)), S(12), 0xE8EDF7);
    };
    bool hasPrev = current_ > 0;
    bool hasNext = current_ + 1 < pageCount();
    hoverBox(px + S(4), S(32), "page:prev");
    drawIcon(g, Icon::ChevronLeft, RectF(px + S(11), py + S(11), S(18), S(18)),
             hasPrev ? 0x596375 : 0xC1C8D3, S(1.8));
    if (hasPrev) addRegion("page:prev", RectF(px + S(4), py + S(4), S(32), ph - S(8)),
    L"上一页  (PageUp)", CursorKind::Hand);
    hoverBox(px + pw - S(72), S(32), "page:next");
    drawIcon(g, Icon::ChevronRight, RectF(px + pw - S(65), py + S(11), S(18), S(18)),
             hasNext ? 0x596375 : 0xC1C8D3, S(1.8));
    if (hasNext) addRegion("page:next", RectF(px + pw - S(72), py + S(4), S(32), ph - S(8)),
    L"下一页  (PageDown)", CursorKind::Hand);

    bool labelHover = hoverId_ == "page:label";
    if (labelHover) fillRoundRect(g, RectF(px + S(38), py + S(4), pw - S(110), ph - S(8)), S(12), 0xE8EDF7);
    std::wstring pageText = std::to_wstring(current_ + 1) + L" / " + std::to_wstring(pageCount());
    drawIcon(g, Icon::Page, RectF(px + S(58), py + S(12), S(16), S(16)), 0x7C8698, S(1.4));
    drawTextAligned(g, pageText, RectF(px + S(80), py, pw - S(160), ph), fNav_, 0x4B5565,
                    StringAlignmentCenter, StringAlignmentCenter, false);
    addRegion("page:label", RectF(px + S(38), py + S(4), pw - S(110), ph - S(8)),
              L"页面管理 · 点击查看全部页面", CursorKind::Hand);

    hoverBox(px + pw - S(36), S(32), "page:add");
    drawIcon(g, Icon::Plus, RectF(px + pw - S(29), py + S(12), S(16), S(16)), 0x4B5565, S(1.7));
    addRegion("page:add", RectF(px + pw - S(36), py + S(4), S(32), ph - S(8)),
              L"新建页面", CursorKind::Hand);
}

// ---------------------------------------------------------------------------
// page
// ---------------------------------------------------------------------------

void App::drawPage(Graphics& g) {
    GraphicsState boardState = g.Save();
    g.SetClip(RectF(0, headerH(), (float)W_, (float)H_ - headerH()), CombineModeReplace);
    RectF pr = pageScreenRect();
    if (pr.Width < 10 || pr.Height < 10) {
        g.Restore(boardState);
        return;
    }
    drawShadow(g, pr, S(10), S(15), S(6));
    fillRoundRect(g, pr, S(7), page().paper);
    GraphicsPath pagePath;
    roundPath(pagePath, pr, S(7));
    GraphicsState pageState = g.Save();
    g.SetClip(&pagePath, CombineModeReplace);
    drawPattern(g, page(), pageXf(), pr);

    bool hideEditing = hEdit_ != nullptr;
    for (int i = 0; i < (int)page().items.size(); ++i) {
        if (i == hideItemForDrag_) continue;
        bool hide = hideEditing && i == editIndex_;
        drawItem(g, page().items[i], pageXf(), hide, false);
    }
    if (page().items.empty() && !hEdit_ && undo_.empty()) {
        uint32_t hintColor = 0xAAB8B1;
        drawTextAligned(g, L"在下方工具栏选择画笔，开始创作", RectF(pr.X, pr.GetBottom() - S(102), pr.Width, S(26)),
                        fUI_, hintColor, StringAlignmentCenter, StringAlignmentCenter, false);
        drawTextAligned(g, L"拖动下方工具栏可移动   ·   Ctrl + 滚轮缩放   ·   0 适应页面",
                        RectF(pr.X, pr.GetBottom() - S(76), pr.Width, S(22)), fMini_, 0xB8C5BE,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    g.Restore(pageState);
    strokeRoundRect(g, pr, S(7), 0xDCE2EA, S(1));
    g.Restore(boardState);

    if (hEdit_) {
        // editor child paints itself; nothing extra here
    }
}

void App::drawPageContent(Graphics& g, const Page& p, Xf xf, const RectF& clip, bool hideEditing) {
    RectF pageRect = xf.rect(0, 0, PAGE_W, PAGE_H);
    GraphicsState st = g.Save();
    g.SetClip(clip, CombineModeIntersect);
    g.SetClip(pageRect, CombineModeIntersect);
    SolidBrush paper(RGBA(p.paper));
    g.FillRectangle(&paper, pageRect);
    drawPattern(g, p, xf, clip);
    for (int i = 0; i < (int)p.items.size(); ++i) {
        if (i == hideItemForDrag_) continue;
        bool hide = hideEditing && hEdit_ && i == editIndex_;
        drawItem(g, p.items[i], xf, hide, true);
    }
    g.Restore(st);
}

void App::drawPattern(Graphics& g, const Page& p, Xf xf, const RectF& clip) {
    if (p.pattern == L"blank") return;
    RectF page = xf.rect(0, 0, PAGE_W, PAGE_H);
    GraphicsState st = g.Save();
    g.SetClip(clip, CombineModeIntersect);
    g.SetClip(page, CombineModeIntersect);
    if (p.pattern == L"grid") {
        Pen pen(RGBA(0xE6EBF2, 220), std::max(0.7f, xf.s * 0.9f));
        for (float x = 0; x <= PAGE_W; x += 40) {
            float sx = xf.tx(x);
            g.DrawLine(&pen, sx, page.Y, sx, page.GetBottom());
        }
        for (float y = 0; y <= PAGE_H; y += 40) {
            float sy = xf.ty(y);
            g.DrawLine(&pen, page.X, sy, page.GetRight(), sy);
        }
    } else if (p.pattern == L"ruled") {
        Pen pen(RGBA(0xE3E9F2, 235), std::max(0.7f, xf.s * 0.95f));
        for (float y = 48; y <= PAGE_H; y += 48) {
            float sy = xf.ty(y);
            g.DrawLine(&pen, page.X, sy, page.GetRight(), sy);
        }
        Pen margin(RGBA(0xF0C9CC, 150), std::max(0.7f, xf.s));
        g.DrawLine(&margin, xf.tx(110), page.Y, xf.tx(110), page.GetBottom());
    } else if (p.pattern == L"dots") {
        float step = 40;
        float rad = std::max(0.7f, xf.s * 1.35f);
        SolidBrush b(RGBA(0xC9D2E0, 220));
        for (float y = 20; y <= PAGE_H; y += step) {
            for (float x = 20; x <= PAGE_W; x += step) {
                float sx = xf.tx(x), sy = xf.ty(y);
                g.FillEllipse(&b, sx - rad, sy - rad, rad * 2, rad * 2);
            }
        }
    }
    g.Restore(st);
}

void App::drawFunctionAxes(Graphics& g, const Item& it, Xf xf) {
    if (!it.functionGraph || !it.showAxes) return;
    float left = xf.tx(it.gLeft), right = xf.tx(it.gRight);
    float top = xf.ty(it.gTop), bottom = xf.ty(it.gBottom);
    if (right - left < S(20) || bottom - top < S(20)) return;
    RectF box(left, top, right - left, bottom - top);
    GraphicsState st = g.Save();
    g.SetClip(box, CombineModeIntersect);

    float ux = it.unitX * xf.s;
    float uy = it.unitY * xf.s;
    float ox = xf.tx(it.originX), oy = xf.ty(it.originY);
    if (ux > S(7)) {
        Pen grid(RGBA(0x405A74, 26), std::max(0.6f, S(0.8f)));
        int first = (int)std::ceil((left - ox) / ux);
        int last = (int)std::floor((right - ox) / ux);
        if (last - first <= 80) {
            for (int k = first; k <= last; ++k) {
                if (k == 0) continue;
                float x = ox + k * ux;
                g.DrawLine(&grid, x, top, x, bottom);
            }
        }
    }
    if (uy > S(7)) {
        Pen grid(RGBA(0x405A74, 26), std::max(0.6f, S(0.8f)));
        int first = (int)std::ceil((top - oy) / uy);
        int last = (int)std::floor((bottom - oy) / uy);
        if (last - first <= 80) {
            for (int k = first; k <= last; ++k) {
                if (k == 0) continue;
                float y = oy + k * uy;
                g.DrawLine(&grid, left, y, right, y);
            }
        }
    }

    Pen axis(RGBA(0x8A9BAC, 225), std::max(1.0f, S(1.2f)));
    axis.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    bool hasX = oy >= top - 2 && oy <= bottom + 2;
    bool hasY = ox >= left - 2 && ox <= right + 2;
    if (hasX) g.DrawLine(&axis, left, oy, right, oy);
    if (hasY) g.DrawLine(&axis, ox, top, ox, bottom);

    Pen arrow(RGBA(0x8A9BAC, 225), std::max(1.0f, S(1.2f)));
    if (hasX) {
        g.DrawLine(&arrow, right - S(9), oy - S(4), right, oy);
        g.DrawLine(&arrow, right - S(9), oy + S(4), right, oy);
    }
    if (hasY) {
        g.DrawLine(&arrow, ox - S(4), top + S(9), ox, top);
        g.DrawLine(&arrow, ox + S(4), top + S(9), ox, top);
    }
    float xLabelY = oy + S(3);
    if (oy + S(22) > bottom - S(2) || oy + S(22) > (float)H_ - S(6))
        xLabelY = oy - S(20);
    if (hasX && hasY) {
        drawTextAligned(g, L"0", RectF(ox - S(20), xLabelY - S(1), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    if (hasX) {
        drawTextAligned(g, L"x", RectF(right - S(18), xLabelY - S(1), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
    if (hasY) {
        drawTextAligned(g, L"y", RectF(ox + S(4), top + S(2), S(16), S(16)), fMini_, 0x8A9BAC,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }

    if (hasX && ux > S(2)) {
        int step = 1;
        while (ux * step < S(14)) step *= 2;
        int first = (int)std::ceil((left - ox) / ux);
        int last = (int)std::floor((right - ox) / ux);
        for (int k = first; k <= last; ++k) {
            if (k == 0 || k % step != 0) continue;
            float x = ox + k * ux;
            drawTextAligned(g, std::to_wstring(k), RectF(x - S(18), xLabelY, S(36), S(16)), fTiny_,
                            0x7A8D9E, StringAlignmentCenter, StringAlignmentCenter, false);
        }
    }
    if (hasY && uy > S(2)) {
        int step = 1;
        while (uy * step < S(14)) step *= 2;
        int first = (int)std::ceil((oy - bottom) / uy);
        int last = (int)std::floor((oy - top) / uy);
        for (int k = first; k <= last; ++k) {
            if (k == 0 || k % step != 0) continue;
            float y = oy - k * uy;
            drawTextAligned(g, std::to_wstring(k), RectF(ox - S(34), y - S(8), S(28), S(16)), fTiny_,
                            0x7A8D9E, StringAlignmentFar, StringAlignmentCenter, true);
        }
    }
    g.Restore(st);
}

void App::drawItem(Graphics& g, const Item& it, Xf xf, bool hideThis, bool exportMode) {
    if (hideThis) return;
    float s = xf.s;
    if (it.kind == ItemKind::Stroke) {
        if (it.points.empty()) return;
        BYTE alpha = (it.strokeTool == L"highlighter") ? 82 : 255;
        Color col = RGBA(it.color, alpha);
        if (it.functionGraph && it.showAxes) drawFunctionAxes(g, it, xf);
        float w = std::max(0.7f, it.width * s);
        if (it.points.size() == 1) {
            SolidBrush b(col);
            float r = std::max(0.7f, w / 2);
            Pt p = xf.to(it.points[0]);
            g.FillEllipse(&b, p.x - r, p.y - r, r * 2, r * 2);
            return;
        }
        Pen pen(col, w);
        pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
        pen.SetLineJoin(LineJoinRound);
        if (it.points.size() == 2) {
            Pt a = xf.to(it.points[0]);
            Pt b = xf.to(it.points[1]);
            g.DrawLine(&pen, a.x, a.y, b.x, b.y);
        } else {
            std::vector<PointF> pts;
            pts.reserve(it.points.size());
            for (auto& p : it.points) {
                Pt q = xf.to(p);
                pts.push_back(PointF(q.x, q.y));
            }
            g.DrawLines(&pen, pts.data(), (INT)pts.size());
        }
    } else if (it.kind == ItemKind::Shape) {
        float x1 = xf.tx(std::min(it.x1, it.x2));
        float y1 = xf.ty(std::min(it.y1, it.y2));
        float x2 = xf.tx(std::max(it.x1, it.x2));
        float y2 = xf.ty(std::max(it.y1, it.y2));
        float w = std::max(0.8f, it.width * s);
        if (it.shape == ShapeKind::Line || it.shape == ShapeKind::Arrow) {
            PointF a(xf.tx(it.x1), xf.ty(it.y1));
            PointF b(xf.tx(it.x2), xf.ty(it.y2));
            Pen pen(RGBA(it.color), w);
            pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            g.DrawLine(&pen, a, b);
            if (it.shape == ShapeKind::Arrow) addArrow(g, a, b, w, it.color);
        } else {
            RectF r(x1, y1, x2 - x1, y2 - y1);
            if (it.filled) {
                SolidBrush b(RGBA(it.color, 42));
                if (it.shape == ShapeKind::Ellipse) g.FillEllipse(&b, r);
                else g.FillRectangle(&b, r);
            }
            Pen pen(RGBA(it.color), w);
            pen.SetLineJoin(LineJoinRound);
            if (it.shape == ShapeKind::Ellipse) g.DrawEllipse(&pen, r);
            else g.DrawRectangle(&pen, r);
            if (it.isPoint) {
                SolidBrush dot(RGBA(it.color));
                g.FillEllipse(&dot, r);
                if (!it.pointText.empty()) {
                    drawTextAligned(g, wide(it.pointText),
                                    RectF(r.GetRight() + S(6), r.Y - S(9), S(120), S(20)), fMini_,
                                    it.color, StringAlignmentNear, StringAlignmentCenter, false);
                }
            }
        }
    } else if (it.kind == ItemKind::Text) {
        if (it.text.empty()) return;
        std::wstring face = L"Microsoft YaHei UI";
        Font font(face.c_str(), std::max(1.0f, it.fontSize * s), FontStyleRegular, UnitPixel);
        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentNear);
        fmt.SetLineAlignment(StringAlignmentNear);
        fmt.SetTrimming(StringTrimmingNone);
        fmt.SetFormatFlags(0);
        RectF layout(xf.tx(it.x), xf.ty(it.y), std::max(20.0f, it.textWidth * s), std::max(10.0f, PAGE_H * s));
        SolidBrush brush(RGBA(it.color));
        g.DrawString(it.text.c_str(), -1, &font, layout, &fmt, &brush);
    } else if (it.kind == ItemKind::Image) {
        Bitmap* bmp = getImage(it);
        RectF r = xf.rect(it.x, it.y, it.w, it.h);
        if (bmp) {
            g.DrawImage(bmp, r);
        } else {
            fillRoundRect(g, r, S(4), 0xF2F4F8);
            strokeRoundRect(g, r, S(4), COL_LINE, S(1));
            drawIcon(g, Icon::Image, RectF(r.X + r.Width / 2 - S(12), r.Y + r.Height / 2 - S(12), S(24), S(24)),
                     0xAAB4C2, S(1.6));
        }
    }
}

void App::drawSelection(Graphics& g) {
    if (sel_ < 0 || sel_ >= (int)page().items.size()) return;
    RectF r = selectionScreenRect();
    r.Inflate(S(3), S(3));
    Pen pen(RGBA(COL_ACCENT, 210), S(1.4f));
    pen.SetDashStyle(DashStyleDash);
    g.DrawRectangle(&pen, r);
    float hs = S(8);
    PointF pts[4] = {{r.X, r.Y}, {r.GetRight(), r.Y}, {r.X, r.GetBottom()}, {r.GetRight(), r.GetBottom()}};
    for (auto& p : pts) {
        SolidBrush b(RGBA(COL_WHITE));
        g.FillRectangle(&b, p.X - hs / 2, p.Y - hs / 2, hs, hs);
        Pen bp(RGBA(COL_ACCENT), S(1.3f));
        g.DrawRectangle(&bp, p.X - hs / 2, p.Y - hs / 2, hs, hs);
    }
    Xf xf = pageXf();
    for (int idx : pointSel_) {
        if (idx < 0 || idx >= (int)page().items.size()) continue;
        RectF b = page().items[idx].bounds();
        RectF sr = xf.rect(b.X, b.Y, b.Width, b.Height);
        sr.Inflate(S(5), S(5));
        Pen ring(RGBA(COL_ACCENT), S(2.0f));
        g.DrawEllipse(&ring, sr);
    }
}

static void drawFastStroke(Graphics& g, const Item& it, Xf xf) {
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
    }
    if (drag_ == DragMode::Shape) {
        drawItem(g, previewShape_, pageXf(), false, false);
    }
    if (tool_ == Tool::Eraser && mouseInside_ && !overlay_ && !hEdit_) {
        float r = eraserSize_ * viewScale() / 2;
        float cx = (float)mouse_.x, cy = (float)mouse_.y;
        SolidBrush fill(RGBA(COL_ACCENT, 16));
        g.FillEllipse(&fill, cx - r, cy - r, r * 2, r * 2);
        Pen pen(RGBA(COL_ACCENT, 155), S(1.3f));
        g.DrawEllipse(&pen, cx - r, cy - r, r * 2, r * 2);
    }
    if (!laser_.empty()) {
        ULONGLONG now = GetTickCount64();
        for (size_t i = 1; i < laser_.size(); ++i) {
            float age = (float)(now - laser_[i].t);
            if (age > 850) continue;
            float a = 1.0f - age / 850.0f;
            Pen glow(RGBA(laserColor_, (BYTE)(a * 60)), S(10));
            glow.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            Pen core(RGBA(laserColor_, (BYTE)(a * 235)), S(3.2f));
            core.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            g.DrawLine(&glow, laser_[i - 1].p.x, laser_[i - 1].p.y, laser_[i].p.x, laser_[i].p.y);
            g.DrawLine(&core, laser_[i - 1].p.x, laser_[i - 1].p.y, laser_[i].p.x, laser_[i].p.y);
        }
        if (!laser_.empty() && laserDown_) {
            float a = 1.0f;
            SolidBrush b(RGBA(laserColor_, 230));
            g.FillEllipse(&b, laser_.back().p.x - S(3), laser_.back().p.y - S(3), S(6), S(6));
        }
    }
}

// ---------------------------------------------------------------------------
// tool rail
// ---------------------------------------------------------------------------

void App::drawRail(Graphics& g) {
    const RailDef defs[] = {
        {"tool:select", Icon::Cursor, L"选择", L"选择 / 移动  (V)"},
        {"tool:pen", Icon::Pen, L"画笔", L"画笔  (P)"},
    {"tool:highlighter", Icon::Highlighter, L"荧光笔", L"荧光笔  (H)"},
    {"tool:eraser", Icon::Eraser, L"橡皮", L"橡皮擦 · 整笔擦除  (E)"},
        {"tool:shape", Icon::Shape, L"形状", L"形状 / 线条  (R)"},
        {"tool:text", Icon::Text, L"文字", L"文字  (T)"},
    {"tool:laser", Icon::Laser, L"激光", L"激光笔  (L)"},
        {"", Icon::Page, L"", L""},
        {"rail:image", Icon::Image, L"图片", L"插入图片"},
        {"rail:bg", Icon::Background, L"背景", L"页面背景  (B)"},
        {"", Icon::Page, L"", L""},
        {"tool:hand", Icon::Hand, L"移动", L"移动画布  (空格 + 拖动)"},
    };
    float pad = S(9);
    float btnH = S(47);
    float gap = S(2);
    float divH = S(11);
    float railW = S(64);
    float railH = pad * 2;
    for (auto& d : defs) railH += (d.id[0] == '\0' ? divH : btnH + gap);
    float x = S(18);
    float y = headerH() + S(18);
    RectF rr(x, y, railW, railH);
    drawShadow(g, rr, S(16), S(8), S(3));
    fillRoundRect(g, rr, S(16), COL_WHITE);
    strokeRoundRect(g, rr, S(16), 0xE8ECF2, S(1));

    float cy = y + pad;
    for (auto& d : defs) {
        if (d.id[0] == '\0') {
            Pen line(RGBA(0xE7EBF2), S(1));
            g.DrawLine(&line, x + S(16), cy + divH / 2, x + railW - S(16), cy + divH / 2);
            cy += divH;
            continue;
        }
        RectF br(x + S(7), cy, railW - S(14), btnH);
        bool selected = false;
        if (std::string(d.id) == "tool:select") selected = tool_ == Tool::Select;
        else if (std::string(d.id) == "tool:pen") selected = tool_ == Tool::Pen;
        else if (std::string(d.id) == "tool:highlighter") selected = tool_ == Tool::Highlighter;
        else if (std::string(d.id) == "tool:eraser") selected = tool_ == Tool::Eraser;
        else if (std::string(d.id) == "tool:shape") selected = tool_ == Tool::Shape;
        else if (std::string(d.id) == "tool:text") selected = tool_ == Tool::Text;
        else if (std::string(d.id) == "tool:laser") selected = tool_ == Tool::Laser;
        else if (std::string(d.id) == "tool:hand") selected = tool_ == Tool::Hand;
        else if (std::string(d.id) == "rail:bg") selected = bgPanel_;
        if (selected) fillRoundRect(g, br, S(12), COL_ACCENT_SOFT);
        else if (hoverId_ == d.id) fillRoundRect(g, br, S(12), 0xF4F6FA);
        uint32_t fg = selected ? COL_ACCENT : 0x5A6474;
        float isz = S(22);
        drawIcon(g, d.icon, RectF(x + railW / 2 - isz / 2, cy + S(5), isz, isz), fg, S(1.75f));
        drawTextAligned(g, d.label, RectF(x + S(1), cy + S(28), railW - S(2), S(15)), fMini_,
                        selected ? COL_ACCENT : 0x6B7585, StringAlignmentCenter, StringAlignmentCenter, false);
        addRegion(d.id, br, d.tip, CursorKind::Hand);
        cy += btnH + gap;
    }
}

// ---------------------------------------------------------------------------
// option bar
// ---------------------------------------------------------------------------

void App::drawOptLabel(Graphics& g, float x, float y, const wchar_t* text) {
    drawTextAligned(g, text, RectF(x, y - S(9), S(34), S(18)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
}

void App::drawSwatch(Graphics& g, const std::string& id, float cx, float cy, uint32_t color,
                     float radius, bool selected, const std::wstring& tip) {
    if (selected) {
        Pen ring(RGBA(COL_ACCENT), S(1.8f));
        g.DrawEllipse(&ring, cx - radius - S(3), cy - radius - S(3),
                      (radius + S(3)) * 2, (radius + S(3)) * 2);
    } else if (hoverId_ == id) {
        Pen ring(RGBA(0xC9D2E2), S(1.2f));
        g.DrawEllipse(&ring, cx - radius - S(3), cy - radius - S(3),
                      (radius + S(3)) * 2, (radius + S(3)) * 2);
    }
    SolidBrush b(RGBA(color));
    g.FillEllipse(&b, cx - radius, cy - radius, radius * 2, radius * 2);
    int lum = (int)(0.299 * ((color >> 16) & 255) + 0.587 * ((color >> 8) & 255) + 0.114 * (color & 255));
    if (lum > 232) {
        Pen edge(RGBA(0xC3D0C8), S(1));
        g.DrawEllipse(&edge, cx - radius, cy - radius, radius * 2, radius * 2);
    }
    addRegion(id, RectF(cx - S(15), cy - S(19), S(30), S(38)), tip, CursorKind::Hand);
}

void App::drawWidthDot(Graphics& g, const std::string& id, float cx, float cy, float width,
                       bool selected, const std::wstring& tip) {
    float r = std::min(S(10), std::max(S(2), width * S(0.62f)));
    if (selected) {
        Pen ring(RGBA(COL_ACCENT), S(1.8f));
        g.DrawEllipse(&ring, cx - r - S(4), cy - r - S(4), (r + S(4)) * 2, (r + S(4)) * 2);
    } else if (hoverId_ == id) {
        Pen ring(RGBA(0xC9D2E2), S(1.2f));
        g.DrawEllipse(&ring, cx - r - S(4), cy - r - S(4), (r + S(4)) * 2, (r + S(4)) * 2);
    }
    SolidBrush b(RGBA(0x3B4555));
    g.FillEllipse(&b, cx - r, cy - r, r * 2, r * 2);
    addRegion(id, RectF(cx - S(15), cy - S(19), S(30), S(38)), tip, CursorKind::Hand);
}

void App::drawMiniButton(Graphics& g, const std::string& id, RectF r, Icon icon, const wchar_t* label,
                         bool enabled, const std::wstring& tip) {
    bool hover = enabled && hoverId_ == id;
    if (hover) fillRoundRect(g, r, S(9), 0xF2F5FA);
    uint32_t fg = enabled ? 0x4B5565 : 0xB9C1CC;
    drawIcon(g, icon, RectF(r.X + S(9), r.Y + (r.Height - S(15)) / 2, S(15), S(15)), fg, S(1.5f));
    drawTextAligned(g, label, RectF(r.X + S(28), r.Y, r.Width - S(30), r.Height), fMini_, fg,
                    StringAlignmentNear, StringAlignmentCenter, false);
    if (enabled) addRegion(id, r, tip, CursorKind::Hand);
}

void App::drawClearButton(Graphics& g, RectF r, const std::wstring& tip) {
    bool hover = hoverId_ == "clearpage";
    fillRoundRect(g, r, S(9), hover ? 0xFFE8E8 : 0xFFF2F2);
    drawIcon(g, Icon::Trash, RectF(r.X + S(10), r.Y + (r.Height - S(15)) / 2, S(15), S(15)),
             COL_DANGER, S(1.5f));
    drawTextAligned(g, L"清空本页", RectF(r.X + S(30), r.Y, r.Width - S(36), r.Height), fMini_,
                    COL_DANGER, StringAlignmentNear, StringAlignmentCenter, false);
    addRegion("clearpage", r, tip, CursorKind::Hand);
}

void App::drawOptionBar(Graphics& g) {
    bool hasSel = sel_ >= 0 && sel_ < (int)page().items.size();
    float logicalW = 516;
    if (tool_ == Tool::Highlighter) logicalW = 456;
    else if (tool_ == Tool::Eraser) logicalW = 342;
    else if (tool_ == Tool::Shape) logicalW = 604;
    else if (tool_ == Tool::Text) logicalW = 574;
    else if (tool_ == Tool::Select) logicalW = hasSel ? 356 : 468;
    else if (tool_ == Tool::Laser) logicalW = 378;
    else if (tool_ == Tool::Hand) logicalW = 470;
    float bw = S(logicalW), bh = S(62);
    float bx = ((float)W_ - bw) / 2.0f;
    float by = (float)H_ - S(18) - bh;
    RectF bar(bx, by, bw, bh);
    drawShadow(g, bar, S(14), S(10), S(3));
    fillRoundRect(g, bar, S(14), COL_WHITE);
    strokeRoundRect(g, bar, S(14), 0xE7EBF1, S(1));

    float cy = by + bh / 2;
    float x = bx + S(16);
    uint32_t curColor = penColor_;
    if (tool_ == Tool::Highlighter) curColor = highlighterColor_;
    else if (tool_ == Tool::Shape) curColor = shapeColor_;
    else if (tool_ == Tool::Text) curColor = textColor_;
    else if (tool_ == Tool::Laser) curColor = laserColor_;

    auto divider = [&]() {
        Pen line(RGBA(0xE9EDF3), S(1));
        g.DrawLine(&line, x + S(7), cy - S(16), x + S(7), cy + S(16));
        x += S(15);
    };

    if (tool_ == Tool::Pen || tool_ == Tool::Highlighter) {
        drawOptLabel(g, x, cy, L"颜色");
        x += S(30);
        const uint32_t palette[] = {0x202633, 0xE5484D, 0xF59E0B, 0x18A56F, 0x2F6BFF, 0x8B5CF6, 0xEC4899};
        for (auto c : palette) {
            drawSwatch(g, "color:" + hexColor(c), x + S(15), cy, c, S(9),
                       curColor == c, L"颜色 #" + wide(hexColor(c)));
            x += S(30);
        }
        // custom color button
        float ccx = x + S(15);
        RectF cr(ccx - S(9), cy - S(9), S(18), S(18));
        SolidBrush c1(RGBA(0xE5484D)), c2(RGBA(0xF59E0B)), c3(RGBA(0x18A56F)), c4(RGBA(0x2F6BFF));
        g.FillPie(&c1, cr, -45, 90);
        g.FillPie(&c2, cr, 45, 90);
        g.FillPie(&c3, cr, 135, 90);
        g.FillPie(&c4, cr, 225, 90);
        Pen plus(RGBA(COL_WHITE), S(1.9f));
        plus.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
        g.DrawLine(&plus, ccx - S(3.5f), cy, ccx + S(3.5f), cy);
        g.DrawLine(&plus, ccx, cy - S(3.5f), ccx, cy + S(3.5f));
        if (hoverId_ == "customcolor") {
            Pen ring(RGBA(COL_ACCENT), S(1.2f));
            g.DrawEllipse(&ring, ccx - S(12), cy - S(12), S(24), S(24));
        }
    addRegion("customcolor", RectF(ccx - S(15), cy - S(19), S(30), S(38)), L"自定义颜色", CursorKind::Hand);
        x += S(30);
        divider();
        drawOptLabel(g, x, cy, L"粗细");
        x += S(30);
        if (tool_ == Tool::Pen) {
            float widths[] = {2, 4, 7, 11, 16};
            for (float wv : widths) {
                drawWidthDot(g, "width:" + strOf((int)wv), x + S(15), cy, wv, std::fabs(penWidth_ - wv) < 0.6f,
                             L"画笔粗细 " + std::to_wstring((int)wv));
                x += S(30);
            }
        } else {
            float widths[] = {12, 20, 30, 42};
            for (float wv : widths) {
                drawWidthDot(g, "width:" + strOf((int)wv), x + S(15), cy, wv * 0.5f,
                             std::fabs(highlighterWidth_ - wv) < 0.6f,
    L"荧光笔宽度 " + std::to_wstring((int)wv));
                x += S(30);
            }
        }
    } else if (tool_ == Tool::Eraser) {
        drawOptLabel(g, x, cy, L"大小");
        x += S(30);
        float sizes[] = {18, 30, 44, 60};
        for (float sv : sizes) {
            drawWidthDot(g, "width:" + strOf((int)sv), x + S(15), cy, sv * 0.32f,
                         std::fabs(eraserSize_ - sv) < 0.6f, L"橡皮大小 " + std::to_wstring((int)sv));
            x += S(32);
        }
        divider();
    drawClearButton(g, RectF(x, cy - S(18), S(132), S(36)), L"清空当前页（可撤销）");
    } else if (tool_ == Tool::Shape) {
        for (int i = 0; i < 4; ++i) {
            RectF br(x, cy - S(18), S(36), S(36));
            bool sel = (int)shapeKind_ == i;
            if (hoverId_ == "shape:" + strOf(i)) fillRoundRect(g, br, S(9), 0xF2F5FA);
            if (sel) fillRoundRect(g, br, S(9), COL_ACCENT_SOFT);
            uint32_t fg = sel ? COL_ACCENT : 0x4F5A6A;
            float cx = x + S(18), cy0 = cy;
            if (i == 0) {
                Pen p(RGBA(fg), S(1.8f));
                p.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
                g.DrawLine(&p, cx - S(9), cy0 + S(7), cx + S(9), cy0 - S(7));
            } else if (i == 1) {
                Pen p(RGBA(fg), S(1.8f));
                p.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
                g.DrawLine(&p, cx - S(9), cy0 + S(7), cx + S(9), cy0 - S(7));
                addArrow(g, PointF(cx - S(9), cy0 + S(7)), PointF(cx + S(9), cy0 - S(7)), S(1.8f), fg);
            } else if (i == 2) {
                Pen p(RGBA(fg), S(1.8f));
                g.DrawRectangle(&p, cx - S(9), cy0 - S(8), S(18), S(16));
            } else {
                Pen p(RGBA(fg), S(1.8f));
                g.DrawEllipse(&p, cx - S(9), cy0 - S(8), S(18), S(16));
            }
    addRegion("shape:" + strOf(i), br, i == 0 ? L"直线" : i == 1 ? L"箭头" : i == 2 ? L"矩形" : L"椭圆",
                      CursorKind::Hand);
            x += S(36);
        }
        x += S(6);
        RectF fillRect(x, cy - S(17), S(54), S(34));
        if (hoverId_ == "fill") fillRoundRect(g, fillRect, S(9), 0xF2F5FA);
        if (shapeFilled_) fillRoundRect(g, fillRect, S(9), COL_ACCENT_SOFT);
    drawTextAligned(g, L"填充", RectF(x + S(9), cy - S(17), S(28), S(34)), fMini_,
                        shapeFilled_ ? COL_ACCENT : 0x5A6474, StringAlignmentNear,
                        StringAlignmentCenter, false);
        if (shapeFilled_) drawCheckmark(g, RectF(x + S(36), cy - S(7), S(14), S(14)), COL_ACCENT, S(1.8f));
    addRegion("fill", fillRect, L"形状是否填色", CursorKind::Hand);
        x += S(54);
        divider();
        const uint32_t palette[] = {0x202633, 0xE5484D, 0xF59E0B, 0x18A56F, 0x2F6BFF, 0x8B5CF6};
        for (auto c : palette) {
            drawSwatch(g, "color:" + hexColor(c), x + S(15), cy, c, S(9),
                       shapeColor_ == c, L"颜色 #" + wide(hexColor(c)));
            x += S(30);
        }
        divider();
    drawOptLabel(g, x, cy, L"线宽");
        x += S(30);
        float widths[] = {2, 4, 7};
        for (float wv : widths) {
            drawWidthDot(g, "width:" + strOf((int)wv), x + S(15), cy, wv, std::fabs(shapeWidth_ - wv) < 0.6f,
    L"线宽 " + std::to_wstring((int)wv));
            x += S(30);
        }
    } else if (tool_ == Tool::Text) {
        drawOptLabel(g, x, cy, L"颜色");
        x += S(30);
        const uint32_t palette[] = {0x202633, 0xE5484D, 0xF59E0B, 0x18A56F, 0x2F6BFF, 0x8B5CF6};
        for (auto c : palette) {
            drawSwatch(g, "color:" + hexColor(c), x + S(15), cy, c, S(9),
                       textColor_ == c, L"文字颜色 #" + wide(hexColor(c)));
            x += S(30);
        }
        divider();
        drawOptLabel(g, x, cy, L"字号");
        x += S(34);
        float sizes[] = {28, 40, 56, 72};
    const wchar_t* labels[] = {L"小", L"中", L"大", L"特大"};
        for (int i = 0; i < 4; ++i) {
            RectF cr(x, cy - S(16), S(42), S(32));
            bool sel = std::fabs(textSize_ - sizes[i]) < 0.6f;
            if (sel) fillRoundRect(g, cr, S(8), COL_ACCENT_SOFT);
            else if (hoverId_ == "textsize:" + strOf((int)sizes[i])) fillRoundRect(g, cr, S(8), 0xF2F5FA);
            drawTextAligned(g, labels[i], cr, fMini_, sel ? COL_ACCENT : 0x5A6474,
                            StringAlignmentCenter, StringAlignmentCenter, false);
            addRegion("textsize:" + strOf((int)sizes[i]), cr,
                      L"字号 " + std::to_wstring((int)sizes[i]), CursorKind::Hand);
            x += S(42);
        }
        x += S(8);
        divider();
        drawTextAligned(g, L"点击页面输入文字", RectF(x, cy - S(18), S(126), S(36)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    } else if (tool_ == Tool::Select) {
        if (hasSel) {
            drawMiniButton(g, "sel:copy", RectF(x, cy - S(18), S(74), S(36)), Icon::Copy, L"复制", true,
                           L"复制选中对象  (Ctrl+D)");
            x += S(74) + S(8);
            drawMiniButton(g, "sel:delete", RectF(x, cy - S(18), S(72), S(36)), Icon::Trash, L"删除", true,
                           L"删除选中对象  (Delete)");
            x += S(72) + S(8);
    drawMiniButton(g, "sel:front", RectF(x, cy - S(18), S(72), S(36)), Icon::Front, L"置顶", true,
    L"移到最上层");
            x += S(72) + S(8);
    drawMiniButton(g, "sel:back", RectF(x, cy - S(18), S(72), S(36)), Icon::Back, L"置底", true,
    L"移到最下层");
        } else {
    drawTextAligned(g, L"点选对象后拖动  ·  双击文字可编辑  ·  Delete 删除",
                            RectF(x, cy - S(18), S(436), S(36)), fUI_, COL_MUTED,
                            StringAlignmentCenter, StringAlignmentCenter, false);
        }
    } else if (tool_ == Tool::Laser) {
        drawOptLabel(g, x, cy, L"颜色");
        x += S(30);
        const uint32_t palette[] = {0xE5484D, 0xF59E0B, 0x18A56F, 0x2F6BFF};
        for (auto c : palette) {
            drawSwatch(g, "color:" + hexColor(c), x + S(15), cy, c, S(9),
    laserColor_ == c, L"激光颜色");
            x += S(30);
        }
        divider();
    drawTextAligned(g, L"按住拖动，松手渐隐", RectF(x, cy - S(18), S(198), S(36)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    } else if (tool_ == Tool::Hand) {
        drawTextAligned(g, L"拖动画布  ·  Ctrl + 滚轮缩放  ·  0 适应页面  ·  空格临时移动",
                        RectF(x, cy - S(18), S(438), S(36)), fUI_, COL_MUTED,
                        StringAlignmentCenter, StringAlignmentCenter, false);
    }
}

// ---------------------------------------------------------------------------
// bottom chrome
// ---------------------------------------------------------------------------

void App::drawZoomBar(Graphics& g) {
    float w = S(178), h = S(38);
    float x = (float)W_ - S(18) - w;
    float y = (float)H_ - S(18) - h;
    RectF r(x, y, w, h);
    drawShadow(g, r, S(12), S(7), S(2));
    fillRoundRect(g, r, S(12), COL_WHITE);
    strokeRoundRect(g, r, S(12), 0xE7EBF1, S(1));
    auto hoverBox = [&](float bx, float bw, const std::string& id) {
        if (hoverId_ == id) fillRoundRect(g, RectF(bx, y + S(4), bw, h - S(8)), S(8), 0xF2F5FA);
    };
    hoverBox(x + S(3), S(31), "zoom:out");
    drawIcon(g, Icon::Minus, RectF(x + S(10), y + S(12), S(14), S(14)), 0x5A6474, S(1.7f));
    addRegion("zoom:out", RectF(x + S(3), y + S(4), S(31), h - S(8)), L"缩小  (-)", CursorKind::Hand);
    std::wstring pct = std::to_wstring((int)std::round(zoom_ * 100)) + L"%";
    hoverBox(x + S(35), S(72), "zoom:reset");
    drawTextAligned(g, pct, RectF(x + S(35), y, S(72), h), fNav_, 0x3B4555,
                    StringAlignmentCenter, StringAlignmentCenter, false);
    addRegion("zoom:reset", RectF(x + S(35), y + S(4), S(72), h - S(8)), L"恢复到 100%", CursorKind::Hand);
    hoverBox(x + S(109), S(31), "zoom:in");
    drawIcon(g, Icon::Plus, RectF(x + S(117), y + S(12), S(14), S(14)), 0x5A6474, S(1.7f));
    addRegion("zoom:in", RectF(x + S(109), y + S(4), S(31), h - S(8)), L"放大  (+)", CursorKind::Hand);
    Pen line(RGBA(0xE9EDF3), S(1));
    g.DrawLine(&line, x + S(143), y + S(11), x + S(143), y + h - S(11));
    hoverBox(x + S(146), S(30), "zoom:fit");
    drawIcon(g, Icon::Fit, RectF(x + S(154), y + S(11), S(15), S(15)), 0x5A6474, S(1.5f));
    addRegion("zoom:fit", RectF(x + S(146), y + S(4), S(30), h - S(8)), L"适应页面  (0)", CursorKind::Hand);
}

void App::drawStatus(Graphics& g) {
    std::wstring line = L"工具：" + toolName(tool_) + L"   ·   空格拖动画布   ·   Ctrl+滚轮缩放";
    drawTextAligned(g, line, RectF(S(20), (float)H_ - S(31), S(300), S(20)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, true);
}

// ---------------------------------------------------------------------------
// background panel
// ---------------------------------------------------------------------------

void App::drawBgPanel(Graphics& g) {
    if (!bgPanel_) return;
    float w = S(340), h = S(262);
    float x = S(94);
    float y = headerH() + S(18);
    RectF r(x, y, w, h);
    drawShadow(g, r, S(15), S(12), S(4));
    fillRoundRect(g, r, S(15), COL_WHITE);
    strokeRoundRect(g, r, S(15), 0xE5EAF1, S(1));
    drawTextAligned(g, L"页面背景", RectF(x + S(18), y + S(13), S(200), S(26)), fTitle_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"只应用到当前页", RectF(x + S(18), y + S(37), S(200), S(18)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    RectF closeR(x + w - S(42), y + S(11), S(30), S(30));
    if (hoverId_ == "bg:close") fillRoundRect(g, closeR, S(8), 0xF2F5FA);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(8), closeR.Y + S(8), S(14), S(14)), 0x7B8495, S(1.5f));
    addRegion("bg:close", closeR, L"关闭", CursorKind::Hand);

    drawTextAligned(g, L"纸张", RectF(x + S(18), y + S(67), S(60), S(18)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    const uint32_t papers[] = {COL_WHITE, COL_CREAM, COL_GREENPAPER, COL_BLUEPAPER};
    const wchar_t* paperNames[] = {L"纯白", L"米黄", L"浅绿", L"浅蓝"};
    for (int i = 0; i < 4; ++i) {
        float cx = x + S(38) + S(47) * i;
        float cy = y + S(100);
        bool sel = page().paper == papers[i];
        std::string id = "bg:paper:" + hexColor(papers[i]);
        if (sel) {
            Pen ring(RGBA(COL_ACCENT), S(1.8f));
            g.DrawEllipse(&ring, cx - S(14), cy - S(14), S(28), S(28));
        } else if (hoverId_ == id) {
            Pen ring(RGBA(0xC9D2E2), S(1.2f));
            g.DrawEllipse(&ring, cx - S(14), cy - S(14), S(28), S(28));
        }
        SolidBrush b(RGBA(papers[i]));
        g.FillEllipse(&b, cx - S(11), cy - S(11), S(22), S(22));
        Pen edge(RGBA(0xDDE3EB), S(1));
        g.DrawEllipse(&edge, cx - S(11), cy - S(11), S(22), S(22));
        addRegion(id, RectF(cx - S(18), cy - S(18), S(36), S(36)), paperNames[i], CursorKind::Hand);
    }

    drawTextAligned(g, L"底纹", RectF(x + S(18), y + S(126), S(60), S(18)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    const wchar_t* patterns[] = {L"blank", L"grid", L"ruled", L"dots"};
    const wchar_t* pnames[] = {L"空白", L"方格", L"横线", L"点阵"};
    for (int i = 0; i < 4; ++i) {
        float cx = x + S(18) + S(78) * i;
        float cy = y + S(151);
        RectF cr(cx, cy, S(66), S(46));
        bool sel = page().pattern == patterns[i];
        fillRoundRect(g, cr, S(7), COL_WHITE);
        strokeRoundRect(g, cr, S(7), sel ? COL_ACCENT : 0xE3E8EF, sel ? S(1.8f) : S(1));
        if (i == 1) {
            Pen p(RGBA(0xDCE3ED), S(0.8f));
            for (int k = 1; k < 4; ++k) g.DrawLine(&p, cx + S(66) * k / 4, cy + S(3), cx + S(66) * k / 4, cy + S(43));
            for (int k = 1; k < 3; ++k) g.DrawLine(&p, cx + S(3), cy + S(46) * k / 3, cx + S(63), cy + S(46) * k / 3);
        } else if (i == 2) {
            Pen p(RGBA(0xDCE3ED), S(0.8f));
            for (int k = 1; k < 4; ++k) g.DrawLine(&p, cx + S(3), cy + S(46) * k / 4, cx + S(63), cy + S(46) * k / 4);
        } else if (i == 3) {
            SolidBrush b(RGBA(0xC8D1E0));
            for (int yy = 1; yy < 5; ++yy)
                for (int xx = 1; xx < 7; ++xx)
                    g.FillEllipse(&b, cx + S(66) * xx / 7 - S(1), cy + S(46) * yy / 5 - S(1), S(2), S(2));
        }
        addRegion("bg:pattern:" + std::string(utf8(patterns[i])), cr, pnames[i], CursorKind::Hand);
        drawTextAligned(g, pnames[i], RectF(cx, cy + S(48), S(66), S(16)), fMini_,
                        sel ? COL_ACCENT : COL_MUTED, StringAlignmentCenter, StringAlignmentCenter, false);
    }
}

// ---------------------------------------------------------------------------
// toast / hint / tooltip
// ---------------------------------------------------------------------------

void App::drawToast(Graphics& g) {
    if (toast_.empty() || GetTickCount64() > toastUntil_) return;
    RectF measured;
    g.MeasureString(toast_.c_str(), -1, fToast_.get(), PointF(0, 0), &measured);
    float w = std::min((float)W_ - S(40), std::max(S(260), measured.Width + S(74)));
    float h = S(42);
    float x = ((float)W_ - w) / 2;
    float slide = 0.0f;
    if (toastAnimStart_ != 0) {
        float t = std::min(1.0f, (float)(GetTickCount64() - toastAnimStart_) / 160.0f);
        slide = S(10) * (1.0f - easeOutCubic(t));
    }
    float y = headerH() + S(14) - slide;
    RectF r(x, y, w, h);
    drawGlass(g, r, S(14), 238);
    float isz = S(18);
    RectF ir(x + S(16), y + (h - isz) / 2, isz, isz);
    if (toastError_) {
        SolidBrush b(RGBA(COL_DANGER));
        g.FillEllipse(&b, ir);
        Pen p(RGBA(COL_WHITE), S(1.5f));
        g.DrawLine(&p, ir.X + S(6), ir.Y + S(6), ir.GetRight() - S(6), ir.GetBottom() - S(6));
        g.DrawLine(&p, ir.GetRight() - S(6), ir.Y + S(6), ir.X + S(6), ir.GetBottom() - S(6));
    } else {
        SolidBrush b(RGBA(COL_SUCCESS));
        g.FillEllipse(&b, ir);
        drawCheckmark(g, ir, COL_WHITE, S(1.8f));
    }
    drawTextAligned(g, toast_, RectF(x + S(44), y, w - S(58), h), fToast_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, true);
}

void App::drawTextEditorHint(Graphics& g) {
    if (!hEdit_) return;
    RECT rc;
    GetWindowRect(hEdit_, &rc);
    POINT pts[2] = {{rc.left, rc.top}, {rc.right, rc.bottom}};
    MapWindowPoints(HWND_DESKTOP, hwnd_, pts, 2);
    std::wstring hint = L"Ctrl+Enter 完成  ·  Esc 取消";
    RectF measured;
    g.MeasureString(hint.c_str(), -1, fTextHint_.get(), PointF(0, 0), &measured);
    float bh = S(30), bw = S(58);
    float hintW = measured.Width + S(22);
    float totalW = hintW + S(8) + bw;
    float y = (float)pts[1].y + S(5);
    if (y + bh > H_ - S(20)) y = (float)pts[0].y - bh - S(5);
    float x = std::max(S(8), std::min((float)pts[1].x - totalW, (float)W_ - totalW - S(8)));
    RectF hr(x, y, hintW, bh);
    fillRoundRect(g, hr, S(9), 0x33404F, 245);
    drawTextAligned(g, hint, RectF(x + S(11), y, hintW - S(18), bh), fTextHint_, COL_WHITE,
                    StringAlignmentNear, StringAlignmentCenter, false);
    RectF br(x + hintW + S(8), y, bw, bh);
    bool hover = hoverId_ == "text:done";
    fillRoundRect(g, br, S(9), hover ? 0x2457D6 : COL_ACCENT);
    drawTextAligned(g, L"完成", br, fTextHint_, COL_WHITE, StringAlignmentCenter,
                    StringAlignmentCenter, false);
    addRegion("text:done", br, L"提交文字", CursorKind::Hand);
}

void App::drawTooltip(Graphics& g) {
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

// ---------------------------------------------------------------------------
// overlays
// ---------------------------------------------------------------------------

void App::drawOverlay(Graphics& g) {
    if (overlay_ == 1) drawPagesOverlay(g);
    else if (overlay_ == 2) drawHelpOverlay(g);
    else if (overlay_ == 3) drawSettings(g);
    else if (overlay_ == 4) drawFunctionDialog(g);
    else if (overlay_ == 6) drawDevPasswordDialog(g);
}

void App::drawPagesOverlay(Graphics& g) {
    fillRoundRect(g, RectF(0, 0, (float)W_, (float)H_), 0, 0x0B1220, 78);
    addRegion("ov:scrim", RectF(0, 0, (float)W_, (float)H_), L"", CursorKind::Arrow);
    float pw = std::min((float)W_ - S(40), S(820));
    float ph = std::min((float)H_ - S(70), S(590));
    float px = ((float)W_ - pw) / 2, py = ((float)H_ - ph) / 2;
    RectF panel(px, py, pw, ph);
    drawShadow(g, panel, S(20), S(18), S(8));
    fillRoundRect(g, panel, S(18), COL_WHITE);
    addRegion("ov:panel", panel, L"", CursorKind::Arrow);

    drawTextAligned(g, L"页面管理", RectF(px + S(24), py + S(16), S(300), S(30)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"共 " + std::to_wstring(pageCount()) + L" 页  ·  拖动排序 · 滚轮翻页",
                    RectF(px + S(24), py + S(43), S(430), S(20)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    RectF closeR(px + pw - S(48), py + S(15), S(32), S(32));
    if (hoverId_ == "ov:pages:close") fillRoundRect(g, closeR, S(8), 0xF2F5FA);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(9), closeR.Y + S(9), S(14), S(14)), 0x6B7585, S(1.6f));
    addRegion("ov:pages:close", closeR, L"关闭", CursorKind::Hand);

    float cx0 = px + S(24), cy0 = py + S(78);
    float cw = pw - S(48), ch = ph - S(156);
    float gap = S(16);
    float cardW = (cw - gap * 2) / 3.0f;
    float cardH = S(184);
    int rows = (pageCount() + 2) / 3;
    float totalH = rows * (cardH + gap);
    float maxScroll = std::max(0.0f, totalH - ch);
    if (pageScrollToCurrent_) {
        int row = current_ / 3;
        pageScroll_ = std::max(0.0f, row * (cardH + gap) - ch * 0.35f);
        pageScrollToCurrent_ = false;
        pageScrollAnimating_ = false;
    }
    pageScroll_ = std::max(0.0f, std::min(pageScroll_, maxScroll));
    pageScrollFrom_ = std::max(0.0f, std::min(pageScrollFrom_, maxScroll));
    pageScrollTo_ = std::max(0.0f, std::min(pageScrollTo_, maxScroll));

    int bw = std::max(1, (int)std::ceil(cw));
    int bh = std::max(1, (int)std::ceil(ch));
    if (!pageContentBmp_ || pageContentBmp_->GetWidth() != bw || pageContentBmp_->GetHeight() != bh)
        pageContentBmp_ = std::make_unique<Bitmap>(bw, bh, PixelFormat32bppPARGB);
    {
        Graphics cg(pageContentBmp_.get());
        cg.SetCompositingMode(CompositingModeSourceCopy);
        cg.Clear(Color(0, 0, 0, 0));
        cg.SetCompositingMode(CompositingModeSourceOver);
        setupGraphics(cg);

        for (int i = 0; i < pageCount(); ++i) {
            int col = i % 3, row = i / 3;
            float lx = col * (cardW + gap);
            float ly = row * (cardH + gap) - pageScroll_;
            RectF card(lx, ly, cardW, cardH);
            if (card.GetBottom() < -S(4) || card.Y > ch + S(4)) continue;
            std::string id = "ov:pages:page:" + strOf(i);
            bool current = i == current_;
            bool hover = hoverId_ == id;
            fillRoundRect(cg, card, S(12), hover ? 0xF7F9FD : COL_WHITE);
            if (pageDragActive_ && i == pageDragFrom_)
                fillRoundRect(cg, card, S(12), COL_ACCENT_SOFT, 150);
            strokeRoundRect(cg, card, S(12), current ? COL_ACCENT : 0xE2E7EF,
                            current ? S(2.0f) : S(1));
            if (pageDragActive_ && i == pageDragTo_ && i != pageDragFrom_) {
                fillRoundRect(cg, card, S(12), COL_ACCENT_SOFT, 110);
                strokeRoundRect(cg, card, S(12), COL_ACCENT, S(3.0f), 235);
            }
            float thumbW = cardW - S(20);
            float thumbH = thumbW * 9.0f / 16.0f;
            RectF thumb(card.X + S(10), card.Y + S(10), thumbW, thumbH);
            Xf xf;
            xf.s = thumbW / PAGE_W;
            xf.o = {thumb.X, thumb.Y};
            fillRoundRect(cg, thumb, S(5), COL_WHITE);
            GraphicsState ts = cg.Save();
            GraphicsPath tp;
            roundPath(tp, thumb, S(5));
            cg.SetClip(&tp, CombineModeReplace);
            drawPageContent(cg, pages_[i], xf, thumb, false);
            cg.Restore(ts);
            strokeRoundRect(cg, thumb, S(5), 0xE7EBF2, S(1));
            drawTextAligned(cg, L"第 " + std::to_wstring(i + 1) + L" 页",
                            RectF(card.X + S(12), card.GetBottom() - S(37), cardW * 0.55f, S(24)),
                            fSmall_, current ? COL_ACCENT : 0x3B4555,
                            StringAlignmentNear, StringAlignmentCenter, false);
            drawTextAligned(cg, std::to_wstring(pages_[i].items.size()) + L" 个对象",
                            RectF(card.GetRight() - S(100), card.GetBottom() - S(37), S(88), S(24)),
                            fMini_, COL_MUTED, StringAlignmentFar, StringAlignmentCenter, false);
            if (current) {
                SolidBrush dot(RGBA(COL_ACCENT));
                cg.FillEllipse(&dot, card.GetRight() - S(18), card.Y + S(16), S(7), S(7));
            }
            RectF absCard(cx0 + lx, cy0 + ly, cardW, cardH);
            addRegion(id, absCard, L"第 " + std::to_wstring(i + 1) + L" 页 · 点击切换 / 拖动排序",
                      CursorKind::Hand);
        }
    }
    g.SetCompositingMode(CompositingModeSourceOver);
    g.DrawImage(pageContentBmp_.get(), RectF(cx0, cy0, cw, ch), 0, 0, (float)bw, (float)bh,
                UnitPixel);

    float fy = py + ph - S(58);
    drawHeaderButton(g, "ov:pages:new", RectF(px + S(24), fy, S(112), S(36)), Icon::Plus, L"新建页面",
                     true, false, true, L"新建页面");
    drawHeaderButton(g, "ov:pages:dup", RectF(px + S(144), fy, S(104), S(36)), Icon::Copy, L"复制",
                     false, true, true, L"复制当前页");
    drawHeaderButton(g, "ov:pages:del", RectF(px + S(256), fy, S(104), S(36)), Icon::Trash, L"删除",
                     false, true, pageCount() > 1, L"删除当前页");
    drawHeaderButton(g, "ov:pages:close", RectF(px + pw - S(114), fy, S(90), S(36)), Icon::Close, L"关闭",
                     false, false, true, L"关闭页面管理");
    if (maxScroll > 0) {
        float trackY = cy0 + S(6), trackH = ch - S(12);
        fillRoundRect(g, RectF(px + pw - S(12), trackY, S(4), trackH), S(2), 0xEEF1F6);
        float thumbH2 = std::max(S(30), trackH * ch / totalH);
        float thumbY = trackY + (trackH - thumbH2) * (pageScroll_ / maxScroll);
        fillRoundRect(g, RectF(px + pw - S(12), thumbY, S(4), thumbH2), S(2), 0xC9D3E2);
        pageScrollMax_ = maxScroll;
        pageScrollTrackH_ = trackH;
        pageScrollThumbH_ = thumbH2;
        addRegion("ov:pages:scroll", RectF(px + pw - S(26), cy0, S(26), ch),
                  L"拖动滚动页面列表", CursorKind::SizeAll);
    }
}

void App::drawFunctionDialog(Graphics& g) {
    fillRoundRect(g, RectF(0, 0, (float)W_, (float)H_), 0, 0x0B1220, 78);
    addRegion("ov:scrim", RectF(0, 0, (float)W_, (float)H_), L"", CursorKind::Arrow);
    float pw = std::min((float)W_ - S(40), S(640));
    float ph = S(380);
    float px = ((float)W_ - pw) / 2, py = ((float)H_ - ph) / 2;
    RectF panel(px, py, pw, ph);
    drawShadow(g, panel, S(20), S(18), S(8));
    fillRoundRect(g, panel, S(18), COL_WHITE);
    addRegion("ov:panel", panel, L"", CursorKind::Arrow);

    bool pm = fnPointMode_;
    std::wstring title = pm ? L"描点" : (editingFunction_ >= 0 ? L"修改函数图像" : L"插入函数图像");
    std::wstring subtitle = pm ? L"输入点的坐标，例如 (2,3) 或 2,3；点会画在函数坐标系上"
                               : L"输入 y = f(x)，例如 sin(x)、x^2、sqrt(x)、1/x";
    drawTextAligned(g, title, RectF(px + S(28), py + S(18), S(420), S(30)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, subtitle, RectF(px + S(28), py + S(47), S(560), S(22)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    RectF closeR(px + pw - S(48), py + S(16), S(32), S(32));
    if (hoverId_ == "fn:close") fillRoundRect(g, closeR, S(8), 0xF2F5FA);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(9), closeR.Y + S(9), S(14), S(14)), 0x6B7585, S(1.6f));
    addRegion("fn:close", closeR, L"关闭", CursorKind::Hand);

    RectF field(px + S(28), py + S(86), pw - S(56), S(52));
    fillRoundRect(g, field, S(12), 0xF6F9FD);
    strokeRoundRect(g, field, S(12), 0xBFD8CE, S(1.4f));
    addRegion("fn:field", field, L"函数解析式", CursorKind::IBeam);
    std::wstring shown = pm ? wide(fnExpr_) : (L"y = " + (fnExpr_.empty() ? L"" : wide(fnExpr_)));
    std::wstring placeholder = pm ? L"输入坐标，如 (2,3)" : L"y = 在这里输入解析式";
    uint32_t fieldColor = fnExpr_.empty() ? 0x9AA9A1 : COL_INK;
    drawTextAligned(g, fnExpr_.empty() ? placeholder : shown,
                    RectF(field.X + S(16), field.Y, field.Width - S(28), field.Height), fUI_,
                    fieldColor, StringAlignmentNear, StringAlignmentCenter, true);
    if (!fnExpr_.empty()) {
        RectF measured;
        g.MeasureString(shown.c_str(), -1, fUI_.get(), PointF(0, 0), &measured);
        float cx = field.X + S(16) + measured.Width + S(2);
        float cy0 = field.Y + S(12), cy1 = field.GetBottom() - S(12);
        if (cx < field.GetRight() - S(10)) {
            Pen caret(RGBA(COL_ACCENT), S(1.6f));
            g.DrawLine(&caret, cx, cy0, cx, cy1);
        }
    }

    drawTextAligned(g, L"常用示例", RectF(px + S(28), py + S(158), S(200), S(20)), fSmall_, 0x53665B,
                    StringAlignmentNear, StringAlignmentCenter, false);
    const wchar_t* examples[] = {L"x^2", L"1/x", L"sin(x)", L"cos(x)", L"tan(x)"};
    float ex = px + S(28), ey = py + S(184);
    for (const wchar_t* example : examples) {
        std::wstring label = example;
        RectF r(ex, ey, S(86), S(30));
        std::string exampleId = "fn:example:" + utf8(label);
        if (hoverId_ == exampleId) {
            fillRoundRect(g, r, S(9), 0xEAF6F1);
        } else {
            fillRoundRect(g, r, S(9), 0xF3F7FA);
        }
        drawTextAligned(g, label, r, fMini_, 0x4B6055, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        addRegion(exampleId, r, L"使用示例 " + label, CursorKind::Hand);
        ex += S(94);
    }
    {
        RectF sq(ex, ey, S(74), S(30));
        if (hoverId_ == "fn:square") fillRoundRect(g, sq, S(9), 0xEAF6F1);
        else fillRoundRect(g, sq, S(9), 0xEAF2FF);
        drawTextAligned(g, L"x²", sq, fSmall_, 0x2457D6, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        addRegion("fn:square", sq, L"插入平方 x²（等价 x^2）", CursorKind::Hand);
    }

    RectF cb(px + S(28), py + S(226), S(20), S(20));
    fillRoundRect(g, cb, S(6), fnShowAxes_ ? COL_ACCENT : 0xEDF2F7);
    if (fnShowAxes_) drawCheckmark(g, cb, COL_WHITE, S(1.7f));
    drawTextAligned(g, L"显示平面直角坐标系（含网格）",
                    RectF(px + S(56), py + S(222), S(360), S(28)), fSmall_, 0x4B5D53,
                    StringAlignmentNear, StringAlignmentCenter, false);
    addRegion("fn:axes", RectF(px + S(24), py + S(218), S(410), S(36)),
              L"切换坐标系显示", CursorKind::Hand);

    if (!fnError_.empty()) {
        drawTextAligned(g, fnError_, RectF(px + S(28), py + S(258), pw - S(56), S(26)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, pm ? L"点会按函数坐标 (x,y) 映射到图中；可 Ctrl+点击两个点后右键连线"
                               : L"支持 +  -  *  /  ^、平方 ²、括号和 sin/cos/tan/sqrt/abs/log/ln/exp 等函数",
                        RectF(px + S(28), py + S(258), pw - S(56), S(24)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }

    drawHeaderButton(g, "fn:insert", RectF(px + pw - S(150), py + ph - S(64), S(112), S(38)),
                     Icon::Check, pm ? L"描点" : L"插入", true, false, true,
                     pm ? L"把点画到函数图上" : L"生成并插入函数图像");
    drawHeaderButton(g, "fn:close", RectF(px + pw - S(258), py + ph - S(64), S(94), S(38)),
                     Icon::Close, L"取消", false, true, true, L"关闭函数输入");
    drawTextAligned(g, pm ? L"Enter 描点   ·   Esc 取消   ·   Ctrl+点击两个点后右键连线"
                          : L"Enter 插入   ·   Esc 取消   ·   插入后可直接拖动曲线",
                    RectF(px + S(28), py + ph - S(58), S(380), S(26)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
}

void App::drawDevPasswordDialog(Graphics& g) {
    fillRoundRect(g, RectF(0, 0, (float)W_, (float)H_), 0, 0x0B1220, 92);
    addRegion("ov:scrim", RectF(0, 0, (float)W_, (float)H_), L"", CursorKind::Arrow);
    float pw = S(500), ph = S(310);
    float px = ((float)W_ - pw) / 2, py = ((float)H_ - ph) / 2;
    RectF panel(px, py, pw, ph);
    drawShadow(g, panel, S(20), S(18), S(8));
    fillRoundRect(g, panel, S(18), COL_WHITE);
    addRegion("ov:panel", panel, L"", CursorKind::Arrow);

    RectF lock(px + S(28), py + S(26), S(38), S(38));
    fillRoundRect(g, lock, S(12), COL_ACCENT_SOFT, 235);
    drawIcon(g, Icon::Settings, RectF(lock.X + S(10), lock.Y + S(10), S(18), S(18)),
             COL_ACCENT, S(1.6f));
    drawTextAligned(g, L"开发者验证", RectF(px + S(80), py + S(24), S(300), S(28)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"请输入开发者密码后继续", RectF(px + S(80), py + S(52), S(300), S(20)),
                    fMini_, COL_MUTED, StringAlignmentNear, StringAlignmentCenter, false);
    RectF closeR(px + pw - S(48), py + S(18), S(32), S(32));
    if (hoverId_ == "devpass:cancel") fillRoundRect(g, closeR, S(8), 0xF2F5FA);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(9), closeR.Y + S(9), S(14), S(14)), 0x6B7585, S(1.6f));
    addRegion("devpass:cancel", closeR, L"取消", CursorKind::Hand);

    RectF field(px + S(28), py + S(96), pw - S(56), S(50));
    fillRoundRect(g, field, S(12), 0xF6F9FD);
    strokeRoundRect(g, field, S(12), devPassError_.empty() ? 0xBFD8CE : COL_DANGER,
                    devPassError_.empty() ? S(1.4f) : S(1.8f));
    std::wstring masked(devPass_.size(), L'●');
    if (masked.empty()) {
        drawTextAligned(g, L"输入密码", RectF(field.X + S(16), field.Y, field.Width - S(28), field.Height),
                        fUI_, 0x9AA9A1, StringAlignmentNear, StringAlignmentCenter, false);
    } else {
        drawTextAligned(g, masked, RectF(field.X + S(16), field.Y, field.Width - S(28), field.Height),
                        fUI_, COL_INK, StringAlignmentNear, StringAlignmentCenter, false);
        RectF measured;
        g.MeasureString(masked.c_str(), -1, fUI_.get(), PointF(0, 0), &measured);
        float cx = field.X + S(16) + measured.Width + S(2);
        if (cx < field.GetRight() - S(10)) {
            Pen caret(RGBA(COL_ACCENT), S(1.6f));
            g.DrawLine(&caret, cx, field.Y + S(12), cx, field.GetBottom() - S(12));
        }
    }
    addRegion("devpass:field", field, L"开发者密码", CursorKind::IBeam);

    if (!devPassError_.empty()) {
        drawTextAligned(g, devPassError_, RectF(px + S(28), py + S(154), pw - S(56), S(22)), fSmall_,
                        COL_DANGER, StringAlignmentNear, StringAlignmentCenter, false);
    } else {
        drawTextAligned(g, L"密码区分大小写", RectF(px + S(28), py + S(154), pw - S(56), S(22)), fMini_,
                        COL_MUTED, StringAlignmentNear, StringAlignmentCenter, false);
    }
    drawHeaderButton(g, "devpass:ok", RectF(px + pw - S(150), py + ph - S(60), S(112), S(36)),
                     Icon::Check, L"解锁", true, false, true, L"验证开发者密码");
    drawHeaderButton(g, "devpass:cancel", RectF(px + pw - S(258), py + ph - S(60), S(94), S(36)),
                     Icon::Close, L"取消", false, true, true, L"返回设置");
    drawTextAligned(g, L"Enter 解锁   ·   Esc 取消", RectF(px + S(28), py + ph - S(54), S(300), S(24)),
                    fMini_, COL_MUTED, StringAlignmentNear, StringAlignmentCenter, false);
}

void App::drawHelpOverlay(Graphics& g) {
    fillRoundRect(g, RectF(0, 0, (float)W_, (float)H_), 0, 0x0B1220, 78);
    addRegion("ov:scrim", RectF(0, 0, (float)W_, (float)H_), L"", CursorKind::Arrow);
    float pw = std::min((float)W_ - S(40), S(560));
    float ph = std::min((float)H_ - S(70), S(560));
    float px = ((float)W_ - pw) / 2, py = ((float)H_ - ph) / 2;
    RectF panel(px, py, pw, ph);
    drawShadow(g, panel, S(20), S(18), S(8));
    fillRoundRect(g, panel, S(18), COL_WHITE);
    addRegion("ov:panel", panel, L"", CursorKind::Arrow);
    drawTextAligned(g, L"快捷键与技巧", RectF(px + S(28), py + S(18), S(320), S(30)), fBrand_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"常用操作，一学就会", RectF(px + S(28), py + S(44), S(300), S(20)), fMini_, COL_MUTED,
                    StringAlignmentNear, StringAlignmentCenter, false);
    RectF closeR(px + pw - S(48), py + S(16), S(32), S(32));
    if (hoverId_ == "ov:help:close") fillRoundRect(g, closeR, S(8), 0xF2F5FA);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(9), closeR.Y + S(9), S(14), S(14)), 0x6B7585, S(1.6f));
    addRegion("ov:help:close", closeR, L"关闭", CursorKind::Hand);

    struct Row { const wchar_t* key; const wchar_t* desc; };
    const Row rows[] = {
    {L"P / H / E", L"画笔 / 荧光笔 / 橡皮"},
        {L"V / R / T", L"选择 / 形状 / 文字"},
    {L"L", L"激光笔"},
        {L"Ctrl+Z / Ctrl+Y", L"撤销 / 重做"},
        {L"Ctrl+S / Ctrl+O", L"保存 / 打开"},
        {L"Ctrl+E", L"导出当前页为 PNG"},
        {L"Delete / Ctrl+D", L"删除选中 / 复制选中"},
        {L"空格 + 拖动", L"临时移动画布"},
        {L"Ctrl+滚轮 / 0", L"缩放 / 适应页面"},
    {L"F11 / F1", L"全屏 / 打开本帮助"},
    };
    float y = py + S(82);
    for (auto& row : rows) {
        RectF keyR(px + S(28), y, S(150), S(28));
        fillRoundRect(g, keyR, S(7), 0xF2F5FA);
        drawTextAligned(g, row.key, RectF(keyR.X + S(10), keyR.Y, keyR.Width - S(18), keyR.Height),
                        fMini_, 0x40506A, StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, row.desc, RectF(px + S(192), y, pw - S(220), S(28)), fSmall_, 0x4B5565,
                        StringAlignmentNear, StringAlignmentCenter, false);
        y += S(35);
    }
    drawHeaderButton(g, "ov:help:close", RectF(px + pw - S(144), py + ph - S(56), S(120), S(36)),
    Icon::Pen, L"开始书写", true, false, true, L"关闭帮助");
}

}  // namespace qhb
