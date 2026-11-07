from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
anchor = "void App::drawHelpOverlay(Graphics& g) {"
impl = r'''void App::drawDevPasswordDialog(Graphics& g) {
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

'''
s = s.replace(anchor, impl + anchor, 1)
p.write_text(s, encoding="utf-8", newline="\n")
print("developer password dialog drawn")
