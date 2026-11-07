// 轻画板 - native Windows whiteboard
#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Gdiplus;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace qhb {

constexpr float PAGE_W = 1600.0f;
constexpr float PAGE_H = 900.0f;
constexpr wchar_t APP_NAME[] = L"HAVEGET白板";
constexpr wchar_t APP_TAGLINE[] = L"为灵感而生";
constexpr wchar_t APP_VERSION[] = L"1.8.2.4";
constexpr wchar_t FILE_EXT[] = L"hgwb";
constexpr int IDI_APP = 101;
constexpr int IDI_LOGO = 102;

// palette
constexpr uint32_t COL_INK = 0x202633;
constexpr uint32_t COL_TEXT = 0x263041;
constexpr uint32_t COL_MUTED = 0x5F7167;
constexpr uint32_t COL_FAINT = 0x84948B;
constexpr uint32_t COL_ACCENT = 0x14A57B;
constexpr uint32_t COL_ACCENT_SOFT = 0xE7F7F1;
constexpr uint32_t COL_ACCENT_HOVER = 0xDAF3E9;
constexpr uint32_t COL_LINE = 0xDCE5DF;
constexpr uint32_t COL_DESK = 0xEDF2EF;
constexpr uint32_t COL_WHITE = 0xFFFFFF;
constexpr uint32_t COL_DANGER = 0xE5484D;
constexpr uint32_t COL_SUCCESS = 0x18A56F;
constexpr uint32_t COL_CREAM = 0xFFFDF4;
constexpr uint32_t COL_GREENPAPER = 0xF7FBF3;
constexpr uint32_t COL_BLUEPAPER = 0xF6FAFF;
constexpr uint32_t COL_YELLOW = 0xFFD54A;

