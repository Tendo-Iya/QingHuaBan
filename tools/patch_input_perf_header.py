from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")

s = s.replace("""    void onMouseWheel(int x, int y, int delta, bool ctrl, bool shift);
    bool onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt);""",
"""    void onMouseWheel(int x, int y, int delta, bool ctrl, bool shift);
    void onPointerDown(UINT id, int sx, int sy);
    void onPointerUpdate(UINT id, int sx, int sy);
    void onPointerUp(UINT id, int sx, int sy);
    void onPointerCaptureLost(UINT id);
    bool onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt);""")

s = s.replace("""    std::unique_ptr<Bitmap> back_;
    HBRUSH editBrush_ = nullptr;""",
"""    std::unique_ptr<Bitmap> back_;
    std::unique_ptr<Bitmap> base_;
    bool baseDirty_ = true;
    bool handlingPointer_ = false;
    bool pointerActive_ = false;
    UINT activePointerId_ = 0;
    ULONGLONG suppressMouseUntil_ = 0;
    HBRUSH editBrush_ = nullptr;""")

s = s.replace("""    void invalidate();
    void updateFonts();
    void toggleFullscreen();""",
"""    void invalidate();
    void invalidateLive();
    bool isLiveDrawing() const;
    void updateFonts();
    void toggleFullscreen();""")

s = s.replace("""    void insertImageDialog();
    void insertImageFile(const std::wstring& file);
    void clearImageCache();""",
"""    void insertImageDialog();
    void insertImageFile(const std::wstring& file);
    void pasteImageFromClipboard();
    void clearImageCache();""")

s = s.replace("""    void render(Graphics& g);
    void drawBackdrop(Graphics& g);""",
"""    void render(Graphics& g);
    void renderBase(Graphics& g);
    void drawBackdrop(Graphics& g);""")

p.write_text(s, encoding="utf-8", newline="\n")
print("header input/perf fields added")
