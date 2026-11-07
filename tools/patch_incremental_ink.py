from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\qhb.h")
s = p.read_text(encoding="utf-8")
s = s.replace("    bool liveDirtyValid_ = false;", "    bool liveDirtyValid_ = false;\n    size_t liveInkDrawn_ = 0;")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
s = s.replace("""    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    liveDirtyValid_ = false;
    SetCapture(hwnd_);""",
"""    liveStroke_.points.push_back({clampf(world.x, 1, PAGE_W - 1), clampf(world.y, 1, PAGE_H - 1)});
    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;
    SetCapture(hwnd_);""")
s = s.replace("""    drag_ = DragMode::None;
    liveDirtyValid_ = false;""",
"""    drag_ = DragMode::None;
    liveDirtyValid_ = false;
    liveInkDrawn_ = 0;""")
p.write_text(s, encoding="utf-8", newline="\n")

p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\render.cpp")
s = p.read_text(encoding="utf-8")
start = s.index("    if (drag_ == DragMode::Draw && dirty && !baseDirty_ && base_ &&")
end = s.index("    if (drag_ == DragMode::Move && sel_ >= 0", start)
new = r'''    if (drag_ == DragMode::Draw && !baseDirty_ && base_ &&
        base_->GetWidth() == W_ && base_->GetHeight() == H_) {
        Graphics g(back_.get());
        setupGraphics(g);
        g.SetCompositingQuality(CompositingQualityHighSpeed);
        g.SetInterpolationMode(InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(PixelOffsetModeHalf);
        size_t n = liveStroke_.points.size();
        if (liveInkDrawn_ == 0 || liveInkDrawn_ > n) {
            drawFastStroke(g, liveStroke_, pageXf());
            liveInkDrawn_ = n;
        } else if (n > liveInkDrawn_) {
            Item tail = liveStroke_;
            size_t begin = liveInkDrawn_ > 0 ? liveInkDrawn_ - 1 : 0;
            tail.points.assign(liveStroke_.points.begin() + begin, liveStroke_.points.end());
            drawFastStroke(g, tail, pageXf());
            liveInkDrawn_ = n;
        }
        presentBitmap(hdc, back_.get(), W_, H_);
        liveDirtyValid_ = false;
        return;
    }
'''
s = s[:start] + new + s[end:]
# Full live path should mark all drawn
s = s.replace("""            drawLive(g);
            drawToast(g);
        }
    } else {""",
"""            drawLive(g);
            drawToast(g);
        }
        liveInkDrawn_ = liveStroke_.points.size();
    } else {""")
p.write_text(s, encoding="utf-8", newline="\n")
print("incremental live ink")
