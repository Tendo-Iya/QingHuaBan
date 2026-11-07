#include "qhb.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace qhb {

static std::string idNum(const char* prefix, float v) {
    return std::string(prefix) + utf8(fmtFloat(v, 0));
}

// ---------------------------------------------------------------------------
// tiny helpers
// ---------------------------------------------------------------------------

static void coverImage(Graphics& g, Bitmap* bmp, const RectF& r) {
    if (!bmp || bmp->GetWidth() <= 0 || bmp->GetHeight() <= 0) return;
    float iw = (float)bmp->GetWidth();
    float ih = (float)bmp->GetHeight();
    float scale = std::max(r.Width / iw, r.Height / ih);
    float w = iw * scale;
    float h = ih * scale;
    g.DrawImage(bmp, RectF(r.X + (r.Width - w) / 2, r.Y + (r.Height - h) / 2, w, h));
}

static bool imageIsDark(Bitmap* bmp) {
    if (!bmp || bmp->GetWidth() <= 0 || bmp->GetHeight() <= 0) return false;
    int iw = bmp->GetWidth();
    int ih = bmp->GetHeight();
    double sum = 0.0;
    int n = 0;
    for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 16; ++xx) {
            int px = std::min(iw - 1, (int)((xx + 0.5) * iw / 16.0));
            int py = std::min(ih - 1, (int)((yy + 0.5) * ih / 9.0));
            Color c;
            if (bmp->GetPixel(px, py, &c) != Ok) continue;
            sum += 0.299 * c.GetR() + 0.587 * c.GetG() + 0.114 * c.GetB();
            ++n;
        }
    }
    return n > 0 && (sum / n) < 112.0;
}

static void roundedImage(Graphics& g, Bitmap* bmp, const RectF& r, float radius, uint32_t fallback) {
    GraphicsPath path;
    roundPath(path, r, radius);
    GraphicsState st = g.Save();
    g.SetClip(&path, CombineModeReplace);
    if (bmp) {
        coverImage(g, bmp, r);
    } else {
        LinearGradientBrush bg(PointF(r.X, r.Y), PointF(r.X, r.GetBottom()),
                               RGBA(0x2AC79A), RGBA(COL_ACCENT));
        g.FillRectangle(&bg, r);
        Font f(L"Microsoft YaHei UI", r.Height * 0.52f, FontStyleBold, UnitPixel);
        drawTextAligned(g, L"H", r, &f, 0xFFFFFF, StringAlignmentCenter, StringAlignmentCenter, false);
    }
    g.Restore(st);
    strokeRoundRect(g, r, radius, 0xFFFFFF, 1.2f, 150);
}

// ---------------------------------------------------------------------------
// glass surfaces
// ---------------------------------------------------------------------------

void App::drawGlass(Graphics& g, RectF r, float radius, BYTE alpha) {
    if (r.Width < 2 || r.Height < 2) return;
    drawShadow(g, r, radius, S(12), S(5));
    GraphicsPath path;
    roundPath(path, r, radius);
    Color top(alpha, 255, 255, 255);
    Color bottom((BYTE)std::max(30, (int)alpha - 32), 238, 246, 242);
    LinearGradientBrush bg(PointF(r.X, r.Y), PointF(r.X, r.GetBottom()), top, bottom);
    g.FillPath(&bg, &path);
    Pen edge(Color(180, 255, 255, 255), S(1.0f));
    g.DrawPath(&edge, &path);
    Pen high(Color(210, 255, 255, 255), S(1.0f));
    g.DrawLine(&high, r.X + radius * 0.6f, r.Y + S(1.0f),
               r.GetRight() - radius * 0.6f, r.Y + S(1.0f));
    Pen low(Color(45, 88, 110, 99), S(1.0f));
    g.DrawLine(&low, r.X + radius, r.GetBottom() - S(0.8f),
               r.GetRight() - radius, r.GetBottom() - S(0.8f));
}

void App::drawGlassCapsule(Graphics& g, RectF r, BYTE alpha) {
    drawGlass(g, r, r.Height / 2, alpha);
}

// ---------------------------------------------------------------------------
// backdrop
// ---------------------------------------------------------------------------

void App::drawBackdrop(Graphics& g) {
    LinearGradientBrush bg(PointF(0, 0), PointF((float)W_, (float)H_),
                           RGBA(0xF8FBF9), RGBA(0xE8F0EB));
    g.FillRectangle(&bg, RectF(0, 0, (float)W_, (float)H_));
    auto orb = [&](float cx, float cy, float rad, uint32_t col, int alpha) {
        for (int i = 0; i < 7; ++i) {
            float rr = rad * (1.0f + i * 0.21f);
            SolidBrush b(RGBA(col, (BYTE)std::max(2, alpha / (i + 1))));
            g.FillEllipse(&b, cx - rr, cy - rr, rr * 2, rr * 2);
        }
    };
    orb((float)W_ * 0.84f, (float)H_ * 0.12f, S(260), 0x9FE2C7, 22);
    orb((float)W_ * 0.10f, (float)H_ * 0.84f, S(240), 0xFFE0C7, 18);
    orb((float)W_ * 0.55f, (float)H_ * 0.48f, S(300), 0xD8E9FF, 12);
    LinearGradientBrush veil(PointF(0, 0), PointF(0, (float)H_),
                             RGBA(0xFFFFFF, 34), RGBA(0xFFFFFF, 0));
    g.FillRectangle(&veil, RectF(0, 0, (float)W_, (float)H_));
}

// ---------------------------------------------------------------------------
// brand
// ---------------------------------------------------------------------------

