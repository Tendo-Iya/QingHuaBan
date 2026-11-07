#include "qhb.h"

#include <cmath>
#include <limits>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <cwctype>
#include <objidl.h>

namespace qhb {

std::string utf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

std::wstring wide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}

static const char B64T[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string b64encode(const std::string& in) {
    std::string out;
    out.reserve((in.size() + 2) / 3 * 4);
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64T[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(B64T[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string b64decode(const std::string& in) {
    std::vector<int> table(256, -1);
    for (int i = 0; i < 64; ++i) table[(unsigned char)B64T[i]] = i;
    std::string out;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (c == '=') break;
        int d = table[c];
        if (d == -1) continue;
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            out.push_back((char)((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

uint32_t hexColor(const std::string& s) {
    std::string t = s;
    if (!t.empty() && t[0] == '#') t.erase(t.begin());
    if (t.size() != 6) return COL_INK;
    return (uint32_t)std::stoul(t, nullptr, 16);
}

std::string hexColor(uint32_t c) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%06X", c & 0xFFFFFF);
    return std::string(buf);
}

std::string trimCopy(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (unsigned char)s[a] <= ' ') ++a;
    while (b > a && (unsigned char)s[b - 1] <= ' ') --b;
    return s.substr(a, b - a);
}

std::wstring trimW(const std::wstring& s) {
    size_t a = 0, b = s.size();
    while (a < b && iswspace(s[a])) ++a;
    while (b > a && iswspace(s[b - 1])) --b;
    return s.substr(a, b - a);
}

std::wstring lowerExt(const std::wstring& path) {
    size_t dot = path.find_last_of(L'.');
    if (dot == std::wstring::npos) return L"";
    std::wstring e = path.substr(dot);
    for (auto& c : e) c = (wchar_t)towlower(c);
    return e;
}

bool fileExists(const std::wstring& path) {
    DWORD a = GetFileAttributesW(path.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring fileNameOf(const std::wstring& path) {
    size_t p = path.find_last_of(L"\\/");
    return p == std::wstring::npos ? path : path.substr(p + 1);
}

std::wstring stemOf(const std::wstring& path) {
    std::wstring f = fileNameOf(path);
    size_t p = f.find_last_of(L'.');
    return p == std::wstring::npos ? f : f.substr(0, p);
}

std::wstring fmtFloat(float v, int decimals) {
    std::wostringstream ss;
    ss.setf(std::ios::fixed);
    ss << std::setprecision(decimals) << v;
    std::wstring s = ss.str();
    if (s.find(L'.') != std::wstring::npos) {
        while (!s.empty() && s.back() == L'0') s.pop_back();
        if (!s.empty() && s.back() == L'.') s.pop_back();
    }
    return s;
}

void roundPath(GraphicsPath& p, RectF r, float radius) {
    p.Reset();
    float d = radius * 2.0f;
    if (d > r.Width) d = r.Width;
    if (d > r.Height) d = r.Height;
    if (d <= 0.1f) {
        p.AddRectangle(r);
        return;
    }
    p.AddArc(r.X, r.Y, d, d, 180, 90);
    p.AddArc(r.X + r.Width - d, r.Y, d, d, 270, 90);
    p.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0, 90);
    p.AddArc(r.X, r.Y + r.Height - d, d, d, 90, 90);
    p.CloseFigure();
}

void fillRoundRect(Graphics& g, RectF r, float radius, uint32_t color, BYTE alpha) {
    GraphicsPath p;
    roundPath(p, r, radius);
    SolidBrush b(RGBA(color, alpha));
    g.FillPath(&b, &p);
}

void strokeRoundRect(Graphics& g, RectF r, float radius, uint32_t color, float width, BYTE alpha) {
    GraphicsPath p;
    roundPath(p, r, radius);
    Pen pen(RGBA(color, alpha), width);
    pen.SetAlignment(PenAlignmentInset);
    g.DrawPath(&pen, &p);
}

void drawTextAligned(Graphics& g, const std::wstring& text, RectF r, Font* font, uint32_t color,
                     StringAlignment h, StringAlignment v, bool ellipsis) {
    StringFormat sf;
    sf.SetAlignment(h);
    sf.SetLineAlignment(v);
    sf.SetFormatFlags(StringFormatFlagsNoWrap);
    if (ellipsis) sf.SetTrimming(StringTrimmingEllipsisCharacter);
    SolidBrush b(RGBA(color));
    g.DrawString(text.c_str(), -1, font, r, &sf, &b);
}

void drawText(Graphics& g, const std::wstring& text, RectF r, Font* font, uint32_t color,
              StringAlignment h, StringAlignment v, bool ellipsis, BYTE alpha) {
    StringFormat sf;
    sf.SetAlignment(h);
    sf.SetLineAlignment(v);
    sf.SetFormatFlags(StringFormatFlagsNoWrap);
    if (ellipsis) sf.SetTrimming(StringTrimmingEllipsisCharacter);
    SolidBrush b(RGBA(color, alpha));
    g.DrawString(text.c_str(), -1, font, r, &sf, &b);
}

void drawShadow(Graphics& g, RectF r, float radius, float spread, float offsetY) {
    for (int i = 4; i >= 1; --i) {
        float e = spread * (float)i / 4.0f;
        RectF rr(r.X - e, r.Y - e + offsetY, r.Width + 2 * e, r.Height + 2 * e);
        fillRoundRect(g, rr, radius + e, 0x000000, (BYTE)(3 + (4 - i)));
    }
}

void drawCheckmark(Graphics& g, RectF box, uint32_t color, float width) {
    Pen p(RGBA(color), width);
    p.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    g.DrawLine(&p, box.X + box.Width * 0.16f, box.Y + box.Height * 0.53f,
               box.X + box.Width * 0.42f, box.Y + box.Height * 0.78f);
    g.DrawLine(&p, box.X + box.Width * 0.42f, box.Y + box.Height * 0.78f,
               box.X + box.Width * 0.84f, box.Y + box.Height * 0.25f);
}

void drawIcon(Graphics& g, Icon icon, RectF b, uint32_t color, float width, BYTE alpha) {
    if (b.Width <= 0 || b.Height <= 0) return;
    const float u = b.Width / 24.0f;
    auto X = [&](float v) { return b.X + b.Width * v / 24.0f; };
    auto Y = [&](float v) { return b.Y + b.Height * v / 24.0f; };
    auto P = [&](float x, float y) { return PointF(X(x), Y(y)); };
    Pen pen(RGBA(color, alpha), width);
    pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    pen.SetLineJoin(LineJoinRound);
    SolidBrush brush(RGBA(color, alpha));

    auto poly = [&](std::initializer_list<PointF> pts, bool fill) {
        GraphicsPath p;
        std::vector<PointF> v(pts);
        if (!v.empty()) {
            p.AddPolygon(v.data(), (INT)v.size());
            if (fill) g.FillPath(&brush, &p);
            else g.DrawPath(&pen, &p);
        }
    };

    switch (icon) {
        case Icon::Cursor: {
            std::vector<PointF> pts = {P(5, 3), P(19, 12), P(12.7f, 12.8f), P(16.2f, 20.5f),
                                       P(13.2f, 21.8f), P(9.8f, 14.1f), P(5, 18.5f)};
            GraphicsPath p;
            p.AddPolygon(pts.data(), (INT)pts.size());
            g.FillPath(&brush, &p);
            break;
        }
        case Icon::Pen: {
            poly({P(4, 20), P(7, 15), P(16, 5), P(20, 9), P(10, 18)}, true);
            Pen wp(RGBA(COL_WHITE, alpha), width * 1.2f);
            wp.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            g.DrawLine(&wp, P(13.2f, 7.8f), P(17.2f, 11.8f));
            g.DrawLine(&pen, P(5.5f, 18.5f), P(7.2f, 16.8f));
            break;
        }
        case Icon::Highlighter: {
            Pen body(RGBA(color, alpha), width * 2.8f);
            body.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            g.DrawLine(&body, P(6.5f, 17.5f), P(15.5f, 8.5f));
            poly({P(14, 5), P(20, 11), P(17.5f, 13.5f), P(11.5f, 7.5f)}, true);
            g.DrawLine(&pen, P(4.2f, 21), P(8.5f, 16.7f));
            break;
        }
        case Icon::Eraser: {
            poly({P(5, 15.5f), P(11.5f, 21), P(21, 11.5f), P(14.5f, 6)}, true);
            Pen wp(RGBA(COL_WHITE, alpha), width * 0.85f);
            g.DrawLine(&wp, P(8, 12.5f), P(14.7f, 19.2f));
            g.DrawLine(&pen, P(7.5f, 18.5f), P(15, 21));
            break;
        }
        case Icon::Shape: {
            g.DrawRectangle(&pen, X(3.5f), Y(4.5f), u * 10.5f, u * 10.5f);
            g.DrawEllipse(&pen, X(12.5f), Y(12), u * 9.0f, u * 9.0f);
            break;
        }
        case Icon::Text: {
            g.DrawLine(&pen, P(5, 6), P(19, 6));
            g.DrawLine(&pen, P(12, 6), P(12, 19.5f));
            g.DrawLine(&pen, P(8, 20), P(16, 20));
            break;
        }
        case Icon::Laser: {
            SolidBrush red(RGBA(color, alpha));
            g.FillEllipse(&red, X(14.5f), Y(4), u * 4, u * 4);
            g.DrawLine(&pen, P(14.5f, 8.5f), P(10, 17));
            g.DrawLine(&pen, P(11, 18), P(5, 21));
            Pen ray(RGBA(color, alpha), width * 0.8f);
            g.DrawLine(&ray, P(20.5f, 6), P(23, 3.5f));
            g.DrawLine(&ray, P(19, 3.5f), P(20, 1.5f));
            g.DrawLine(&ray, P(22, 10), P(23.5f, 9));
            break;
        }
        case Icon::Image: {
            strokeRoundRect(g, RectF(X(3.2f), Y(4.5f), u * 17.6f, u * 15.0f), u * 2.2f, color, width, alpha);
            g.FillEllipse(&brush, X(7), Y(8), u * 2.6f, u * 2.6f);
            poly({P(5, 18), P(10, 12.5f), P(13.2f, 15.7f), P(16, 13), P(20, 18)}, false);
            break;
        }
        case Icon::Background: {
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 2; ++j)
                    g.DrawRectangle(&pen, X(4 + i * 8), Y(4 + j * 8), u * 7, u * 7);
            SolidBrush fill(RGBA(color, (BYTE)(alpha * 0.28f)));
            g.FillRectangle(&fill, X(12), Y(4), u * 7, u * 7);
            g.FillRectangle(&fill, X(4), Y(12), u * 7, u * 7);
            break;
        }
        case Icon::Hand: {
            g.DrawLine(&pen, P(12, 3), P(12, 21));
            g.DrawLine(&pen, P(3, 12), P(21, 12));
            poly({P(12, 1), P(9.5f, 4.5f), P(14.5f, 4.5f)}, true);
            poly({P(12, 23), P(9.5f, 19.5f), P(14.5f, 19.5f)}, true);
            poly({P(1, 12), P(4.5f, 9.5f), P(4.5f, 14.5f)}, true);
            poly({P(23, 12), P(19.5f, 9.5f), P(19.5f, 14.5f)}, true);
            break;
        }
        case Icon::Undo:
        case Icon::Redo: {
            bool redo = icon == Icon::Redo;
            auto MX = [&](float x) { return redo ? X(24 - x) : X(x); };
            auto MP = [&](float x, float y) { return PointF(MX(x), Y(y)); };
            GraphicsPath path;
            path.AddBezier(MP(19, 19), MP(10, 19), MP(6, 14), MP(6, 7));
            g.DrawPath(&pen, &path);
            std::vector<PointF> head = {MP(6, 3.8f), MP(2.8f, 9.5f), MP(9.2f, 9.5f)};
            GraphicsPath hp;
            hp.AddPolygon(head.data(), (INT)head.size());
            g.FillPath(&brush, &hp);
            break;
        }
        case Icon::Folder: {
            GraphicsPath p;
            p.AddLine(P(3, 7), P(9, 7));
            p.AddLine(P(9, 7), P(11, 9.5f));
            p.AddLine(P(11, 9.5f), P(21, 9.5f));
            p.AddLine(P(21, 9.5f), P(21, 20));
            p.AddLine(P(21, 20), P(3, 20));
            p.CloseFigure();
            g.DrawPath(&pen, &p);
            g.DrawLine(&pen, P(3, 12), P(21, 12));
            break;
        }
        case Icon::Save: {
            GraphicsPath p;
            p.AddLine(P(4, 4), P(17, 4));
            p.AddLine(P(17, 4), P(20, 7));
            p.AddLine(P(20, 7), P(20, 20));
            p.AddLine(P(20, 20), P(4, 20));
            p.CloseFigure();
            g.DrawPath(&pen, &p);
            g.DrawRectangle(&pen, X(7), Y(4), u * 8, u * 5.5f);
            g.DrawRectangle(&pen, X(7), Y(13.5f), u * 10, u * 6.5f);
            break;
        }
        case Icon::Export: {
            g.DrawLine(&pen, P(12, 3), P(12, 14.5f));
            poly({P(12, 18), P(7.8f, 13.2f), P(16.2f, 13.2f)}, false);
            GraphicsPath tray;
            tray.AddLine(P(4, 13), P(4, 20));
            tray.AddLine(P(4, 20), P(20, 20));
            tray.AddLine(P(20, 20), P(20, 13));
            g.DrawPath(&pen, &tray);
            break;
        }
        case Icon::Fullscreen: {
            auto corner = [&](float x, float y, float dx, float dy) {
                GraphicsPath p;
                p.AddLine(P(x, y + dy * 4.5f), P(x, y));
                p.AddLine(P(x, y), P(x + dx * 4.5f, y));
                g.DrawPath(&pen, &p);
            };
            corner(4, 4, 1, 1);
            corner(20, 4, -1, 1);
            corner(4, 20, 1, -1);
            corner(20, 20, -1, -1);
            break;
        }
        case Icon::Help: {
            g.DrawEllipse(&pen, X(3), Y(3), u * 18, u * 18);
            GraphicsPath q;
            q.AddArc(X(8), Y(7), u * 8, u * 8, 190, 250);
            g.DrawPath(&pen, &q);
            g.DrawLine(&pen, P(12, 13.5f), P(12, 15.5f));
            g.FillEllipse(&brush, X(11), Y(17.2f), u * 2, u * 2);
            break;
        }
        case Icon::Plus: {
            g.DrawLine(&pen, P(12, 4.5f), P(12, 19.5f));
            g.DrawLine(&pen, P(4.5f, 12), P(19.5f, 12));
            break;
        }
        case Icon::ChevronLeft: {
            g.DrawLine(&pen, P(14.5f, 5), P(7.5f, 12));
            g.DrawLine(&pen, P(7.5f, 12), P(14.5f, 19));
            break;
        }
        case Icon::ChevronRight: {
            g.DrawLine(&pen, P(9.5f, 5), P(16.5f, 12));
            g.DrawLine(&pen, P(16.5f, 12), P(9.5f, 19));
            break;
        }
        case Icon::Close: {
            g.DrawLine(&pen, P(6, 6), P(18, 18));
            g.DrawLine(&pen, P(18, 6), P(6, 18));
            break;
        }
        case Icon::Trash: {
            g.DrawLine(&pen, P(4, 6.5f), P(20, 6.5f));
            g.DrawLine(&pen, P(9, 6.5f), P(9.5f, 3.5f));
            g.DrawLine(&pen, P(9.5f, 3.5f), P(14.5f, 3.5f));
            g.DrawLine(&pen, P(14.5f, 3.5f), P(15, 6.5f));
            GraphicsPath p;
            p.AddLine(P(6, 9), P(7, 21));
            p.AddLine(P(7, 21), P(17, 21));
            p.AddLine(P(17, 21), P(18, 9));
            g.DrawPath(&pen, &p);
            g.DrawLine(&pen, P(10, 12), P(10.5f, 18));
            g.DrawLine(&pen, P(14, 12), P(13.5f, 18));
            break;
        }
        case Icon::Copy: {
            g.DrawRectangle(&pen, X(4), Y(7), u * 12, u * 13);
            g.DrawRectangle(&pen, X(8), Y(3.5f), u * 12, u * 13);
            break;
        }
        case Icon::Front:
        case Icon::Back: {
            bool front = icon == Icon::Front;
            if (front) {
                g.DrawRectangle(&pen, X(7), Y(3.5f), u * 12, u * 12);
                SolidBrush soft(RGBA(COL_ACCENT, 40));
                g.FillRectangle(&soft, X(4), Y(8.5f), u * 12, u * 12);
                g.DrawRectangle(&pen, X(4), Y(8.5f), u * 12, u * 12);
            } else {
                g.DrawRectangle(&pen, X(4), Y(3.5f), u * 12, u * 12);
                SolidBrush soft(RGBA(COL_ACCENT, 40));
                g.FillRectangle(&soft, X(8), Y(8.5f), u * 12, u * 12);
                g.DrawRectangle(&pen, X(8), Y(8.5f), u * 12, u * 12);
            }
            break;
        }
        case Icon::Fit: {
            auto corner = [&](float x, float y, float dx, float dy) {
                g.DrawLine(&pen, P(x, y), P(x + dx * 4, y));
                g.DrawLine(&pen, P(x, y), P(x, y + dy * 4));
            };
            corner(3.5f, 3.5f, 1, 1);
            corner(20.5f, 3.5f, -1, 1);
            corner(3.5f, 20.5f, 1, -1);
            corner(20.5f, 20.5f, -1, -1);
            g.DrawRectangle(&pen, X(8), Y(8), u * 8, u * 8);
            break;
        }
        case Icon::Minus: {
            g.DrawLine(&pen, P(4.5f, 12), P(19.5f, 12));
            break;
        }
        case Icon::Check: {
            drawCheckmark(g, b, color, width);
            break;
        }
        case Icon::Page: {
            GraphicsPath p;
            p.AddLine(P(5, 3), P(15, 3));
            p.AddLine(P(15, 3), P(20, 8));
            p.AddLine(P(20, 8), P(20, 21));
            p.AddLine(P(20, 21), P(5, 21));
            p.CloseFigure();
            g.DrawPath(&pen, &p);
            g.DrawLine(&pen, P(15, 3), P(15, 8));
            g.DrawLine(&pen, P(15, 8), P(20, 8));
            g.DrawLine(&pen, P(8.5f, 13), P(16, 13));
            g.DrawLine(&pen, P(8.5f, 17), P(14, 17));
            break;
        }
        case Icon::Function: {
            g.DrawLine(&pen, P(4, 19), P(20, 19));
            g.DrawLine(&pen, P(6, 21), P(6, 4));
            GraphicsPath path;
            path.AddBezier(P(8, 17), P(10, 12), P(13, 6), P(19, 5));
            g.DrawPath(&pen, &path);
            SolidBrush dot(RGBA(color, alpha));
            g.FillEllipse(&dot, X(8) - u * 1.1f, Y(17) - u * 1.1f, u * 2.2f, u * 2.2f);
            break;
        }
        case Icon::Settings: {
            g.DrawLine(&pen, P(4, 7), P(20, 7));
            g.DrawLine(&pen, P(4, 12), P(20, 12));
            g.DrawLine(&pen, P(4, 17), P(20, 17));
            SolidBrush knob(RGBA(color, alpha));
            g.FillEllipse(&knob, X(7) - u * 1.7f, Y(7) - u * 1.7f, u * 3.4f, u * 3.4f);
            g.FillEllipse(&knob, X(14) - u * 1.7f, Y(10) - u * 1.7f, u * 3.4f, u * 3.4f);
            g.FillEllipse(&knob, X(9) - u * 1.7f, Y(15) - u * 1.7f, u * 3.4f, u * 3.4f);
            break;
        }
    }
}

int getEncoderClsid(const WCHAR* format, CLSID* clsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    std::vector<BYTE> buf(size);
    ImageCodecInfo* info = (ImageCodecInfo*)buf.data();
    GetImageEncoders(num, size, info);
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(info[i].MimeType, format) == 0) {
            *clsid = info[i].Clsid;
            return (int)i;
        }
    }
    return -1;
}

bool saveBitmapPng(Bitmap* bmp, const std::wstring& path) {
    CLSID clsid;
    if (getEncoderClsid(L"image/png", &clsid) < 0) return false;
    return bmp->Save(path.c_str(), &clsid, nullptr) == Ok;
}

std::string encodePngBase64(Bitmap* bmp) {
    if (!bmp) return {};
    CLSID clsid;
    if (getEncoderClsid(L"image/png", &clsid) < 0) return {};
    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) return {};
    std::string out;
    if (bmp->Save(stream, &clsid, nullptr) == Ok) {
        STATSTG stat{};
        if (SUCCEEDED(stream->Stat(&stat, STATFLAG_NONAME))) {
            LARGE_INTEGER zero{};
            stream->Seek(zero, STREAM_SEEK_SET, nullptr);
            ULONG remaining = (ULONG)stat.cbSize.QuadPart;
            std::string raw;
            raw.resize(remaining);
            ULONG read = 0;
            if (remaining < 64u * 1024u * 1024u) {
                stream->Read(raw.data(), remaining, &read);
                raw.resize(read);
                out = b64encode(raw);
            }
        }
    }
    stream->Release();
    return out;
}


// ---------------------------------------------------------------------------
// tiny math expression parser for the function plotter
// ---------------------------------------------------------------------------
namespace {

struct FNode {
    enum Type { Num, Var, Bin, Unary, Call } type = Num;
    double v = 0.0;
    char op = 0;
    int fn = 0;
    std::unique_ptr<FNode> a, b;
};

enum FnId { FnSin, FnCos, FnTan, FnAsin, FnAcos, FnAtan, FnSqrt, FnAbs,
            FnLn, FnLog, FnExp, FnFloor, FnCeil, FnRound };

class FParser {
public:
    explicit FParser(const std::string& text) : s_(text) {}
    std::unique_ptr<FNode> parse(std::string& err) {
        skip();
        auto n = expr();
        skip();
        if (!err_.empty()) { err = err_; return nullptr; }
        if (p_ != s_.size()) { err = "无法识别的字符: " + s_.substr(p_, 1); return nullptr; }
        return n;
    }
private:
    const std::string& s_;
    size_t p_ = 0;
    std::string err_;

    void skip() { while (p_ < s_.size() && std::isspace((unsigned char)s_[p_])) ++p_; }
    bool eat(char c) { skip(); if (p_ < s_.size() && s_[p_] == c) { ++p_; return true; } return false; }
    char peek() { skip(); return p_ < s_.size() ? s_[p_] : '\0'; }
    bool startsPrimary(char c) const {
        return std::isdigit((unsigned char)c) || std::isalpha((unsigned char)c) || c == '(' || c == '.';
    }
    std::unique_ptr<FNode> binary(char op, std::unique_ptr<FNode> a, std::unique_ptr<FNode> b) {
        auto n = std::make_unique<FNode>();
        n->type = FNode::Bin; n->op = op; n->a = std::move(a); n->b = std::move(b);
        return n;
    }
    std::unique_ptr<FNode> expr() {
        auto a = term();
        while (err_.empty()) {
            char c = peek();
            if (c != '+' && c != '-') break;
            ++p_;
            auto b = term();
            a = binary(c, std::move(a), std::move(b));
        }
        return a;
    }
    std::unique_ptr<FNode> term() {
        auto a = unary();
        while (err_.empty()) {
            char c = peek();
            if (c == '*' || c == '/') {
                ++p_;
                auto b = unary();
                a = binary(c, std::move(a), std::move(b));
            } else if (startsPrimary(c)) {
                auto b = unary();
                a = binary('*', std::move(a), std::move(b));
            } else {
                break;
            }
        }
        return a;
    }
    std::unique_ptr<FNode> unary() {
        char c = peek();
        if (c == '+' || c == '-') {
            ++p_;
            auto n = std::make_unique<FNode>();
            n->type = FNode::Unary; n->op = c; n->a = unary();
            return n;
        }
        return power();
    }
    std::unique_ptr<FNode> power() {
        auto a = primary();
        if (err_.empty() && eat('^')) {
            auto b = unary();
            a = binary('^', std::move(a), std::move(b));
        }
        return a;
    }
    std::unique_ptr<FNode> primary() {
        skip();
        if (p_ >= s_.size()) { err_ = "表达式不完整"; return nullptr; }
        char c = s_[p_];
        if (c == '(') {
            ++p_;
            auto n = expr();
            if (!eat(')')) err_ = "缺少右括号 )";
            return n;
        }
        if (std::isdigit((unsigned char)c) || c == '.') {
            const char* begin = s_.c_str() + p_;
            char* end = nullptr;
            double v = std::strtod(begin, &end);
            if (end == begin) { err_ = "数字格式错误"; return nullptr; }
            p_ += (size_t)(end - begin);
            auto n = std::make_unique<FNode>();
            n->type = FNode::Num; n->v = v;
            return n;
        }
        if (std::isalpha((unsigned char)c)) {
            size_t start = p_;
            while (p_ < s_.size() && (std::isalnum((unsigned char)s_[p_]) || s_[p_] == '_')) ++p_;
            std::string id = s_.substr(start, p_ - start);
            for (auto& ch : id) ch = (char)std::tolower((unsigned char)ch);
            auto n = std::make_unique<FNode>();
            if (id == "x") {
                n->type = FNode::Var;
                if (p_ < s_.size() && (s_[p_] == '2' || s_[p_] == '3')) {
                    char d = s_[p_++];
                    auto e = std::make_unique<FNode>();
                    e->type = FNode::Num;
                    e->v = d == '2' ? 2.0 : 3.0;
                    return binary('^', std::move(n), std::move(e));
                }
                return n;
            }
            if (id == "pi") { n->type = FNode::Num; n->v = 3.14159265358979323846; return n; }
            if (id == "e") { n->type = FNode::Num; n->v = 2.71828182845904523536; return n; }
            int fn = -1;
            if (id == "sin") fn = FnSin; else if (id == "cos") fn = FnCos;
            else if (id == "tan") fn = FnTan; else if (id == "asin") fn = FnAsin;
            else if (id == "acos") fn = FnAcos; else if (id == "atan") fn = FnAtan;
            else if (id == "sqrt") fn = FnSqrt; else if (id == "abs") fn = FnAbs;
            else if (id == "ln") fn = FnLn; else if (id == "log") fn = FnLog;
            else if (id == "exp") fn = FnExp; else if (id == "floor") fn = FnFloor;
            else if (id == "ceil") fn = FnCeil; else if (id == "round") fn = FnRound;
            if (fn < 0) { err_ = "未知函数或变量: " + id; return nullptr; }
            if (!eat('(')) { err_ = "函数 " + id + " 需要括号"; return nullptr; }
            auto arg = expr();
            if (!eat(')')) err_ = "函数 " + id + " 缺少右括号";
            n->type = FNode::Call; n->fn = fn; n->a = std::move(arg);
            return n;
        }
        err_ = std::string("无法识别的字符: ") + c;
        return nullptr;
    }
};

double evalF(const FNode* n, double x) {
    if (!n) return std::numeric_limits<double>::quiet_NaN();
    switch (n->type) {
        case FNode::Num: return n->v;
        case FNode::Var: return x;
        case FNode::Unary: {
            double v = evalF(n->a.get(), x);
            return n->op == '-' ? -v : v;
        }
        case FNode::Bin: {
            double a = evalF(n->a.get(), x), b = evalF(n->b.get(), x);
            switch (n->op) {
                case '+': return a + b;
                case '-': return a - b;
                case '*': return a * b;
                case '/': return b == 0.0 ? std::numeric_limits<double>::quiet_NaN() : a / b;
                case '^': return std::pow(a, b);
            }
            return std::numeric_limits<double>::quiet_NaN();
        }
        case FNode::Call: {
            double v = evalF(n->a.get(), x);
            switch (n->fn) {
                case FnSin: return std::sin(v);
                case FnCos: return std::cos(v);
                case FnTan: return std::tan(v);
                case FnAsin: return (v < -1 || v > 1) ? std::numeric_limits<double>::quiet_NaN() : std::asin(v);
                case FnAcos: return (v < -1 || v > 1) ? std::numeric_limits<double>::quiet_NaN() : std::acos(v);
                case FnAtan: return std::atan(v);
                case FnSqrt: return v < 0 ? std::numeric_limits<double>::quiet_NaN() : std::sqrt(v);
                case FnAbs: return std::fabs(v);
                case FnLn: return v <= 0 ? std::numeric_limits<double>::quiet_NaN() : std::log(v);
                case FnLog: return v <= 0 ? std::numeric_limits<double>::quiet_NaN() : std::log10(v);
                case FnExp: return std::exp(v);
                case FnFloor: return std::floor(v);
                case FnCeil: return std::ceil(v);
                case FnRound: return std::round(v);
            }
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

}  // namespace

bool sampleExpression(const std::string& expr, double xmin, double xmax, int count,
                      std::vector<std::pair<double, double>>& out, std::string& error) {
    out.clear();
    error.clear();
    std::string text = expr;
    auto replaceAll = [&](const std::string& from, const std::string& to) {
        size_t at = 0;
        while ((at = text.find(from, at)) != std::string::npos) {
            text.replace(at, from.size(), to);
            at += to.size();
        }
    };
    replaceAll("\xC2\xB2", "^2");
    replaceAll("\xC2\xB3", "^3");
    replaceAll("\xC3\x97", "*");
    replaceAll("\xC3\xB7", "/");
    replaceAll("\xE2\x88\x92", "-");
    replaceAll("\xCF\x80", "pi");
    size_t a = text.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) { error = "请输入函数解析式"; return false; }
    text = text.substr(a);
    if (text.rfind("y=", 0) == 0) text = text.substr(2);
    else if (text.rfind("f(x)=", 0) == 0) text = text.substr(5);
    FParser parser(text);
    auto root = parser.parse(error);
    if (!root) return false;
    count = std::max(2, std::min(count, 5000));
    double step = (xmax - xmin) / count;
    out.reserve((size_t)count + 1);
    for (int i = 0; i <= count; ++i) {
        double x = xmin + step * i;
        double y = evalF(root.get(), x);
        if (std::isfinite(y) && std::fabs(y) < 1.0e12) out.push_back({x, y});
    }
    if (out.size() < 2) { error = "这个解析式在范围内没有可绘制的点"; out.clear(); return false; }
    return true;
}

}  // namespace qhb
