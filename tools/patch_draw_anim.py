from pathlib import Path

# render paint/page/toast
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    Graphics dst(hdc);
    dst.DrawImage(back_.get(), 0, 0, W_, H_);
}""",
"""    {
        Graphics ov(back_.get());
        setupGraphics(ov);
        ULONGLONG now = GetTickCount64();
        if (boardIntroStart_ != 0 && now - boardIntroStart_ < 260) {
            float t = easeOutCubic((float)(now - boardIntroStart_) / 260.0f);
            SolidBrush veil(RGBA(0xFFFFFF, (BYTE)(190 * (1.0f - t))));
            ov.FillRectangle(&veil, 0, 0, (float)W_, (float)H_);
        }
        if (pageFadeStart_ != 0 && now - pageFadeStart_ < 220) {
            float t = easeOutCubic((float)(now - pageFadeStart_) / 220.0f);
            SolidBrush veil(RGBA(0xFFFFFF, (BYTE)(105 * (1.0f - t))));
            ov.FillRectangle(&veil, 0, 0, (float)W_, (float)H_);
        }
    }
    Graphics dst(hdc);
    dst.DrawImage(back_.get(), 0, 0, W_, H_);
}""")
s = s.replace("""    float x = ((float)W_ - w) / 2;
    float y = headerH() + S(14);
    RectF r(x, y, w, h);""",
"""    float x = ((float)W_ - w) / 2;
    float slide = 0.0f;
    if (toastAnimStart_ != 0) {
        float t = std::min(1.0f, (float)(GetTickCount64() - toastAnimStart_) / 160.0f);
        slide = S(10) * (1.0f - easeOutCubic(t));
    }
    float y = headerH() + S(14) - slide;
    RectF r(x, y, w, h);""")
p.write_text(s, encoding="utf-8", newline="\n")

# ui splash exit / settings slide
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    g.Restore(st);
    strokeRoundRect(g, card, S(30), 0xFFFFFF, S(1.2f), 170);

    drawBrand(g, true);
}""",
"""    g.Restore(st);
    strokeRoundRect(g, card, S(30), 0xFFFFFF, S(1.2f), 170);

    drawBrand(g, true);

    ULONGLONG now = GetTickCount64();
    float intro = easeOutCubic((float)(now - splashStart_) / 430.0f);
    if (intro < 1.0f) {
        fillRoundRect(g, card, S(30), 0xFFFFFF, (BYTE)(235 * (1.0f - intro)));
    }
    if (splashLeaving_) {
        float t = easeOutCubic((float)(now - splashLeaveStart_) / 240.0f);
        fillRoundRect(g, card, S(30), 0xFFFFFF, (BYTE)(255 * t));
    }
}""")
s = s.replace("""    addRegion("ov:scrim", clientRect(), L"关闭设置", CursorKind::Arrow);
    SolidBrush scrim(RGBA(0x0A1510, 96));
    g.FillRectangle(&scrim, clientRect());

    RectF r = settingsRect();
    addRegion("ov:panel", r, L"", CursorKind::Arrow);
    drawGlass(g, r, S(28), 244);""",
"""    addRegion("ov:scrim", clientRect(), L"关闭设置", CursorKind::Arrow);
    float anim = std::max(0.0f, std::min(1.0f, settingsAnim_));
    float animEase = easeOutCubic(anim);
    SolidBrush scrim(RGBA(0x0A1510, (BYTE)(96 * animEase)));
    g.FillRectangle(&scrim, clientRect());

    RectF r = settingsRect();
    addRegion("ov:panel", r, L"", CursorKind::Arrow);
    GraphicsState slideState = g.Save();
    g.TranslateTransform(0.0f, (1.0f - animEase) * S(20));
    drawGlass(g, r, S(28), 244);""")
s = s.replace("""    drawNameEditor(g);
}""",
"""    drawNameEditor(g);
    g.Restore(slideState);
}""")
# settings version string
s = s.replace('drawTextAligned(g, std::wstring(APP_NAME) + L"   1.0.0  (Build 1001)",',
              'drawTextAligned(g, std::wstring(APP_NAME) + L"   " + APP_VERSION,')
p.write_text(s, encoding="utf-8", newline="\n")
print("draw animations patched")