void App::drawBrand(Graphics& g, bool onSplash) {
    std::wstring name = appName_.empty() ? std::wstring(APP_NAME) : appName_;
    std::wstring title = onSplash ? std::wstring() : title_;
    RectF nm;
    g.MeasureString(name.c_str(), -1, fBrand_.get(), PointF(0, 0), &nm);
    float nameW = nm.Width;
    float titleW = 0;
    if (!title.empty()) {
        RectF tm;
        g.MeasureString(title.c_str(), -1, fMini_.get(), PointF(0, 0), &tm);
        titleW = tm.Width;
    }
    float contentW = std::max(nameW, titleW);
    float chipW = std::min(S(410), std::max(S(210), S(62) + contentW + S(24)));
    RectF chip(S(16), S(12), chipW, S(52));
    drawGlass(g, chip, S(16), 218);

    RectF lr(chip.X + S(9), chip.Y + S(8), S(36), S(36));
    Bitmap* bmp = customLogo_ ? customLogo_.get() : logo_.get();
    roundedImage(g, bmp, lr, S(9), COL_ACCENT);

    float tx = chip.X + S(48);
    float tw = chip.GetRight() - tx - S(12);
    if (onSplash) {
        drawTextAligned(g, name, RectF(tx, chip.Y, tw, chip.Height), fBrand_, COL_INK,
                        StringAlignmentNear, StringAlignmentCenter, true);
    } else {
        drawTextAligned(g, name, RectF(tx, chip.Y + S(3), tw, S(27)), fBrand_, COL_INK,
                        StringAlignmentNear, StringAlignmentCenter, true);
        drawTextAligned(g, title, RectF(tx, chip.Y + S(29), tw - S(10), S(19)), fMini_, COL_MUTED,
                        StringAlignmentNear, StringAlignmentCenter, true);
        if (dirty_) {
            SolidBrush dot(RGBA(COL_ACCENT));
            g.FillEllipse(&dot, chip.GetRight() - S(16), chip.Y + S(20), S(5), S(5));
        }
    }
}

// ---------------------------------------------------------------------------
// launch screen
// ---------------------------------------------------------------------------

void App::drawSplashLoading(Graphics& g, RectF card) {
    bool dark = imageIsDark(launcherBg_.get());
    float w = S(164), h = S(4);
    RectF track(card.X + (card.Width - w) / 2, card.Y + card.Height * 0.665f, w, h);
    fillRoundRect(g, track, h / 2, dark ? 0xFFFFFF : 0x26342E, dark ? 48 : 24);
    float t = 0;
    if (splashStart_ != 0) t = std::min(1.0f, std::max(0.0f, (float)(GetTickCount64() - splashStart_) / 2400.0f));
    if (t > 0.01f) fillRoundRect(g, RectF(track.X, track.Y, track.Width * t, h), h / 2, COL_ACCENT, 235);
}

void App::drawSplash(Graphics& g) {
    RectF card = splashRect();
    drawShadow(g, card, S(30), S(26), S(12));
    GraphicsPath cp;
    roundPath(cp, card, S(30));
    GraphicsState st = g.Save();
    g.SetClip(&cp, CombineModeReplace);
    bool dark = false;
    if (launcherBg_) {
        coverImage(g, launcherBg_.get(), card);
        dark = imageIsDark(launcherBg_.get());
        SolidBrush veil(RGBA(dark ? 0x07110C : 0xFFFFFF, dark ? 30 : 28));
        g.FillRectangle(&veil, card);
    } else {
        LinearGradientBrush bg(PointF(card.X, card.Y), PointF(card.GetRight(), card.GetBottom()),
                               RGBA(0xFFFFFF), RGBA(0xF3FAF6));
        g.FillRectangle(&bg, card);
        SolidBrush mint(RGBA(0x8FE0C4, 20));
        g.FillEllipse(&mint, card.GetRight() - S(420), card.Y - S(160), S(520), S(520));
        SolidBrush peach(RGBA(0xFFDCC6, 15));
        g.FillEllipse(&peach, card.X - S(160), card.GetBottom() - S(260), S(430), S(430));
    }

    // center wordmark
    float areaW = card.Width - S(150);
    float fs = S(64);
    while (fs > S(30)) {
        Font probe(L"Microsoft YaHei UI", fs, FontStyleBold, UnitPixel);
        RectF m;
        g.MeasureString(appName_.c_str(), -1, &probe, PointF(0, 0), &m);
        if (m.Width <= areaW) break;
        fs -= S(4);
    }
    Font hero(L"Microsoft YaHei UI", fs, FontStyleBold, UnitPixel);
    RectF nameArea(card.X + S(60), card.Y + card.Height * 0.375f,
                   card.Width - S(120), S(104));
    drawTextAligned(g, appName_, nameArea, &hero, dark ? 0xFFFFFF : 0x1D2A24,
                    StringAlignmentCenter, StringAlignmentCenter, false);

    std::wstring dotText = L"正在启动";
    int dots = (int)((GetTickCount64() / 360) % 4);
    for (int i = 0; i < dots; ++i) dotText += L".";
    RectF sub(card.X + S(60), nameArea.Y + S(100), card.Width - S(120), S(26));
    drawTextAligned(g, dotText, sub, fSplashSub_, dark ? 0xE6F2EC : 0x71827A,
                    StringAlignmentCenter, StringAlignmentCenter, false);

    drawSplashLoading(g, card);

    drawTextAligned(g, L"© 2026 HAVEGET工作室  ·  All Rights Reserved.",
                    RectF(card.X + S(24), card.GetBottom() - S(76), card.Width - S(48), S(22)),
                    fMini_, dark ? 0xE8F3EE : 0x5F7167,
                    StringAlignmentCenter, StringAlignmentCenter, false);
    drawTextAligned(g, L"点击任意处跳过",
                    RectF(card.GetRight() - S(164), card.GetBottom() - S(76), S(140), S(22)),
                    fMini_, dark ? 0xDCEBE4 : 0x6E8076,
                    StringAlignmentNear, StringAlignmentCenter, false);

    g.Restore(st);
    strokeRoundRect(g, card, S(30), 0xFFFFFF, S(1.2f), 170);

    drawBrand(g, true);

    ULONGLONG now = GetTickCount64();
    float intro = easeOutCubic((float)(now - splashStart_) / 430.0f);
    if (intro < 1.0f) {
        fillRoundRect(g, card, S(30), 0xFFFFFF, (BYTE)(80 * (1.0f - intro)));
    }
    if (splashLeaving_) {
        float t = easeOutCubic((float)(now - splashLeaveStart_) / 240.0f);
        fillRoundRect(g, card, S(30), 0xFFFFFF, (BYTE)(255 * t));
    }
}

