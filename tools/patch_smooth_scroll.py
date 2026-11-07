from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")

# page scrollbar down: cancel animation
s = s.replace("""        if (rid == "ov:pages:scroll") {
            pageScrollDrag_ = true;
            pageScrollDragStartY_ = (float)y;
            pageScrollDragStart_ = pageScroll_;
            SetCapture(hwnd_);
            return;
        }""",
"""        if (rid == "ov:pages:scroll") {
            pageScrollDrag_ = true;
            pageScrollAnimating_ = false;
            pageScrollDragStartY_ = (float)y;
            pageScrollDragStart_ = pageScroll_;
            pageScrollFrom_ = pageScrollTo_ = pageScroll_;
            SetCapture(hwnd_);
            return;
        }""")

# wheel: animate page list and canvas
s = s.replace("""    if (overlay_ == 1) {
        pageScroll_ -= (float)delta / 120.0f * S(90);
        pageScroll_ = std::max(0.0f, std::min(pageScroll_, std::max(0.0f, pageScrollMax_)));
        invalidate();
        return;
    }
    if (overlay_ || splash_) return;
    if (dockRect().Contains((float)x, (float)y)) return;
    Pt sp{(float)x, (float)y};
    if (ctrl) {
        float mul = delta > 0 ? 1.12f : 1.0f / 1.12f;
        zoomAt(sp, zoom_ * mul);
    } else {
        float s = viewScale();
        if (shift) camCenter_.x -= (float)delta / 120.0f * 80.0f / s;
        else camCenter_.y -= (float)delta / 120.0f * 80.0f / s;
        clampCam();
        invalidate();
    }
}""",
"""    if (overlay_ == 1) {
        float maxS = std::max(0.0f, pageScrollMax_);
        float base = pageScrollAnimating_ ? pageScrollTo_ : pageScroll_;
        pageScrollFrom_ = pageScroll_;
        pageScrollTo_ = std::max(0.0f, std::min(base - (float)delta / 120.0f * S(110), maxS));
        pageScrollAnimStart_ = GetTickCount64();
        pageScrollAnimating_ = true;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
        return;
    }
    if (overlay_ || splash_) return;
    if (dockRect().Contains((float)x, (float)y)) return;
    Pt sp{(float)x, (float)y};
    if (ctrl) {
        camScrollAnimating_ = false;
        float mul = delta > 0 ? 1.12f : 1.0f / 1.12f;
        zoomAt(sp, zoom_ * mul);
    } else {
        float s = viewScale();
        Pt target = camScrollAnimating_ ? camScrollTo_ : camCenter_;
        if (shift) target.x -= (float)delta / 120.0f * 80.0f / s;
        else target.y -= (float)delta / 120.0f * 80.0f / s;
        Pt saved = camCenter_;
        camCenter_ = target;
        clampCam();
        target = camCenter_;
        camCenter_ = saved;
        camScrollFrom_ = camCenter_;
        camScrollTo_ = target;
        camScrollStart_ = GetTickCount64();
        camScrollAnimating_ = true;
        SetTimer(hwnd_, 5, 16, nullptr);
        invalidate();
    }
}""")

# page scrollbar move sync targets
s = s.replace("""        pageScroll_ = pageScrollDragStart_ +
                      (sp.y - pageScrollDragStartY_) * pageScrollMax_ / denom;
        pageScroll_ = std::max(0.0f, std::min(pageScroll_, pageScrollMax_));
        invalidate();""",
"""        pageScroll_ = pageScrollDragStart_ +
                      (sp.y - pageScrollDragStartY_) * pageScrollMax_ / denom;
        pageScroll_ = std::max(0.0f, std::min(pageScroll_, pageScrollMax_));
        pageScrollFrom_ = pageScrollTo_ = pageScroll_;
        invalidate();""")

# timer 5 scroll easing
s = s.replace("""        if (pageFadeStart_ != 0) {
            if (now - pageFadeStart_ < 220) had = true;
            else pageFadeStart_ = 0;
        }""",
"""        if (pageScrollAnimating_) {
            float t = std::min(1.0f, (float)(now - pageScrollAnimStart_) / 190.0f);
            float e = easeOutCubic(t);
            pageScroll_ = pageScrollFrom_ + (pageScrollTo_ - pageScrollFrom_) * e;
            had = true;
            if (t >= 1.0f) {
                pageScroll_ = pageScrollTo_;
                pageScrollAnimating_ = false;
            }
        }
        if (camScrollAnimating_) {
            float t = std::min(1.0f, (float)(now - camScrollStart_) / 170.0f);
            float e = easeOutCubic(t);
            camCenter_.x = camScrollFrom_.x + (camScrollTo_.x - camScrollFrom_.x) * e;
            camCenter_.y = camScrollFrom_.y + (camScrollTo_.y - camScrollFrom_.y) * e;
            had = true;
            if (t >= 1.0f) {
                camCenter_ = camScrollTo_;
                camScrollAnimating_ = false;
            }
        }
        if (pageFadeStart_ != 0) {
            if (now - pageFadeStart_ < 220) had = true;
            else pageFadeStart_ = 0;
        }""")

p.write_text(s, encoding="utf-8", newline="\n")
print("smooth page and canvas scrolling")
