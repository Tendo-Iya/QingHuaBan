from pathlib import Path

# app: timer resolution and live quality
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\main.cpp")
s = p.read_text(encoding="utf-8")
if "#include <mmsystem.h>" not in s:
    s = s.replace("#include <objbase.h>", "#include <objbase.h>\n#include <mmsystem.h>")
s = s.replace("""int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
    SetCurrentProcessExplicitAppUserModelID(L"HAVEGET.Whiteboard.1");""",
"""int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
    timeBeginPeriod(1);
    SetCurrentProcessExplicitAppUserModelID(L"HAVEGET.Whiteboard.1");""")
s = s.replace("""    GdiplusShutdown(token);
    CoUninitialize();
    return 0;
}""",
"""    GdiplusShutdown(token);
    CoUninitialize();
    timeEndPeriod(1);
    return 0;
}""")
p.write_text(s, encoding="utf-8", newline="\n")

# build link winmm
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\build.ps1")
s = p.read_text(encoding="utf-8-sig")
s = s.replace("-lmsimg32 -lwinpthread", "-lmsimg32 -lwinpthread -lwinmm")
p.write_text(s, encoding="utf-8-sig", newline="\n")

# live paint speed + skip base copy during animations + splash veil
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""        Graphics g(back_.get());
        setupGraphics(g);
        blitBase(g, base_.get(), W_, H_);
        drawLive(g);
        drawToast(g);""",
"""        Graphics g(back_.get());
        setupGraphics(g);
        blitBase(g, base_.get(), W_, H_);
        g.SetCompositingQuality(CompositingQualityHighSpeed);
        g.SetInterpolationMode(InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(PixelOffsetModeHalf);
        drawLive(g);
        drawToast(g);""")
s = s.replace("""        if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_)
            base_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
        Graphics bg(base_.get());
        setupGraphics(bg);
        blitBase(bg, back_.get(), W_, H_);
        baseDirty_ = false;
    }""",
"""        ULONGLONG animNow = GetTickCount64();
        bool animating = splash_ || splashLeaving_ || overlay_ == 3 ||
                         (boardIntroStart_ != 0 && animNow - boardIntroStart_ < 260) ||
                         (pageFadeStart_ != 0 && animNow - pageFadeStart_ < 220) ||
                         (toastAnimStart_ != 0 && animNow - toastAnimStart_ < 180);
        if (!animating) {
            if (!base_ || base_->GetWidth() != W_ || base_->GetHeight() != H_)
                base_ = std::make_unique<Bitmap>(W_, H_, PixelFormat32bppPARGB);
            Graphics bg(base_.get());
            setupGraphics(bg);
            blitBase(bg, back_.get(), W_, H_);
            baseDirty_ = false;
        } else {
            baseDirty_ = true;
        }
    }""")
p.write_text(s, encoding="utf-8", newline="\n")

# splash intro veil less blank
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\ui.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("fillRoundRect(g, card, S(30), 0xFFFFFF, (BYTE)(235 * (1.0f - intro)));",
              "fillRoundRect(g, card, S(30), 0xFFFFFF, (BYTE)(80 * (1.0f - intro)));")
p.write_text(s, encoding="utf-8", newline="\n")

# invalidateLive threshold 10ms
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("if (now - lastLivePaint_ >= 8) {", "if (now - lastLivePaint_ >= 10) {")
p.write_text(s, encoding="utf-8", newline="\n")
print("smoothness tuning patched")