// ---------------------------------------------------------------------------
// bottom dock
// ---------------------------------------------------------------------------

void App::drawDock(Graphics& g) {
    RectF r = dockRect();
    drawGlass(g, r, S(26), 232);

    // drag grip
    RectF grip(r.X + S(10), r.Y + S(12), S(20), S(40));
    if (hoverId_ == "dock:grip") fillRoundRect(g, grip, S(8), 0xFFFFFF, 80);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 2; ++col) {
            SolidBrush b(RGBA(0x6F8379, 150));
            g.FillEllipse(&b, grip.X + S(5) + col * S(6), grip.Y + S(12) + row * S(8), S(2.6f), S(2.6f));
        }
    }
    addRegion("dock:grip", grip, L"拖动工具栏，放到任意位置", CursorKind::SizeAll);

    float btn = S(34);
    float rowY = r.Y + S(10);
    float x = r.X + S(34);
    auto toolBtn = [&](const std::string& id, Icon ic, bool selected, bool enabled, const std::wstring& tip) {
        RectF br(x, rowY, btn, btn);
        bool hov = enabled && hoverId_ == id;
        float te = easeOutCubic(toolAnim_);
        if (selected) {
            float inset = S(4) * (1.0f - te);
            RectF pill = br;
            pill.Inflate(-inset, -inset);
            fillRoundRect(g, pill, S(10), COL_ACCENT_SOFT, (BYTE)(235 * te));
            strokeRoundRect(g, pill, S(10), 0xFFFFFF, S(1), (BYTE)(160 * te));
        } else if (hov) {
            fillRoundRect(g, br, S(10), 0xFFFFFF, 86);
        }
        uint32_t fg = !enabled ? 0xB8C4BD : (selected ? COL_ACCENT : 0x35463E);
        drawIcon(g, ic, RectF(br.X + S(8), br.Y + S(8), S(18), S(18)), fg, S(1.7f));
        if (selected) {
            SolidBrush dot(RGBA(COL_ACCENT, (BYTE)(255 * te)));
            g.FillEllipse(&dot, br.X + btn / 2 - S(2), br.GetBottom() - S(5), S(4), S(4));
        }
        if (enabled) addRegion(id, br, tip, CursorKind::Hand);
        x += btn;
    };

    struct ToolDef { Tool tool; Icon icon; const char* id; const wchar_t* tip; };
    const ToolDef toolDefs[] = {
        {Tool::Select, Icon::Cursor, "tool:select", L"选择  (V)"},
        {Tool::Pen, Icon::Pen, "tool:pen", L"画笔  (P)"},
        {Tool::Highlighter, Icon::Highlighter, "tool:highlighter", L"荧光笔  (H)"},
        {Tool::Eraser, Icon::Eraser, "tool:eraser", L"橡皮擦  (E)"},
        {Tool::Shape, Icon::Shape, "tool:shape", L"形状  (R)"},
        {Tool::Text, Icon::Text, "tool:text", L"文字  (T)"},
        {Tool::Laser, Icon::Laser, "tool:laser", L"激光笔  (L)"},
        {Tool::Background, Icon::Background, "tool:background", L"画布颜色与底纹  (B)"},
        {Tool::Hand, Icon::Hand, "tool:hand", L"移动画布"},
    };
    for (const auto& d : toolDefs) {
        toolBtn(d.id, d.icon, tool_ == d.tool, true, d.tip);
        if (d.tool == Tool::Background) {
            toolBtn("dock:image", Icon::Image, false, true, L"插入图片  (I)");
        }
    }
    toolBtn("dock:function", Icon::Function, false, true, L"插入函数图像");

    // divider
    Pen divider(RGBA(0x5D7469, 55), S(1));
    g.DrawLine(&divider, x + S(4), r.Y + S(13), x + S(4), r.Y + S(41));
    x += S(10);

    toolBtn("dock:undo", Icon::Undo, false, !undo_.empty(), L"撤销  (Ctrl+Z)");
    toolBtn("dock:redo", Icon::Redo, false, !redo_.empty(), L"重做  (Ctrl+Y)");

    // page capsule
    float pnW = S(108), pnH = S(34);
    RectF pn(x, rowY, pnW, pnH);
    fillRoundRect(g, pn, S(12), 0xFFFFFF, 78);
    strokeRoundRect(g, pn, S(12), 0xFFFFFF, S(1), 96);
    bool hasPrev = current_ > 0;
    bool hasNext = current_ + 1 < pageCount();
    RectF prevR(pn.X + S(3), pn.Y + S(3), S(24), pnH - S(6));
    if (hasPrev && hoverId_ == "page:prev") fillRoundRect(g, prevR, S(8), 0xFFFFFF, 105);
    drawIcon(g, Icon::ChevronLeft, RectF(prevR.X + S(5), prevR.Y + S(8), S(14), S(14)),
             hasPrev ? 0x5A6D63 : 0xB7C4BD, S(1.7f));
    if (hasPrev) addRegion("page:prev", prevR, L"上一页  (PageUp)", CursorKind::Hand);
    RectF nextR(pn.GetRight() - S(27), pn.Y + S(3), S(24), pnH - S(6));
    if (hasNext && hoverId_ == "page:next") fillRoundRect(g, nextR, S(8), 0xFFFFFF, 105);
    drawIcon(g, Icon::ChevronRight, RectF(nextR.X + S(5), nextR.Y + S(8), S(14), S(14)),
             hasNext ? 0x5A6D63 : 0xB7C4BD, S(1.7f));
    if (hasNext) addRegion("page:next", nextR, L"下一页  (PageDown)", CursorKind::Hand);
    RectF labelR(pn.X + S(27), pn.Y + S(3), pn.Width - S(54), pnH - S(6));
    if (hoverId_ == "page:label") fillRoundRect(g, labelR, S(8), 0xFFFFFF, 105);
    drawTextAligned(g, std::to_wstring(current_ + 1) + L" / " + std::to_wstring(pageCount()),
                    labelR, fCapsule_, 0x45584E, StringAlignmentCenter, StringAlignmentCenter, false);
    addRegion("page:label", labelR, L"页面管理", CursorKind::Hand);
    x += pnW;

    toolBtn("dock:open", Icon::Folder, false, true, L"打开  (Ctrl+O)");
    toolBtn("dock:save", Icon::Save, false, true, L"保存  (Ctrl+S)");
    toolBtn("dock:export", Icon::Export, false, true, L"导出 PNG  (Ctrl+E)");
    toolBtn("dock:settings", Icon::Settings, false, true, L"设置");
    toolBtn("dock:help", Icon::Help, false, true, L"快捷键与帮助  (F1)");
    toolBtn("dock:fullscreen", Icon::Fullscreen, false, true, L"全屏  (F11)");

    // zoom pill on the far right
    float zw = S(116);
    float zx = r.GetRight() - S(10) - zw;
    RectF zr(zx, rowY, zw, S(34));
    fillRoundRect(g, zr, S(12), 0xFFFFFF, 74);
    strokeRoundRect(g, zr, S(12), 0xFFFFFF, S(1), 92);
    auto zoomBtn = [&](const std::string& id, RectF br, Icon ic, const std::wstring& tip) {
        if (hoverId_ == id) fillRoundRect(g, br, S(7), 0xFFFFFF, 110);
        drawIcon(g, ic, RectF(br.X + (br.Width - S(13)) / 2, br.Y + (br.Height - S(13)) / 2, S(13), S(13)),
                 0x56695F, S(1.6f));
        addRegion(id, br, tip, CursorKind::Hand);
    };
    zoomBtn("zoom:out", RectF(zr.X + S(3), zr.Y + S(4), S(24), zr.Height - S(8)), Icon::Minus, L"缩小");
    RectF pctR(zr.X + S(28), zr.Y + S(3), S(42), zr.Height - S(6));
    if (hoverId_ == "zoom:reset") fillRoundRect(g, pctR, S(7), 0xFFFFFF, 110);
    drawTextAligned(g, std::to_wstring((int)std::round(zoom_ * 100)) + L"%", pctR, fCapsule_, 0x45584E,
                    StringAlignmentCenter, StringAlignmentCenter, false);
    addRegion("zoom:reset", pctR, L"适应页面  (0)", CursorKind::Hand);
    zoomBtn("zoom:in", RectF(zr.X + S(72), zr.Y + S(4), S(24), zr.Height - S(8)), Icon::Plus, L"放大");
    zoomBtn("zoom:fit", RectF(zr.X + S(96), zr.Y + S(4), S(17), zr.Height - S(8)), Icon::Fit, L"适应页面");

    // contextual second row
    RectF cr(r.X + S(12), r.Y + S(62), r.Width - S(24), S(50));
    fillRoundRect(g, cr, S(18), 0xFFFFFF, 122);
    strokeRoundRect(g, cr, S(18), 0xFFFFFF, S(1), 120);
    GraphicsState ctxState = g.Save();
    g.TranslateTransform((1.0f - easeOutCubic(toolAnim_)) * S(10), 0.0f);
    drawDockContext(g, cr, tool_);
    g.Restore(ctxState);
}

