from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
start = s.index("void App::drawPagesOverlay(Graphics& g) {")
end = s.index("void App::drawHelpOverlay(Graphics& g) {", start)
new = r'''void App::drawPagesOverlay(Graphics& g) {
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

'''
s = s[:start] + new + s[end:]
p.write_text(s, encoding="utf-8", newline="\n")
print("page manager content bitmap + clipped cards")
