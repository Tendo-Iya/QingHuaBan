from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
old = """    } else if (id == 4) {
        if (splash_) {
            if (GetTickCount64() - splashStart_ > 2400) enterBoard();
            else invalidate();
        } else {
            KillTimer(hwnd_, 4);
        }
    }
}"""
new = """    } else if (id == 4) {
        ULONGLONG now = GetTickCount64();
        if (splash_) {
            if (splashLeaving_) {
                if (now - splashLeaveStart_ >= 240) {
                    splash_ = false;
                    splashLeaving_ = false;
                    KillTimer(hwnd_, 4);
                    fitView();
                    SetFocus(hwnd_);
                    boardIntroStart_ = now;
                    showToast(L"欢迎使用 " + appName_, false, 2000);
                    SetTimer(hwnd_, 5, 16, nullptr);
                    invalidate();
                } else {
                    invalidate();
                }
            } else if (now - splashStart_ > 2400) {
                enterBoard();
            } else {
                invalidate();
            }
        } else {
            KillTimer(hwnd_, 4);
        }
    } else if (id == 5) {
        ULONGLONG now = GetTickCount64();
        bool had = false;
        if (overlay_ == 3 && settingsAnimStart_ != 0) {
            had = true;
            float t = std::min(1.0f, (float)(now - settingsAnimStart_) / 180.0f);
            float e = easeOutCubic(t);
            settingsAnim_ = settingsClosing_ ? (1.0f - e) : e;
            if (t >= 1.0f) {
                settingsAnimStart_ = 0;
                if (settingsClosing_) {
                    settingsClosing_ = false;
                    overlay_ = 0;
                }
                settingsAnim_ = settingsClosing_ ? 0.0f : 1.0f;
            }
        }
        if (pageFadeStart_ != 0) {
            if (now - pageFadeStart_ < 220) had = true;
            else pageFadeStart_ = 0;
        }
        if (boardIntroStart_ != 0) {
            if (now - boardIntroStart_ < 260) had = true;
            else boardIntroStart_ = 0;
        }
        if (toastAnimStart_ != 0 && now - toastAnimStart_ < 180) had = true;
        if (had) invalidate();
        else KillTimer(hwnd_, 5);
    }
}"""
if old not in s: raise SystemExit("onTimer anchor missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("animation timer added")