void App::drawDockContext(Graphics& g, RectF r, Tool t) {
    float x = r.X + S(16);
    float cy = r.Y + r.Height / 2;
    auto label = [&](const wchar_t* s) {
        drawTextAligned(g, s, RectF(x, r.Y, S(38), r.Height), fCapsule_, 0x566A60,
                        StringAlignmentNear, StringAlignmentCenter, false);
        x += S(38);
    };
    auto divider = [&]() {
        Pen p(RGBA(0x5F746A, 42), S(1));
        g.DrawLine(&p, x + S(6), r.Y + S(11), x + S(6), r.GetBottom() - S(11));
        x += S(14);
    };
    auto custom = [&](const std::string& id) {
        RectF c(x, cy - S(11), S(22), S(22));
        fillRoundRect(g, c, S(11), 0xFFFFFF, 160);
        strokeRoundRect(g, c, S(11), 0x8DA199, S(1), 180);
        drawIcon(g, Icon::Plus, RectF(c.X + S(5), c.Y + S(5), S(12), S(12)), 0x4D6258, S(1.6f));
        addRegion(id, RectF(c.X - S(4), c.Y - S(4), c.Width + S(8), c.Height + S(8)),
                  L"自定义颜色", CursorKind::Hand);
        x += S(30);
    };
    auto iconMini = [&](const std::string& id, Icon ic, const std::wstring& tip, bool enabled) {
        RectF b(x, cy - S(16), S(34), S(32));
        if (hoverId_ == id && enabled) fillRoundRect(g, b, S(9), 0xFFFFFF, 150);
        drawIcon(g, ic, RectF(b.X + S(8), b.Y + S(7), S(18), S(18)),
                 enabled ? 0x46594F : 0xB8C4BD, S(1.6f));
        if (enabled) addRegion(id, b, tip, CursorKind::Hand);
        x += S(36);
    };

    std::wstring name = toolName(t);
    SolidBrush dot(RGBA(COL_ACCENT));
    g.FillEllipse(&dot, x + S(2), cy - S(3), S(6), S(6));
    drawTextAligned(g, name, RectF(x + S(12), r.Y, S(70), r.Height), fCapsule_, 0x40534A,
                    StringAlignmentNear, StringAlignmentCenter, false);
    x += S(88);

    const uint32_t inkColors[] = {COL_INK, 0xE5484D, 0xF08C32, 0x1AAE7B, 0x2F6BFF, 0x8B5CF6};
    const uint32_t highlighterColors[] = {0xFFD54A, 0x7BE495, 0xFF9EC4, 0x86C8FF};
    const uint32_t laserColors[] = {0xE5484D, 0xF08C32, 0x2F6BFF, 0x1AAE7B};
    const uint32_t paperColors[] = {0xFFFFFF, 0xFFF8EC, 0xEFFBF4, 0xF2F8FF, 0xFFF3F6, 0xF5F1FF};
    const wchar_t* patterns[] = {L"blank", L"grid", L"ruled", L"dots"};
    const wchar_t* patternNames[] = {L"空白", L"方格", L"横线", L"点阵"};

    if (t == Tool::Background) {
        label(L"颜色");
        for (uint32_t c : paperColors) {
            drawSwatch(g, "canvas:color:" + hexColor(c), x + S(11), cy, c, S(10),
                       page().paper == c, L"画布颜色");
            x += S(30);
        }
        custom("canvas:color_custom");
        divider();
        label(L"底纹");
        for (int i = 0; i < 4; ++i) {
            RectF pc(x, cy - S(15), S(56), S(30));
            bool sel = page().pattern == patterns[i];
            fillRoundRect(g, pc, S(8), 0xFFFFFF, sel ? 215 : 120);
            strokeRoundRect(g, pc, S(8), sel ? COL_ACCENT : 0xCDD8D1, sel ? S(1.6f) : S(1), 220);
            if (i == 1) {
                Pen p(RGBA(0x91A399, 110), S(0.8f));
                for (int k = 1; k < 4; ++k) g.DrawLine(&p, pc.X + pc.Width * k / 4, pc.Y + S(3), pc.X + pc.Width * k / 4, pc.GetBottom() - S(3));
                for (int k = 1; k < 3; ++k) g.DrawLine(&p, pc.X + S(3), pc.Y + pc.Height * k / 3, pc.GetRight() - S(3), pc.Y + pc.Height * k / 3);
            } else if (i == 2) {
                Pen p(RGBA(0x91A399, 110), S(0.8f));
                for (int k = 1; k < 4; ++k) g.DrawLine(&p, pc.X + S(3), pc.Y + pc.Height * k / 4, pc.GetRight() - S(3), pc.Y + pc.Height * k / 4);
            } else if (i == 3) {
                SolidBrush b(RGBA(0x8EA299, 120));
                for (int yy = 1; yy < 4; ++yy)
                    for (int xx = 1; xx < 7; ++xx)
                        g.FillEllipse(&b, pc.X + pc.Width * xx / 8 - S(1), pc.Y + pc.Height * yy / 5 - S(1), S(2), S(2));
            }
            addRegion("canvas:pattern:" + std::string(utf8(patterns[i])), pc, patternNames[i], CursorKind::Hand);
            x += S(60);
        }
        return;
    }

    if (t == Tool::Pen || t == Tool::Highlighter) {
        const uint32_t* cols = t == Tool::Pen ? inkColors : highlighterColors;
        int n = t == Tool::Pen ? 6 : 4;
        uint32_t cur = t == Tool::Pen ? penColor_ : highlighterColor_;
        label(L"颜色");
        for (int i = 0; i < n; ++i) {
            drawSwatch(g, "color:" + hexColor(cols[i]), x + S(11), cy, cols[i], S(10), cur == cols[i], L"颜色");
            x += S(30);
        }
        custom("customcolor");
        divider();
        label(L"粗细");
        float widths[] = {2, 4, 7, 12};
        if (t == Tool::Highlighter) { widths[0] = 14; widths[1] = 24; widths[2] = 36; widths[3] = 48; }
        float curW = t == Tool::Pen ? penWidth_ : highlighterWidth_;
        for (float w : widths) {
            drawWidthDot(g, idNum("width:", w), x + S(11), cy, w, std::fabs(curW - w) < 0.01f, L"粗细");
            x += S(28);
        }
        return;
    }

    if (t == Tool::Eraser) {
        label(L"大小");
        float widths[] = {16, 30, 48, 72};
        for (float w : widths) {
            float rr = S(4.0f) + (w / 72.0f) * S(9.0f);
            float cx = x + S(14);
            bool sel = std::fabs(eraserSize_ - w) < 0.01f;
            std::string id = idNum("width:", w);
            if (sel) {
                Pen ring(RGBA(COL_ACCENT), S(1.8f));
                g.DrawEllipse(&ring, cx - rr - S(3), cy - rr - S(3), (rr + S(3)) * 2, (rr + S(3)) * 2);
            } else if (hoverId_ == id) {
                Pen ring(RGBA(0xC9D2E2), S(1.2f));
                g.DrawEllipse(&ring, cx - rr - S(3), cy - rr - S(3), (rr + S(3)) * 2, (rr + S(3)) * 2);
            }
            SolidBrush brush(RGBA(0x3B4555, sel ? 235 : 190));
            g.FillEllipse(&brush, cx - rr, cy - rr, rr * 2, rr * 2);
            addRegion(id, RectF(cx - S(17), cy - S(20), S(34), S(40)),
                      L"橡皮大小 " + std::to_wstring((int)w), CursorKind::Hand);
            x += S(36);
        }
        divider();
        RectF b(x, cy - S(15), S(64), S(30));
        fillRoundRect(g, b, S(9), 0xFFFFFF, 140);
        strokeRoundRect(g, b, S(9), 0xD4DFD8, S(1), 180);
        drawTextAligned(g, L"清空本页", b, fCapsule_, 0x5B6E64, StringAlignmentCenter, StringAlignmentCenter, false);
        addRegion("clearpage", b, L"清空当前页", CursorKind::Hand);
        return;
    }

    if (t == Tool::Shape) {
        label(L"形状");
        const wchar_t* names[] = {L"线", L"箭", L"矩", L"圆"};
        for (int i = 0; i < 4; ++i) {
            RectF b(x, cy - S(15), S(40), S(30));
            bool sel = (int)shapeKind_ == i;
            fillRoundRect(g, b, S(8), 0xFFFFFF, sel ? 215 : 115);
            strokeRoundRect(g, b, S(8), sel ? COL_ACCENT : 0xCDD8D1, sel ? S(1.5f) : S(1), 220);
            drawTextAligned(g, names[i], b, fCapsule_, sel ? COL_ACCENT : 0x52665C,
                            StringAlignmentCenter, StringAlignmentCenter, false);
            addRegion("shape:" + std::to_string(i), b, names[i], CursorKind::Hand);
            x += S(44);
        }
        RectF fb(x, cy - S(15), S(56), S(30));
        fillRoundRect(g, fb, S(8), shapeFilled_ ? COL_ACCENT_SOFT : 0xFFFFFF, shapeFilled_ ? 235 : 115);
        strokeRoundRect(g, fb, S(8), shapeFilled_ ? COL_ACCENT : 0xCDD8D1, S(1));
        drawTextAligned(g, L"填充", fb, fCapsule_, shapeFilled_ ? COL_ACCENT : 0x52665C,
                        StringAlignmentCenter, StringAlignmentCenter, false);
        addRegion("fill", fb, L"切换填充", CursorKind::Hand);
        x += S(62);
        divider();
        label(L"颜色");
        for (int i = 0; i < 5; ++i) {
            drawSwatch(g, "color:" + hexColor(inkColors[i]), x + S(10), cy, inkColors[i], S(9),
                       shapeColor_ == inkColors[i], L"形状颜色");
            x += S(28);
        }
        divider();
        label(L"粗细");
        float widths[] = {2, 4, 7, 10};
        for (float w : widths) {
            drawWidthDot(g, idNum("width:", w), x + S(10), cy, w, std::fabs(shapeWidth_ - w) < 0.01f, L"边线粗细");
            x += S(26);
        }
        return;
    }

    if (t == Tool::Text) {
        label(L"颜色");
        for (int i = 0; i < 5; ++i) {
            drawSwatch(g, "color:" + hexColor(inkColors[i]), x + S(11), cy, inkColors[i], S(10),
                       textColor_ == inkColors[i], L"文字颜色");
            x += S(30);
        }
        custom("customcolor");
        divider();
        label(L"字号");
        const float sizes[] = {24, 36, 48, 64};
        for (float s : sizes) {
            RectF b(x, cy - S(15), S(38), S(30));
            bool sel = std::fabs(textSize_ - s) < 0.01f;
            fillRoundRect(g, b, S(8), 0xFFFFFF, sel ? 215 : 108);
            strokeRoundRect(g, b, S(8), sel ? COL_ACCENT : 0xCDD8D1, sel ? S(1.5f) : S(1), 220);
            drawTextAligned(g, std::to_wstring((int)s), b, fCapsule_, sel ? COL_ACCENT : 0x52665C,
                            StringAlignmentCenter, StringAlignmentCenter, false);
            addRegion(idNum("textsize:", s), b, L"字号", CursorKind::Hand);
            x += S(42);
        }
        return;
    }

    if (t == Tool::Laser) {
        label(L"颜色");
        for (uint32_t c : laserColors) {
            drawSwatch(g, "color:" + hexColor(c), x + S(11), cy, c, S(10), laserColor_ == c, L"激光颜色");
            x += S(30);
        }
        divider();
        drawTextAligned(g, L"按住拖动，松手渐隐", RectF(x, r.Y, S(200), r.Height), fCapsule_, 0x566A60,
                        StringAlignmentNear, StringAlignmentCenter, false);
        return;
    }

    if (t == Tool::Select) {
        label(L"选中对象");
        iconMini("sel:copy", Icon::Copy, L"复制选中  (Ctrl+D)", sel_ >= 0);
        iconMini("sel:delete", Icon::Trash, L"删除选中  (Delete)", sel_ >= 0);
        iconMini("sel:front", Icon::Front, L"移到最前", sel_ >= 0);
        iconMini("sel:back", Icon::Back, L"移到最后", sel_ >= 0);
        if (sel_ < 0) {
            drawTextAligned(g, L"点击页面上的对象进行选择", RectF(x + S(8), r.Y, S(300), r.Height),
                            fCapsule_, 0x6B7E74, StringAlignmentNear, StringAlignmentCenter, false);
        }
        return;
    }

    if (t == Tool::Hand) {
        drawTextAligned(g, L"拖动画布  ·  Ctrl + 滚轮缩放  ·  0 适应页面  ·  空格临时移动",
                        RectF(x, r.Y, r.GetRight() - x - S(18), r.Height), fCapsule_, 0x566A60,
                        StringAlignmentNear, StringAlignmentCenter, false);
        return;
    }
}

