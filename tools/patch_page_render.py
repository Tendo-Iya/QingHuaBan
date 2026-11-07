from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")

# fast blit helpers replace blitBase
s = s.replace("""static void blitBase(Graphics& g, Bitmap* base, int w, int h) {
    g.SetCompositingMode(CompositingModeSourceCopy);
    g.SetInterpolationMode(InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);
    g.DrawImage(base, RectF(0, 0, (float)w, (float)h));
    g.SetCompositingMode(CompositingModeSourceOver);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
}""",
"""static bool fastBlit(Graphics& dst, Bitmap* src, int w, int h) {
    if (!src || src->GetWidth() != w || src->GetHeight() != h) return false;
    dst.Flush(FlushIntentionSync);
    Graphics sg(src);
    HDC shdc = sg.GetHDC();
    HDC dhdc = dst.GetHDC();
    bool ok = false;
    if (shdc && dhdc) ok = BitBlt(dhdc, 0, 0, w, h, shdc, 0, 0, SRCCOPY) != FALSE;
    if (shdc) sg.ReleaseHDC(shdc);
    if (dhdc) dst.ReleaseHDC(dhdc);
    return ok;
}

static void blitBase(Graphics& g, Bitmap* base, int w, int h) {
    if (fastBlit(g, base, w, h)) return;
    g.SetCompositingMode(CompositingModeSourceCopy);
    g.SetInterpolationMode(InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);
    g.DrawImage(base, RectF(0, 0, (float)w, (float)h));
    g.SetCompositingMode(CompositingModeSourceOver);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
}""")

# presentation bitblt
s = s.replace("""    Graphics dst(hdc);
    dst.DrawImage(back_.get(), 0, 0, W_, H_);
}""",
"""    Graphics dst(back_.get());
    dst.Flush(FlushIntentionSync);
    HDC shdc = dst.GetHDC();
    if (shdc) {
        BitBlt(hdc, 0, 0, W_, H_, shdc, 0, 0, SRCCOPY);
        dst.ReleaseHDC(shdc);
    } else {
        Graphics out(hdc);
        out.DrawImage(back_.get(), 0, 0, W_, H_);
    }
}""")

# page manager header/auto-scroll/drop/scrollbar
s = s.replace('drawTextAligned(g, L"共 " + std::to_wstring(pageCount()) + L" 页  ·  点击页面切换",',
              'drawTextAligned(g, L"共 " + std::to_wstring(pageCount()) + L" 页  ·  拖动排序 · 滚轮翻页",')
s = s.replace("""    float cardH = S(184);
    int rows = (pageCount() + 2) / 3;
    float totalH = rows * (cardH + gap);
    float maxScroll = std::max(0.0f, totalH - ch);
    pageScroll_ = std::max(0.0f, std::min(pageScroll_, maxScroll));""",
"""    float cardH = S(184);
    int rows = (pageCount() + 2) / 3;
    float totalH = rows * (cardH + gap);
    float maxScroll = std::max(0.0f, totalH - ch);
    if (pageScrollToCurrent_) {
        int row = current_ / 3;
        pageScroll_ = std::max(0.0f, row * (cardH + gap) - ch * 0.35f);
        pageScrollToCurrent_ = false;
    }
    pageScroll_ = std::max(0.0f, std::min(pageScroll_, maxScroll));""")
s = s.replace("""        fillRoundRect(g, card, S(12), hover ? 0xF7F9FD : COL_WHITE);
        strokeRoundRect(g, card, S(12), current ? COL_ACCENT : 0xE2E7EF, current ? S(2.0f) : S(1));""",
"""        fillRoundRect(g, card, S(12), hover ? 0xF7F9FD : COL_WHITE);
        if (pageDragActive_ && i == pageDragFrom_)
            fillRoundRect(g, card, S(12), COL_ACCENT_SOFT, 150);
        strokeRoundRect(g, card, S(12), current ? COL_ACCENT : 0xE2E7EF, current ? S(2.0f) : S(1));
        if (pageDragActive_ && i == pageDragTo_ && i != pageDragFrom_) {
            fillRoundRect(g, card, S(12), COL_ACCENT_SOFT, 110);
            strokeRoundRect(g, card, S(12), COL_ACCENT, S(3.0f), 235);
        }""")
s = s.replace('    addRegion(id, card, L"切换到第 " + std::to_wstring(i + 1) + L" 页", CursorKind::Hand);',
              '    addRegion(id, card, L"第 " + std::to_wstring(i + 1) + L" 页 · 点击切换 / 拖动排序", CursorKind::Hand);')
s = s.replace("""    if (maxScroll > 0) {
        float trackY = cy0 + S(6), trackH = ch - S(12);
        fillRoundRect(g, RectF(px + pw - S(12), trackY, S(4), trackH), S(2), 0xEEF1F6);
        float thumbH2 = std::max(S(30), trackH * ch / totalH);
        float thumbY = trackY + (trackH - thumbH2) * (pageScroll_ / maxScroll);
        fillRoundRect(g, RectF(px + pw - S(12), thumbY, S(4), thumbH2), S(2), 0xC9D3E2);
    }""",
"""    if (maxScroll > 0) {
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
    }""")

# remove new document dock button
s = s.replace('    toolBtn("dock:new", Icon::Plus, false, true, L"新建白板  (Ctrl+N)");\n', '')

# selected tool pill animation
s = s.replace("""        if (selected) {
            fillRoundRect(g, br, S(10), COL_ACCENT_SOFT, 235);
            strokeRoundRect(g, br, S(10), 0xFFFFFF, S(1), 160);
        } else if (hov) {""",
"""        if (selected) {
            float te = easeOutCubic(toolAnim_);
            float inset = S(4) * (1.0f - te);
            RectF pill = br;
            pill.Inflate(-inset, -inset);
            fillRoundRect(g, pill, S(10), COL_ACCENT_SOFT, (BYTE)(235 * te));
            strokeRoundRect(g, pill, S(10), 0xFFFFFF, S(1), (BYTE)(160 * te));
        } else if (hov) {""")
s = s.replace("""        if (selected) {
            SolidBrush dot(RGBA(COL_ACCENT));
            g.FillEllipse(&dot, br.X + btn / 2 - S(2), br.GetBottom() - S(5), S(4), S(4));
        }""",
"""        if (selected) {
            SolidBrush dot(RGBA(COL_ACCENT, (BYTE)(255 * easeOutCubic(toolAnim_))));
            g.FillEllipse(&dot, br.X + btn / 2 - S(2), br.GetBottom() - S(5), S(4), S(4));
        }""")
# context row slide
s = s.replace("""    fillRoundRect(g, cr, S(18), 0xFFFFFF, 122);
    strokeRoundRect(g, cr, S(18), 0xFFFFFF, S(1), 120);
    drawDockContext(g, cr, tool_);""",
"""    fillRoundRect(g, cr, S(18), 0xFFFFFF, 122);
    strokeRoundRect(g, cr, S(18), 0xFFFFFF, S(1), 120);
    GraphicsState ctxState = g.Save();
    g.TranslateTransform((1.0f - easeOutCubic(toolAnim_)) * S(10), 0.0f);
    drawDockContext(g, cr, tool_);
    g.Restore(ctxState);""")

p.write_text(s, encoding="utf-8", newline="\n")
print("page drag/scroll/tool anim/BitBlt patched")
