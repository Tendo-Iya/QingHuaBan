from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# Lifecycle / config / splash
s = s.replace("""    fitView();
    updateWindowTitle();
    editBrush_ = CreateSolidBrush(RGB(255, 255, 255));
    SetTimer(hwnd_, 2, 400, nullptr);
    KillTimer(hwnd_, 2);
    showToast(L"画笔已就绪 · 直接在页面上书写", false, 3000);
}""",
"""    loadConfig();
    fitView();
    updateWindowTitle();
    editBrush_ = CreateSolidBrush(RGB(255, 255, 255));
    splash_ = true;
    splashStart_ = GetTickCount64();
    SetTimer(hwnd_, 4, 33, nullptr);
}""")

s = s.replace("""    if (hEdit_) {
        DestroyWindow(hEdit_);
        hEdit_ = nullptr;
    }
}""",
"""    if (hEdit_) {
        DestroyWindow(hEdit_);
        hEdit_ = nullptr;
    }
    if (hNameEdit_) {
        DestroyWindow(hNameEdit_);
        hNameEdit_ = nullptr;
    }
}""")

# Fonts
s = s.replace("""    fToast_ = mk(11.5f);
    fTextHint_ = mk(10.2f);
}""",
"""    fToast_ = mk(11.5f);
    fTextHint_ = mk(10.2f);
    fHero_ = mk(58, true);
    fSplashSub_ = mk(13.5f);
    fSection_ = mk(15.5f, true);
    fTiny_ = mk(9.2f);
    fCapsule_ = mk(11.0f);
}""")

# Window title
s = s.replace("    std::wstring t = std::wstring(APP_NAME) + L\" — \" + title_;",
              "    std::wstring t = (appName_.empty() ? std::wstring(APP_NAME) : appName_) + L\" — \" + title_;")

# Camera margins for the dock
s = s.replace("""    float top = headerH();
    float availW = (float)W_ - S(120) - S(56);
    float availH = (float)H_ - top - S(36) - S(128);
""",
"""    float top = headerH();
    float availW = (float)W_ - S(112) - S(50);
    float availH = (float)H_ - top - S(28) - S(178);
""")
s = s.replace("""    float top = headerH();
    float left = S(120);
    float right = S(56);
    float mtop = S(36), mbottom = S(128);
""",
"""    float top = headerH();
    float left = S(112);
    float right = S(50);
    float mtop = S(28), mbottom = S(178);
""")

# File extensions
s = s.replace('std::wstring def = (title_.empty() ? L"未命名白板" : title_) + L".qhb";',
              'std::wstring def = (title_.empty() ? L"未命名白板" : title_) + L".hgwb";')
s = s.replace('L"轻画板文件 (*.qhb)\\0*.qhb\\0所有文件 (*.*)\\0*.*\\0\\0",\n            L"qhb", def.c_str());',
              'L"HAVEGET白板文件 (*.hgwb)\\0*.hgwb\\0旧版轻画板文件 (*.qhb)\\0*.qhb\\0所有文件 (*.*)\\0*.*\\0\\0",\n            L"hgwb", def.c_str());')
s = s.replace('L"轻画板文件 (*.qhb)\\0*.qhb\\0所有文件 (*.*)\\0*.*\\0\\0");',
              'L"HAVEGET白板文件 (*.hgwb)\\0*.hgwb\\0旧版轻画板文件 (*.qhb)\\0*.qhb\\0所有文件 (*.*)\\0*.*\\0\\0");')

# Tool background keyboard / cursor
s = s.replace("    if (vk == 'B') { bgPanel_ = !bgPanel_; overlay_ = 0; invalidate(); return true; }",
              "    if (vk == 'B') { setTool(Tool::Background); return true; }")
s = s.replace("        if (spaceDown_ || tool_ == Tool::Hand) cur = CursorKind::SizeAll;",
              "        if (spaceDown_ || tool_ == Tool::Hand) cur = CursorKind::SizeAll;")
s = s.replace("        else if (tool_ == Tool::Laser) cur = CursorKind::Hand;\n        else cur = CursorKind::Cross;",
              "        else if (tool_ == Tool::Laser) cur = CursorKind::Hand;\n        else if (tool_ == Tool::Background) cur = CursorKind::Arrow;\n        else cur = CursorKind::Cross;")

# Background tool in canvas mouse down
s = s.replace("""        case Tool::Laser: startLaser(sp); break;
        case Tool::Hand: startPan(sp); break;
    }""",
"""        case Tool::Laser: startLaser(sp); break;
        case Tool::Background: break;
        case Tool::Hand: startPan(sp); break;
    }""")

# Splash timer in onTimer
s = s.replace("""    } else if (id == 3) {
        ULONGLONG now = GetTickCount64();
        while (!laser_.empty() && now - laser_.front().t > 850) laser_.erase(laser_.begin());
        if (laser_.empty() && !laserDown_) {
            KillTimer(hwnd_, 3);
        }
        invalidate();
    }""",
"""    } else if (id == 3) {
        ULONGLONG now = GetTickCount64();
        while (!laser_.empty() && now - laser_.front().t > 850) laser_.erase(laser_.begin());
        if (laser_.empty() && !laserDown_) {
            KillTimer(hwnd_, 3);
        }
        invalidate();
    } else if (id == 4) {
        if (splash_) {
            if (GetTickCount64() - splashStart_ > 2400) enterBoard();
            else invalidate();
        } else {
            KillTimer(hwnd_, 4);
        }
    }""")

# Open dropped files: accept hgwb and qhb
s = s.replace('std::wstring ext = lowerExt(files[0]);\n    if (ext == L".qhb") {',
              'std::wstring ext = lowerExt(files[0]);\n    if (ext == L".hgwb" || ext == L".qhb") {')

# Commit name on close
s = s.replace("""void App::onCloseRequest() {
    if (confirmDiscard()) DestroyWindow(hwnd_);
}""",
"""void App::onCloseRequest() {
    commitAppName();
    saveConfig();
    if (confirmDiscard()) DestroyWindow(hwnd_);
}""")

p.write_text(s, encoding="utf-8", newline="\n")
print("app core patched")