// ---------------------------------------------------------------------------
// settings
// ---------------------------------------------------------------------------

void App::drawSettings(Graphics& g) {
    const uint32_t paperColors[] = {0xFFFFFF, 0xFFF8EC, 0xEFFBF4, 0xF2F8FF, 0xFFF3F6, 0xF5F1FF};
    addRegion("ov:scrim", clientRect(), L"关闭设置", CursorKind::Arrow);
    float anim = std::max(0.0f, std::min(1.0f, settingsAnim_));
    float animEase = easeOutCubic(anim);
    SolidBrush scrim(RGBA(0x0A1510, (BYTE)(96 * animEase)));
    g.FillRectangle(&scrim, clientRect());

    RectF r = settingsRect();
    addRegion("ov:panel", r, L"", CursorKind::Arrow);
    GraphicsState slideState = g.Save();
    g.TranslateTransform(0.0f, (1.0f - animEase) * S(20));
    drawGlass(g, r, S(28), 244);

    float x = r.X + S(28);
    float right = r.GetRight() - S(28);
    drawTextAligned(g, L"设置", RectF(x, r.Y + S(18), S(260), S(40)), fBig_, COL_INK,
                    StringAlignmentNear, StringAlignmentCenter, false);
    drawTextAligned(g, L"HAVEGET白板", RectF(x, r.Y + S(48), S(260), S(18)), fMini_, 0x6B7E74,
                    StringAlignmentNear, StringAlignmentCenter, false);

    RectF closeR(right - S(32), r.Y + S(20), S(32), S(32));
    if (hoverId_ == "settings:close") fillRoundRect(g, closeR, S(9), 0xFFFFFF, 160);
    drawIcon(g, Icon::Close, RectF(closeR.X + S(8), closeR.Y + S(8), S(16), S(16)), 0x596C62, S(1.6f));
    addRegion("settings:close", closeR, L"关闭", CursorKind::Hand);

    auto section = [&](const wchar_t* text, float y) {
        drawTextAligned(g, text, RectF(x, r.Y + y, S(240), S(22)), fSection_, 0x31443B,
                        StringAlignmentNear, StringAlignmentCenter, false);
    };
    auto button = [&](const std::string& id, RectF br, const std::wstring& text, bool primary, bool enabled) {
        bool hov = enabled && hoverId_ == id;
        if (primary) fillRoundRect(g, br, S(11), hov ? 0x0F8F6B : COL_ACCENT, 245);
        else {
            fillRoundRect(g, br, S(11), 0xFFFFFF, hov ? 175 : 110);
            strokeRoundRect(g, br, S(11), 0xD2DDD6, S(1), 210);
        }
        drawTextAligned(g, text, br, fCapsule_, primary ? 0xFFFFFF : (enabled ? 0x4B5F55 : 0xB6C2BB),
                        StringAlignmentCenter, StringAlignmentCenter, false);
        if (enabled) addRegion(id, br, text, CursorKind::Hand);
    };
    auto toggle = [&](const std::string& id, RectF tr, bool on, const std::wstring& text) {
        RectF pill(tr.X, tr.Y, S(42), S(24));
        fillRoundRect(g, pill, S(12), on ? COL_ACCENT : 0xD8E2DC, 245);
        SolidBrush knob(RGBA(0xFFFFFF));
        float kx = on ? pill.GetRight() - S(20) : pill.X + S(3);
        g.FillEllipse(&knob, kx, pill.Y + S(3), S(18), S(18));
        drawTextAligned(g, text, RectF(pill.GetRight() + S(9), tr.Y, S(260), S(24)), fCapsule_,
                        on ? 0x3F544A : 0x6F8278, StringAlignmentNear, StringAlignmentCenter, false);
        addRegion(id, RectF(tr.X, tr.Y - S(3), S(330), S(30)), text, CursorKind::Hand);
    };

    // toolbar position
    section(L"外观", 80);
    button("settings:dock_reset", RectF(right - S(132), r.Y + S(74), S(132), S(32)),
           L"重置到底部", false, true);
    drawTextAligned(g, L"拖动底部工具栏左侧的圆点，可自由移动到任意位置",
                    RectF(x, r.Y + S(116), r.Width - S(56), S(22)), fCapsule_, 0x5C6F65,
                    StringAlignmentNear, StringAlignmentCenter, false);

    // canvas colors
    section(L"画布颜色", 150);
    for (int i = 0; i < 6; ++i) {
        drawSwatch(g, "settings:canvas_color:" + hexColor(paperColors[i]), x + S(15) + S(32) * i,
                   r.Y + S(188), paperColors[i], S(11), page().paper == paperColors[i], L"画布颜色");
    }
    RectF customR(x + S(222), r.Y + S(177), S(76), S(24));
    fillRoundRect(g, customR, S(12), 0xFFFFFF, 120);
    strokeRoundRect(g, customR, S(12), 0xD2DDD6, S(1), 200);
    drawTextAligned(g, L"自定义…", customR, fCapsule_, 0x5B6E64,
                    StringAlignmentCenter, StringAlignmentCenter, false);
    addRegion("settings:canvas_custom", customR, L"选择任意颜色", CursorKind::Hand);

    toggle("settings:apply_all", RectF(x, r.Y + S(218), S(330), S(26)), applyAllPages_, L"应用到全部页面");

    Pen line(RGBA(0x5F746A, 38), S(1));
    g.DrawLine(&line, x, r.Y + S(266), right, r.Y + S(266));

    if (devMode_ && devUnlocked_) {
        section(L"开发者选项", 286);
        RectF pill(right - S(82), r.Y + S(282), S(82), S(24));
        fillRoundRect(g, pill, S(12), COL_ACCENT_SOFT, 235);
        drawTextAligned(g, L"已启用", pill, fCapsule_, COL_ACCENT, StringAlignmentCenter,
                        StringAlignmentCenter, false);
        drawTextAligned(g, L"自定义启动器背景、左上角 Logo 和名称",
                        RectF(x, r.Y + S(316), r.Width - S(56), S(20)), fCapsule_, 0x5C6F65,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, L"开发者名单：田浚哲 · 吴晨曦",
                        RectF(x, r.Y + S(338), r.Width - S(56), S(18)), fCapsule_, 0x45594F,
                        StringAlignmentNear, StringAlignmentCenter, false);

        RectF nr = settingsNameRect();
        drawTextAligned(g, L"应用名称", RectF(nr.X, nr.Y - S(22), S(160), S(18)), fTiny_, 0x566A60,
                        StringAlignmentNear, StringAlignmentCenter, false);
        fillRoundRect(g, nr, S(10), 0xFFFFFF, 150);
        strokeRoundRect(g, nr, S(10), hNameEdit_ ? COL_ACCENT : 0xD2DDD6, S(1.4f), 220);
        if (!hNameEdit_) {
            drawTextAligned(g, appName_, RectF(nr.X + S(10), nr.Y, nr.Width - S(18), nr.Height),
                            fUI_, 0x34483E, StringAlignmentNear, StringAlignmentCenter, true);
        }
        addRegion("settings:name_edit", nr, L"修改应用名称", CursorKind::IBeam);
        RectF nameBtn(nr.GetRight() + S(8), nr.Y, r.GetRight() - S(28) - (nr.GetRight() + S(8)), nr.Height);
        button(hNameEdit_ ? "settings:name_save" : "settings:name_edit", nameBtn,
               hNameEdit_ ? L"保存" : L"修改", hNameEdit_, true);

        auto assetRow = [&](float y, const wchar_t* title, const wchar_t* status,
                            const std::string& chooseId, const std::string& clearId, bool hasImage) {
            drawTextAligned(g, title, RectF(x, r.Y + y, S(116), S(30)), fCapsule_, 0x45594F,
                            StringAlignmentNear, StringAlignmentCenter, false);
            drawTextAligned(g, status, RectF(x + S(112), r.Y + y, S(212), S(30)), fCapsule_,
                            hasImage ? COL_ACCENT : 0x6B7E74, StringAlignmentNear,
                            StringAlignmentCenter, true);
            RectF clearB(right - S(64), r.Y + y, S(64), S(30));
            button(clearId, clearB, L"恢复", false, hasImage);
            RectF chooseB(clearB.X - S(8) - S(94), r.Y + y, S(94), S(30));
            button(chooseId, chooseB, L"选择图片", false, true);
        };
        assetRow(430, L"启动器背景", launcherBg_ ? L"已自定义" : L"默认白色",
                 "settings:launcher_bg", "settings:launcher_bg_clear", (bool)launcherBg_);
        assetRow(482, L"左上角 Logo", customLogo_ ? L"已自定义" : L"默认",
                 "settings:logo", "settings:logo_clear", (bool)customLogo_);

        button("settings:reset_look", RectF(x, r.Y + S(536), S(124), S(30)), L"恢复默认外观", false, true);
        button("settings:dev_off", RectF(right - S(126), r.Y + S(536), S(126), S(30)),
               L"关闭开发者模式", false, true);
    } else {
        section(L"关于", 286);
        drawTextAligned(g, appName_, RectF(x, r.Y + S(322), S(360), S(30)), fSection_, 0x31443B,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, L"为灵感而生 · 本地保存 · 无需联网", RectF(x, r.Y + S(354), S(420), S(22)),
                        fCapsule_, 0x5C6F65, StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, L"空格 + 拖动  移动画布        Ctrl + 滚轮  缩放",
                        RectF(x, r.Y + S(392), S(460), S(22)), fTiny_, 0x6B7E74,
                        StringAlignmentNear, StringAlignmentCenter, false);
        drawTextAligned(g, L"0  适应页面        Ctrl+S  保存        F1  快捷键",
                        RectF(x, r.Y + S(418), S(460), S(22)), fTiny_, 0x6B7E74,
                        StringAlignmentNear, StringAlignmentCenter, false);
    }

    float vy = r.GetBottom() - S(48);
    drawTextAligned(g, std::wstring(APP_NAME) + L"   " + APP_VERSION,
                    RectF(x, vy, S(350), S(24)), fMini_, 0x77887F,
                    StringAlignmentNear, StringAlignmentCenter, false);
    addRegion("settings:version", RectF(x - S(4), vy - S(5), S(360), S(34)), L"版本信息", CursorKind::Arrow);

    drawNameEditor(g);
    g.Restore(slideState);
}

void App::drawNameEditor(Graphics& g) {
    if (!hNameEdit_) return;
    RectF nr = settingsNameRect();
    strokeRoundRect(g, nr, S(10), COL_ACCENT, S(1.6f), 220);
}

}  // namespace qhb