inline float easeOutCubic(float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

inline Color RGBA(uint32_t rgb, BYTE a = 255) {
    return Color(a, (BYTE)((rgb >> 16) & 0xFF), (BYTE)((rgb >> 8) & 0xFF), (BYTE)(rgb & 0xFF));
}

struct Pt {
    float x = 0, y = 0;
};

struct Xf {
    float s = 1.0f;
    Pt o{0, 0};
    Pt to(Pt p) const { return {o.x + p.x * s, o.y + p.y * s}; }
    float tx(float x) const { return o.x + x * s; }
    float ty(float y) const { return o.y + y * s; }
    RectF rect(float x, float y, float w, float h) const {
        return RectF(tx(x), ty(y), w * s, h * s);
    }
};

enum class Tool { Select, Pen, Highlighter, Eraser, Shape, Text, Laser, Background, Hand };
enum class ItemKind { Stroke, Shape, Text, Image };
enum class ShapeKind { Line, Arrow, Rect, Ellipse };
enum class DragMode { None, Draw, Erase, Shape, Move, Resize, Pan, Laser, Dock };
enum class CursorKind { Arrow, Cross, Hand, SizeAll, IBeam };

struct Item {
    ItemKind kind = ItemKind::Stroke;
    uint64_t id = 0;
    uint32_t color = COL_INK;
    float width = 4.0f;

    // stroke
    std::wstring strokeTool = L"pen";
    std::string funcExpr;
    bool isPoint = false;
    std::string pointText;
    std::vector<Pt> points;

    // plotted function metadata (stored on Stroke items)
    bool functionGraph = false;
    bool showAxes = true;
    float originX = 0, originY = 0;
    float gLeft = 0, gTop = 0, gRight = 0, gBottom = 0;
    float unitX = 0, unitY = 0;

    // shape
    ShapeKind shape = ShapeKind::Rect;
    bool filled = false;
    float x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    // text
    std::wstring text;
    float fontSize = 36.0f;
    float textWidth = 420.0f;
    float x = 0, y = 0;

    // image
    float w = 0, h = 0;
    std::string data;  // base64 PNG

    RectF bounds() const;
    bool hit(float px, float py, float tolerance) const;
    Item shifted(float dx, float dy) const;
};

struct Page {
    std::vector<Item> items;
    std::wstring pattern = L"blank";
    uint32_t paper = COL_WHITE;
    int revision = 1;
    void touch() { ++revision; }
};

struct Snapshot {
    std::vector<Page> pages;
    int current = 0;
};

struct Region {
    std::string id;
    RectF rect;
    std::wstring tip;
    CursorKind cursor = CursorKind::Arrow;
    bool enabled = true;
};

struct LaserPt {
    Pt p;
    ULONGLONG t = 0;
};

enum class Icon {
    Cursor, Pen, Highlighter, Eraser, Shape, Text, Laser, Image, Background, Function, Hand,
    Undo, Redo, Folder, Save, Export, Fullscreen, Help, Plus, ChevronLeft,
    ChevronRight, Close, Trash, Copy, Front, Back, Fit, Minus, Check, Page, Settings
};

struct RailDef {
    const char* id;
    Icon icon;
    const wchar_t* label;
    const wchar_t* tip;
};

class App {
public:
    App(HWND hwnd, HINSTANCE inst);
    ~App();

    void paint(HDC hdc, const RECT* dirty = nullptr);
    void onResize();
    void onDpiChanged(UINT dpi, const RECT* suggested);
    void onMouseMove(int x, int y, WPARAM buttons);
    void onMouseLeave();
    void onLButtonDown(int x, int y);
    void onLButtonUp(int x, int y);
    void onLButtonDblClk(int x, int y);
    void onRButtonUp(int x, int y);
    void onMouseWheel(int x, int y, int delta, bool ctrl, bool shift);
    bool onGesture(LPARAM lp);
    void verifyDevPassword();
    void onPointerDown(UINT id, int sx, int sy);
    void onPointerUpdate(UINT id, int sx, int sy);
    void onPointerUp(UINT id, int sx, int sy);
    void onPointerCaptureLost(UINT id);
    bool onKeyDown(WPARAM vk, bool ctrl, bool shift, bool alt);
    void onKeyUp(WPARAM vk);
    bool onChar(wchar_t ch);
    void openFunctionDialog();
    void onTimer(UINT_PTR id);
    void onCloseRequest();
    void onDrop(HDROP drop);
    HWND hwnd() const { return hwnd_; }
    bool isTextEditing() const { return hEdit_ != nullptr; }
    void commitText(bool cancel = false);
    void refreshCursor() { updateCursor(); }
    void finishAppNameEdit() { commitAppName(); }
    void cancelAppNameEdit();

    static constexpr UINT WM_APP_COMMIT_TEXT = WM_APP + 17;
    static constexpr UINT WM_APP_COMMIT_NAME = WM_APP + 18;

private:
    HWND hwnd_ = nullptr;
    HINSTANCE inst_ = nullptr;
    int W_ = 0, H_ = 0;
    UINT dpi_ = 96;
    float sc_ = 1.0f;

    std::vector<Page> pages_;
    int current_ = 0;
    std::wstring title_ = L"未命名白板";
    std::wstring path_;
    bool dirty_ = false;

    Tool tool_ = Tool::Pen;
    ShapeKind shapeKind_ = ShapeKind::Rect;
    bool shapeFilled_ = false;
    uint32_t penColor_ = COL_INK;
    uint32_t highlighterColor_ = 0xFFD54A;
    uint32_t shapeColor_ = COL_INK;
    uint32_t textColor_ = COL_INK;
    uint32_t laserColor_ = 0xE5484D;
    float penWidth_ = 4.0f;
    float highlighterWidth_ = 24.0f;
    float eraserSize_ = 30.0f;
    float shapeWidth_ = 4.0f;
    float textSize_ = 36.0f;

    float zoom_ = 1.0f;
    Pt camCenter_{PAGE_W / 2.0f, PAGE_H / 2.0f};

    int sel_ = -1;
    std::string hoverId_;
    ULONGLONG hoverSince_ = 0;
    bool trackingMouse_ = false;
    POINT mouse_{0, 0};
    bool mouseInside_ = false;

    std::vector<Region> regions_;
    std::deque<Snapshot> undo_;
    std::deque<Snapshot> redo_;

    DragMode drag_ = DragMode::None;
    bool gestureUndoPushed_ = false;
    Snapshot pendingSnapshot_;
    bool pendingSnapshotValid_ = false;
    Item dragOrig_;
    RectF dragOrigBounds_{};
    Pt dragStartWorld_{}, dragLastWorld_{};
    Pt panStartMouse_{}, panStartCenter_{};
    int resizeCorner_ = -1;
    Item liveStroke_;
    Item previewShape_;
    bool eraserChanged_ = false;
    bool spaceDown_ = false;

    std::vector<LaserPt> laser_;
    bool laserDown_ = false;
    ULONGLONG lastLaserTime_ = 0;

    int overlay_ = 0;  // 0 none, 1 pages, 2 help, 3 settings, 4 function, 6 dev password
    float pageScroll_ = 0;
    bool bgPanel_ = false;

    bool splash_ = true;
    bool splashLeaving_ = false;
    ULONGLONG splashStart_ = 0;
    ULONGLONG splashLeaveStart_ = 0;
    ULONGLONG boardIntroStart_ = 0;
    ULONGLONG pageFadeStart_ = 0;
    float settingsAnim_ = 1.0f;
    bool settingsClosing_ = false;
    ULONGLONG settingsAnimStart_ = 0;
    ULONGLONG toastAnimStart_ = 0;
    std::wstring appName_ = APP_NAME;
    std::wstring configDir_, configPath_, imageDir_;
    std::wstring launcherBgFile_, logoFile_;
    bool devMode_ = false;
    bool devUnlocked_ = false;
    std::string devPass_;
    std::wstring devPassError_;
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

    HWND hEdit_ = nullptr;
    int editIndex_ = -1;
    std::wstring editOriginal_;
    float editX_ = 0, editY_ = 0, editWidth_ = 420;
    bool textCommitting_ = false;

    HWND hNameEdit_ = nullptr;
    bool nameCommitting_ = false;

    std::wstring toast_;
    ULONGLONG toastUntil_ = 0;
    bool toastError_ = false;

    bool fullscreen_ = false;
    WINDOWPLACEMENT savedPlacement_{};

    std::unique_ptr<Font> fBrand_, fUI_, fSmall_, fMini_, fNav_, fTitle_, fBig_, fToast_, fTextHint_;
    std::unique_ptr<Font> fHero_, fSplashSub_, fSection_, fTiny_, fCapsule_;
    std::unique_ptr<Bitmap> logo_;
    std::unique_ptr<Bitmap> launcherBg_, customLogo_;
    std::unique_ptr<Bitmap> back_;
    std::unique_ptr<Bitmap> base_;
    bool baseDirty_ = true;
    bool liveDirtyValid_ = false;
    std::vector<Pt> eraserPath_;
    size_t eraserDrawn_ = 0;
    size_t liveInkDrawn_ = 0;
    RECT liveDirty_{};
    ULONGLONG lastLivePaint_ = 0;
    int pageDragFrom_ = -1;
    int pageDragTo_ = -1;
    bool pageDragActive_ = false;
    Pt pageDragStart_{};
    bool pageScrollDrag_ = false;
    bool pageScrollToCurrent_ = false;
    float pageScrollDragStartY_ = 0;
    float pageScrollDragStart_ = 0;
    float pageScrollFrom_ = 0;
    float pageScrollTo_ = 0;
    ULONGLONG pageScrollAnimStart_ = 0;
    bool pageScrollAnimating_ = false;
    Pt camScrollFrom_{};
    Pt camScrollTo_{};
    ULONGLONG camScrollStart_ = 0;
    bool camScrollAnimating_ = false;
    std::unique_ptr<Bitmap> pageContentBmp_;
    std::unique_ptr<Bitmap> dockDragBase_;
    std::unique_ptr<Bitmap> moveBase_;
    int hideItemForDrag_ = -1;
    float pageScrollMax_ = 0;
    float pageScrollTrackH_ = 0;
    float pageScrollThumbH_ = 0;
    std::string fnExpr_;
    std::wstring fnError_;
    bool fnShowAxes_ = true;
    bool fnIgnoreNextChar_ = false;
    int editingFunction_ = -1;
    bool fnPointMode_ = false;
    int fnPointGraph_ = -1;
    std::vector<int> pointSel_;
    double gestureLastZoom_ = 0;
    float toolAnim_ = 1.0f;
    ULONGLONG toolAnimStart_ = 0;
    bool handlingPointer_ = false;
    bool pointerActive_ = false;
    UINT activePointerId_ = 0;
    ULONGLONG suppressMouseUntil_ = 0;
    HBRUSH editBrush_ = nullptr;
    std::unordered_map<uint64_t, Bitmap*> imageCache_;
    uint64_t nextId_ = 1;

    // ---------- geometry / camera ----------
    float S(float v) const { return v * sc_; }
    float headerH() const { return S(64); }
    RectF clientRect() const { return RectF(0, 0, (float)W_, (float)H_); }
    RectF pageScreenRect() const;
    RectF splashRect() const;
    RectF dockRect() const;
    float dockWidth() const;
    float dockHeight() const;
    RectF settingsRect() const;
    RectF settingsNameRect() const;
    Xf pageXf() const;
    float baseScale() const;
    float viewScale() const { return baseScale() * zoom_; }
    Pt viewCenter() const;
    Pt worldToScreen(Pt p) const;
    Pt screenToWorld(Pt p) const;
    bool inPage(Pt screen) const;
    void fitView();
    void zoomAt(Pt screen, float newZoom);
    void zoomStep(float mul, Pt anchor);
    void clampCam();

    // ---------- pages / history ----------
    Page& page() { return pages_[current_]; }
    const Page& page() const { return pages_[current_]; }
    Snapshot snapshot() const { return {pages_, current_}; }
    void restoreSnapshot(const Snapshot& s);
    void pushUndo();
    void pushUndoSnapshot(const Snapshot& s);
    void undo();
    void redo();
    void markDirty();
    void updateWindowTitle();
    void switchPage(int index, bool closeOverlay = true);
    void addPage();
    void movePage(int from, int to);
    void duplicatePage();
    void deletePage();
    void clearPage();
    int pageCount() const { return (int)pages_.size(); }

    // ---------- selection / editing ----------
    int hitItemAt(Pt world, float tolerancePx = 5.0f) const;
    RectF selectionScreenRect() const;
    int hitHandle(int x, int y) const;
    void deleteSelection();
    void duplicateSelection();
    void bringSelection(bool front);
    void beginTextEdit(Pt world, int existing = -1);
    void createEditControl();
    void layoutEditControl();
    HFONT makeEditFont();
    void applyTextCommit();

    // ---------- file ----------
    std::string serializeDocument() const;
    bool loadDocumentData(const std::string& data, std::wstring& err);
    bool saveDocument(bool saveAs);
    bool openDocumentDialog();
    bool exportPng();
    void newDocument();
    bool confirmDiscard();
    std::wstring chooseSavePath(const wchar_t* filter, const wchar_t* defExt, const wchar_t* defaultName);
    std::wstring chooseOpenPath(const wchar_t* filter);
    void setTitleFromPath();
    void showMessage(const std::wstring& text, bool error = false);
    void showToast(const std::wstring& text, bool error = false, int ms = 2400);

    // ---------- image ----------
    Bitmap* getImage(const Item& it);
    std::string encodeImageBase64(const std::wstring& file, float& outW, float& outH);
    void insertImageDialog();
    void insertImageFile(const std::wstring& file);
    void pasteImageFromClipboard();
    void clearImageCache();

    // ---------- tools / UI actions ----------
    void setTool(Tool t);
    void handleRegion(const std::string& id);
    std::string regionAt(int x, int y, CursorKind* cursor = nullptr) const;
    void addRegion(const std::string& id, RectF r, const std::wstring& tip = L"", CursorKind cur = CursorKind::Arrow, bool enabled = true);
    Region* findRegion(const std::string& id);
    void resetHover();
    void updateHoverTooltip();
    void invalidate();
    void invalidateLive();
    void invalidateLiveRect(const RECT& r);
    void addLiveSample(Pt world);
    void addEraserSample(Pt world);
    bool eraseGeometry(Pt world);
    bool isLiveDrawing() const;
    void updateFonts();
    void toggleFullscreen();
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
    void renderBase(Graphics& g);
    void drawBackdrop(Graphics& g);
    void drawSplash(Graphics& g);
    void drawHeader(Graphics& g);
    void drawPage(Graphics& g);
    void drawPageContent(Graphics& g, const Page& p, Xf xf, const RectF& clip, bool hideEditing);
    void drawItem(Graphics& g, const Item& it, Xf xf, bool hideThis, bool exportMode);
    void drawFunctionAxes(Graphics& g, const Item& it, Xf xf);
    void drawPattern(Graphics& g, const Page& p, Xf xf, const RectF& clip);
    void drawSelection(Graphics& g);
    void drawLive(Graphics& g);
    void drawRail(Graphics& g);
    void drawOptionBar(Graphics& g);
    void drawZoomBar(Graphics& g);
    void drawStatus(Graphics& g);
    void drawBgPanel(Graphics& g);
    void drawToast(Graphics& g);
    void drawOverlay(Graphics& g);
    void drawPagesOverlay(Graphics& g);
    void drawHelpOverlay(Graphics& g);
    void drawFunctionDialog(Graphics& g);
    void drawDevPasswordDialog(Graphics& g);
    void insertFunctionGraph();
    void submitFunctionDialog();
    void insertPointFromInput();
    void connectSelectedPoints(int mode, int direction);
    void fitFunctionToPage(Item& item);
    void scaleSelectedFunction(float factor, Pt screenAnchor);

    void drawTooltip(Graphics& g);
    void drawTextEditorHint(Graphics& g);

    // drawing helpers
    void drawHeaderButton(Graphics& g, const std::string& id, RectF r, Icon icon, const std::wstring& label,
                          bool primary, bool outline, bool enabled, const std::wstring& tip);
    void drawIconButton(Graphics& g, const std::string& id, RectF r, Icon icon, bool enabled,
                        const std::wstring& tip);
    void drawPageNav(Graphics& g);
    void drawSwatch(Graphics& g, const std::string& id, float cx, float cy, uint32_t color,
                    float radius, bool selected, const std::wstring& tip);
    void drawWidthDot(Graphics& g, const std::string& id, float cx, float cy, float width,
                      bool selected, const std::wstring& tip);
    void drawOptLabel(Graphics& g, float x, float y, const wchar_t* text);
    void drawMiniButton(Graphics& g, const std::string& id, RectF r, Icon icon, const wchar_t* label,
                        bool enabled, const std::wstring& tip);
    void drawClearButton(Graphics& g, RectF r, const std::wstring& tip);
    void drawGlass(Graphics& g, RectF r, float radius, BYTE alpha = 225);
    void drawGlassCapsule(Graphics& g, RectF r, BYTE alpha = 210);
    void drawBrand(Graphics& g, bool onSplash);
    void drawDock(Graphics& g);
    void drawDockContext(Graphics& g, RectF r, Tool t);
    void drawSettings(Graphics& g);
    void drawNameEditor(Graphics& g);
    void drawSplashLoading(Graphics& g, RectF card);

    // ---------- input ----------
    void startPan(Pt screen);
    void startDraw(Pt world);
    void startShape(Pt world);
    void startErase(Pt world);
    void eraseAt(Pt world);
    void finishDrag(Pt screen);
    void canvasMouseDown(int x, int y);
    void rightClickMenu(int x, int y);
    void updateCursor();
    void updateLaser();
    void startLaser(Pt screen);
};

// ---- free helpers ----
std::string utf8(const std::wstring& s);
std::wstring wide(const std::string& s);
std::string b64encode(const std::string& in);
std::string b64decode(const std::string& in);
uint32_t hexColor(const std::string& s);
std::string hexColor(uint32_t c);
std::string trimCopy(const std::string& s);
std::wstring trimW(const std::wstring& s);
std::wstring lowerExt(const std::wstring& path);
bool fileExists(const std::wstring& path);
std::wstring fileNameOf(const std::wstring& path);
std::wstring stemOf(const std::wstring& path);
std::wstring fmtFloat(float v, int decimals = 2);

void fillRoundRect(Graphics& g, RectF r, float radius, uint32_t color, BYTE alpha = 255);
void strokeRoundRect(Graphics& g, RectF r, float radius, uint32_t color, float width, BYTE alpha = 255);
void roundPath(GraphicsPath& out, RectF r, float radius);
void drawText(Graphics& g, const std::wstring& text, RectF r, Font* font, uint32_t color,
              StringAlignment h = StringAlignmentNear, StringAlignment v = StringAlignmentCenter,
              bool ellipsis = false, BYTE alpha = 255);
void drawTextAligned(Graphics& g, const std::wstring& text, RectF r, Font* font, uint32_t color,
                     StringAlignment h, StringAlignment v, bool ellipsis = false);
inline void drawTextAligned(Graphics& g, const std::wstring& text, RectF r,
                            const std::unique_ptr<Font>& font, uint32_t color,
                            StringAlignment h = StringAlignmentNear,
                            StringAlignment v = StringAlignmentCenter, bool ellipsis = false) {
    drawTextAligned(g, text, r, font.get(), color, h, v, ellipsis);
}
inline void drawText(Graphics& g, const std::wstring& text, RectF r,
                     const std::unique_ptr<Font>& font, uint32_t color,
                     StringAlignment h = StringAlignmentNear,
                     StringAlignment v = StringAlignmentCenter,
                     bool ellipsis = false, BYTE alpha = 255) {
    drawText(g, text, r, font.get(), color, h, v, ellipsis, alpha);
}
void drawIcon(Graphics& g, Icon icon, RectF box, uint32_t color, float width, BYTE alpha = 255);
bool sampleExpression(const std::string& expr, double xmin, double xmax, int count,
                      std::vector<std::pair<double, double>>& out, std::string& error);
void drawShadow(Graphics& g, RectF r, float radius, float spread, float offsetY);
void drawCheckmark(Graphics& g, RectF box, uint32_t color, float width);
std::wstring toolName(Tool t);
const wchar_t* toolHint(Tool t);
int getEncoderClsid(const WCHAR* format, CLSID* clsid);
bool saveBitmapPng(Bitmap* bmp, const std::wstring& path);
std::string encodePngBase64(Bitmap* bmp);

}  // namespace qhb
