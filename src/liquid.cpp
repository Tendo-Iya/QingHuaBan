#include "qhb.h"

#include <cstdio>
#include <sstream>

namespace qhb {

static float clampVal(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static const wchar_t* kImageFilter =
    L"图片文件 (*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp\0所有文件 (*.*)\0*.*\0\0";

static std::string readWholeFile(const std::wstring& path) {
    FILE* f = _wfopen(path.c_str(), L"rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return {}; }
    std::string out((size_t)n, '\0');
    size_t got = fread(out.data(), 1, (size_t)n, f);
    fclose(f);
    out.resize(got);
    return out;
}

static void writeWholeFile(const std::wstring& path, const std::string& data) {
    FILE* f = _wfopen(path.c_str(), L"wb");
    if (!f) return;
    if (!data.empty()) fwrite(data.data(), 1, data.size(), f);
    fclose(f);
}

static Bitmap* loadBitmapFile(const std::wstring& path) {
    if (path.empty() || !fileExists(path)) return nullptr;
    Bitmap* src = Bitmap::FromFile(path.c_str());
    if (!src || src->GetLastStatus() != Ok) {
        delete src;
        return nullptr;
    }
    Bitmap* copy = src->Clone(0, 0, src->GetWidth(), src->GetHeight(), PixelFormat32bppPARGB);
    delete src;
    return copy;
}

static std::wstring imageStorePath(const std::wstring& dir, const wchar_t* name) {
    return dir + L"\\" + name;
}

std::wstring App::appDataDir() const {
    wchar_t buf[MAX_PATH] = {};
    if (SHGetFolderPathW(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr,
                         SHGFP_TYPE_CURRENT, buf) == S_OK) {
        std::wstring d = buf;
        d += L"\\HAVEGET白板";
        CreateDirectoryW(d.c_str(), nullptr);
        return d;
    }
    wchar_t tmp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, tmp);
    std::wstring d = std::wstring(tmp) + L"HAVEGET白板";
    CreateDirectoryW(d.c_str(), nullptr);
    return d;
}

void App::loadConfig() {
    configDir_ = appDataDir();
    imageDir_ = configDir_ + L"\\Images";
    configPath_ = configDir_ + L"\\settings.ini";
    CreateDirectoryW(imageDir_.c_str(), nullptr);
    appName_ = APP_NAME;
    dockDefault_ = true;
    dockNormX_ = 0.5f;
    dockNormY_ = 0.90f;
    devMode_ = false;
    launcherBgFile_.clear();
    logoFile_.clear();

    std::string raw = readWholeFile(configPath_);
    std::istringstream iss(raw);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trimCopy(line.substr(0, eq));
        std::string val = trimCopy(line.substr(eq + 1));
        try {
            if (key == "dev") devMode_ = val == "1";
            else if (key == "app_name") {
                std::wstring n = wide(b64decode(val));
                if (!n.empty()) appName_ = n;
            } else if (key == "dock_default") dockDefault_ = val == "1";
            else if (key == "dock_x") dockNormX_ = std::stof(val);
            else if (key == "dock_y") dockNormY_ = std::stof(val);
            else if (key == "launcher_bg") launcherBgFile_ = wide(b64decode(val));
            else if (key == "logo") logoFile_ = wide(b64decode(val));
        } catch (...) {
            // Ignore malformed values and keep defaults.
        }
    }
    dockNormX_ = clampVal(dockNormX_, 0.0f, 1.0f);
    dockNormY_ = clampVal(dockNormY_, 0.0f, 1.0f);
    if (!launcherBgFile_.empty()) {
        launcherBg_.reset(loadBitmapFile(imageStorePath(imageDir_, launcherBgFile_.c_str())));
        if (!launcherBg_) launcherBgFile_.clear();
    }
    if (!logoFile_.empty()) {
        customLogo_.reset(loadBitmapFile(imageStorePath(imageDir_, logoFile_.c_str())));
        if (!customLogo_) logoFile_.clear();
    }
}

void App::saveConfig() {
    if (configDir_.empty()) {
        configDir_ = appDataDir();
        imageDir_ = configDir_ + L"\\Images";
        configPath_ = configDir_ + L"\\settings.ini";
    }
    CreateDirectoryW(configDir_.c_str(), nullptr);
    CreateDirectoryW(imageDir_.c_str(), nullptr);
    std::ostringstream os;
    os << "dev=" << (devMode_ ? 1 : 0) << "\n";
    os << "app_name=" << b64encode(utf8(appName_)) << "\n";
    os << "dock_default=" << (dockDefault_ ? 1 : 0) << "\n";
    os << "dock_x=" << dockNormX_ << "\n";
    os << "dock_y=" << dockNormY_ << "\n";
    os << "launcher_bg=" << b64encode(utf8(launcherBgFile_)) << "\n";
    os << "logo=" << b64encode(utf8(logoFile_)) << "\n";
    writeWholeFile(configPath_, os.str());
}

void App::enterBoard() {
    if (!splash_ || splashLeaving_) return;
    splashLeaving_ = true;
    splashLeaveStart_ = GetTickCount64();
    SetTimer(hwnd_, 4, 16, nullptr);
}

void App::startDockDrag(Pt screen) {
    dockDragBase_.reset();
    RectF r = dockRect();
    dockDragStart_ = screen;
    dockDragStartCenter_ = {r.X + r.Width / 2, r.Y + r.Height / 2};
    if (dockDefault_) {
        dockNormX_ = dockDragStartCenter_.x / std::max(1.0f, (float)W_);
        dockNormY_ = dockDragStartCenter_.y / std::max(1.0f, (float)H_);
        dockDefault_ = false;
    }
    drag_ = DragMode::Dock;
    SetCapture(hwnd_);
}

void App::resetDockPosition() {
    dockDefault_ = true;
    saveConfig();
    invalidate();
    showToast(L"工具栏已重置到底部", false, 1400);
}

RectF App::splashRect() const {
    float mx = S(20), my = S(16);
    float availW = std::max(100.0f, (float)W_ - mx * 2);
    float availH = std::max(100.0f, (float)H_ - my * 2);
    float w = availW;
    float h = w * 9.0f / 16.0f;
    if (h > availH) {
        h = availH;
        w = h * 16.0f / 9.0f;
    }
    return RectF(((float)W_ - w) / 2.0f, ((float)H_ - h) / 2.0f, w, h);
}

float App::dockWidth() const {
    return std::min((float)W_ - S(28), S(940));
}

float App::dockHeight() const {
    return S(124);
}

RectF App::dockRect() const {
    float w = dockWidth();
    float h = dockHeight();
    float cx = dockDefault_ ? (float)W_ * 0.5f : (float)W_ * dockNormX_;
    float cy = dockDefault_ ? (float)H_ - S(58) - h / 2 : (float)H_ * dockNormY_;
    float minX = w / 2 + S(12);
    float maxX = (float)W_ - w / 2 - S(12);
    if (minX > maxX) { minX = maxX = (float)W_ / 2; }
    cx = clampVal(cx, minX, maxX);
    float minY = h / 2 + S(12);
    float maxY = (float)H_ - h / 2 - S(50);
    if (minY > maxY) { minY = maxY = (float)H_ / 2; }
    cy = clampVal(cy, minY, maxY);
    return RectF(cx - w / 2, cy - h / 2, w, h);
}

RectF App::settingsRect() const {
    float w = S(620);
    float h = S(680);
    if (h > (float)H_ - S(28)) h = (float)H_ - S(28);
    if (w > (float)W_ - S(28)) w = (float)W_ - S(28);
    return RectF(((float)W_ - w) / 2, ((float)H_ - h) / 2, w, h);
}

RectF App::settingsNameRect() const {
    RectF r = settingsRect();
    float x = r.X + S(28);
    float y = r.Y + S(382);
    float w = r.Width - S(56) - S(92);
    return RectF(x, y, w, S(38));
}

void App::chooseLauncherBackground() {
    std::wstring p = chooseOpenPath(kImageFilter);
    if (p.empty()) return;
    Bitmap src(p.c_str());
    if (src.GetLastStatus() != Ok || src.GetWidth() <= 0) {
        showMessage(L"无法读取这张图片，请换一张 PNG/JPG/BMP 图片。", true);
        return;
    }
    std::wstring dest = imageStorePath(imageDir_, L"launcher_bg.png");
    if (!saveBitmapPng(&src, dest)) {
        showMessage(L"无法保存启动器背景图片。", true);
        return;
    }
    launcherBg_.reset(loadBitmapFile(dest));
    if (!launcherBg_) {
        showMessage(L"无法加载启动器背景图片。", true);
        return;
    }
    launcherBgFile_ = L"launcher_bg.png";
    saveConfig();
    invalidate();
    showToast(L"启动器背景已更新", false, 1500);
}

void App::chooseCustomLogo() {
    std::wstring p = chooseOpenPath(kImageFilter);
    if (p.empty()) return;
    Bitmap src(p.c_str());
    if (src.GetLastStatus() != Ok || src.GetWidth() <= 0) {
        showMessage(L"无法读取这张图片，请换一张 PNG/JPG/BMP 图片。", true);
        return;
    }
    std::wstring dest = imageStorePath(imageDir_, L"logo.png");
    if (!saveBitmapPng(&src, dest)) {
        showMessage(L"无法保存 Logo 图片。", true);
        return;
    }
    customLogo_.reset(loadBitmapFile(dest));
    if (!customLogo_) {
        showMessage(L"无法加载 Logo 图片。", true);
        return;
    }
    logoFile_ = L"logo.png";
    saveConfig();
    invalidate();
    showToast(L"左上角 Logo 已更新", false, 1500);
}

void App::clearLauncherBackground() {
    DeleteFileW(imageStorePath(imageDir_, L"launcher_bg.png").c_str());
    launcherBgFile_.clear();
    launcherBg_.reset();
    saveConfig();
    invalidate();
    showToast(L"启动器背景已恢复默认白色", false, 1500);
}

void App::clearCustomLogo() {
    DeleteFileW(imageStorePath(imageDir_, L"logo.png").c_str());
    logoFile_.clear();
    customLogo_.reset();
    saveConfig();
    invalidate();
    showToast(L"Logo 已恢复默认", false, 1400);
}

void App::applyCanvasColor(uint32_t color, bool allPages) {
    if (!allPages && page().paper == color) return;
    pushUndo();
    if (allPages) {
        for (auto& p : pages_) {
            p.paper = color;
            p.touch();
        }
    } else {
        page().paper = color;
        page().touch();
    }
    markDirty();
    invalidate();
}

// ---------------------------------------------------------------------------
// app name editor
// ---------------------------------------------------------------------------

static WNDPROC g_oldNameProc = nullptr;
static App* g_nameOwner = nullptr;

static LRESULT CALLBACK NameEditProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        if (wp == VK_RETURN) {
            if (g_nameOwner) g_nameOwner->finishAppNameEdit();
            return 0;
        }
        if (wp == VK_ESCAPE) {
            if (g_nameOwner) g_nameOwner->cancelAppNameEdit();
            return 0;
        }
    }
    if (msg == WM_KILLFOCUS && g_nameOwner) {
        PostMessageW(g_nameOwner->hwnd(), App::WM_APP_COMMIT_NAME, 0, 0);
    }
    return CallWindowProcW(g_oldNameProc, h, msg, wp, lp);
}

