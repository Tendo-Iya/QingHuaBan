from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\liquid.cpp")
s = p.read_text(encoding="utf-8")
old = """void App::enterBoard() {
    if (!splash_) return;
    splash_ = false;
    KillTimer(hwnd_, 4);
    fitView();
    SetFocus(hwnd_);
    showToast(L"欢迎使用 " + appName_, false, 2000);
    invalidate();
}"""
new = """void App::enterBoard() {
    if (!splash_ || splashLeaving_) return;
    splashLeaving_ = true;
    splashLeaveStart_ = GetTickCount64();
    SetTimer(hwnd_, 4, 16, nullptr);
}"""
if old not in s: raise SystemExit("enterBoard anchor missing")
s = s.replace(old, new)
p.write_text(s, encoding="utf-8", newline="\n")
print("enterBoard transition patched")
