from pathlib import Path

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")

s = s.replace('constexpr wchar_t APP_NAME[] = L"轻画板";', 'constexpr wchar_t APP_NAME[] = L"HAVEGET白板";')
s = s.replace('constexpr wchar_t APP_TAGLINE[] = L"即点即写";', 'constexpr wchar_t APP_TAGLINE[] = L"为灵感而生";')
s = s.replace('constexpr wchar_t FILE_EXT[] = L"qhb";', 'constexpr wchar_t FILE_EXT[] = L"hgwb";')
s = s.replace("constexpr uint32_t COL_ACCENT = 0x2F6BFF;", "constexpr uint32_t COL_ACCENT = 0x14A57B;")
s = s.replace("constexpr uint32_t COL_ACCENT_SOFT = 0xEAF0FF;", "constexpr uint32_t COL_ACCENT_SOFT = 0xE7F7F1;")
s = s.replace("constexpr uint32_t COL_ACCENT_HOVER = 0xDFE8FF;", "constexpr uint32_t COL_ACCENT_HOVER = 0xDAF3E9;")
s = s.replace("constexpr uint32_t COL_DESK = 0xEEF2F7;", "constexpr uint32_t COL_DESK = 0xEDF2EF;")
s = s.replace("enum class DragMode { None, Draw, Erase, Shape, Move, Resize, Pan, Laser };",
              "enum class DragMode { None, Draw, Erase, Shape, Move, Resize, Pan, Laser, Dock };")
s = s.replace("    ChevronRight, Close, Trash, Copy, Front, Back, Fit, Minus, Check, Page\n};",
              "    ChevronRight, Close, Trash, Copy, Front, Back, Fit, Minus, Check, Page, Settings\n};")

s = s.replace("    static constexpr UINT WM_APP_COMMIT_TEXT = WM_APP + 17;",
              "    static constexpr UINT WM_APP_COMMIT_TEXT = WM_APP + 17;\n    static constexpr UINT WM_APP_COMMIT_NAME = WM_APP + 18;")

s = s.replace("""    std::unique_ptr<Font> fBrand_, fUI_, fSmall_, fMini_, fNav_, fTitle_, fBig_, fToast_, fTextHint_;
    std::unique_ptr<Bitmap> logo_;
    std::unique_ptr<Bitmap> back_;""",
"""    std::unique_ptr<Font> fBrand_, fUI_, fSmall_, fMini_, fNav_, fTitle_, fBig_, fToast_, fTextHint_;
    std::unique_ptr<Font> fHero_, fSplashSub_, fSection_, fTiny_, fCapsule_;
    std::unique_ptr<Bitmap> logo_;
    std::unique_ptr<Bitmap> launcherBg_, customLogo_;
    std::unique_ptr<Bitmap> back_;""")

s = s.replace("""    int overlay_ = 0;  // 0 none, 1 pages, 2 help
    float pageScroll_ = 0;
    bool bgPanel_ = false;

    HWND hEdit_ = nullptr;""",
"""    int overlay_ = 0;  // 0 none, 1 pages, 2 help, 3 settings
    float pageScroll_ = 0;
    bool bgPanel_ = false;

    bool splash_ = true;
    ULONGLONG splashStart_ = 0;
    std::wstring appName_ = APP_NAME;
    std::wstring configDir_, configPath_, imageDir_;
    std::wstring launcherBgFile_, logoFile_;
    bool devMode_ = false;
    bool applyAllPages_ = false;
    bool dockDefault_ = true;
    float dockNormX_ = 0.5f;
    float dockNormY_ = 0.90f;
    float dockW_ = 0.0f;
    float dockH_ = 0.0f;
    Pt dockDragStart_{};
    Pt dockDragStartCenter_{};
    int versionClicks_ = 0;
    ULONGLONG versionClickLast_ = 0;

    HWND hEdit_ = nullptr;""")

s = s.replace("""    bool textCommitting_ = false;

    std::wstring toast_;""",
"""    bool textCommitting_ = false;

    HWND hNameEdit_ = nullptr;
    bool nameCommitting_ = false;

    std::wstring toast_;""")

s = s.replace("""    float S(float v) const { return v * sc_; }
    float headerH() const { return S(72); }
    RectF clientRect() const { return RectF(0, 0, (float)W_, (float)H_); }
    RectF pageScreenRect() const;""",
"""    float S(float v) const { return v * sc_; }
    float headerH() const { return S(64); }
    RectF clientRect() const { return RectF(0, 0, (float)W_, (float)H_); }
    RectF pageScreenRect() const;
    RectF splashRect() const;
    RectF dockRect() const;
    float dockWidth() const;
    float dockHeight() const;
    RectF settingsRect() const;""")

s = s.replace("""    void toggleFullscreen();

    // ---------- drawing ----------
    void render(Graphics& g);
    void drawHeader(Graphics& g);""",
"""    void toggleFullscreen();
    void enterBoard();
    void startDockDrag(Pt screen);
    void resetDockPosition();
    void loadConfig();
    void saveConfig();
    std::wstring appDataDir() const;
    void chooseLauncherBackground();
    void chooseCustomLogo();
    void clearLauncherBackground();
    void clearCustomLogo();
    void applyCanvasColor(uint32_t color, bool allPages);
    void beginAppNameEdit();
    void createNameEdit();
    void layoutNameEdit();
    void commitAppName();

    // ---------- drawing ----------
    void render(Graphics& g);
    void drawBackdrop(Graphics& g);
    void drawSplash(Graphics& g);
    void drawHeader(Graphics& g);""")

s = s.replace("""    void drawClearButton(Graphics& g, RectF r, const std::wstring& tip);

    // ---------- input ----------""",
"""    void drawClearButton(Graphics& g, RectF r, const std::wstring& tip);
    void drawGlass(Graphics& g, RectF r, float radius, BYTE alpha = 225);
    void drawGlassCapsule(Graphics& g, RectF r, BYTE alpha = 210);
    void drawBrand(Graphics& g, bool onSplash);
    void drawDock(Graphics& g);
    void drawDockContext(Graphics& g, RectF r, Tool t);
    void drawSettings(Graphics& g);
    void drawNameEditor(Graphics& g);
    void drawSplashLoading(Graphics& g, RectF card);

    // ---------- input ----------""")

p.write_text(s, encoding="utf-8", newline="\n")
print("header patched")