void App::beginAppNameEdit() {
    if (overlay_ != 3) return;
    createNameEdit();
}

void App::createNameEdit() {
    if (hNameEdit_) {
        DestroyWindow(hNameEdit_);
        hNameEdit_ = nullptr;
    }
    RectF r = settingsNameRect();
    hNameEdit_ = CreateWindowExW(0, L"EDIT", L"",
                                 WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER,
                                 (int)r.X, (int)r.Y, (int)r.Width, (int)r.Height,
                                 hwnd_, nullptr, inst_, nullptr);
    if (!hNameEdit_) return;
    HFONT hf = CreateFontW(-(int)std::round(S(14)), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    SendMessageW(hNameEdit_, WM_SETFONT, (WPARAM)hf, TRUE);
    SendMessageW(hNameEdit_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(7, 7));
    SetWindowTextW(hNameEdit_, appName_.c_str());
    g_oldNameProc = (WNDPROC)SetWindowLongPtrW(hNameEdit_, GWLP_WNDPROC, (LONG_PTR)NameEditProc);
    g_nameOwner = this;
    nameCommitting_ = false;
    SetFocus(hNameEdit_);
    SendMessageW(hNameEdit_, EM_SETSEL, 0, -1);
}

void App::layoutNameEdit() {
    if (!hNameEdit_) return;
    RectF r = settingsNameRect();
    SetWindowPos(hNameEdit_, HWND_TOP, (int)r.X, (int)r.Y, (int)r.Width, (int)r.Height,
                 SWP_NOACTIVATE);
}

void App::commitAppName() {
    if (!hNameEdit_ || nameCommitting_) return;
    nameCommitting_ = true;
    int len = GetWindowTextLengthW(hNameEdit_);
    std::wstring value;
    if (len > 0) {
        value.resize((size_t)len + 1);
        GetWindowTextW(hNameEdit_, value.data(), len + 1);
        value.resize((size_t)len);
    }
    HWND edit = hNameEdit_;
    hNameEdit_ = nullptr;
    g_nameOwner = nullptr;
    DestroyWindow(edit);
    std::wstring name = trimW(value);
    if (!name.empty()) {
        appName_ = name;
        updateWindowTitle();
        saveConfig();
    }
    nameCommitting_ = false;
    SetFocus(hwnd_);
    invalidate();
}

void App::cancelAppNameEdit() {
    if (!hNameEdit_ || nameCommitting_) return;
    nameCommitting_ = true;
    HWND edit = hNameEdit_;
    hNameEdit_ = nullptr;
    g_nameOwner = nullptr;
    DestroyWindow(edit);
    nameCommitting_ = false;
    SetFocus(hwnd_);
    invalidate();
}

}  // namespace qhb
