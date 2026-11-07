from pathlib import Path
p = Path(r"C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\src\app.cpp")
s = p.read_text(encoding="utf-8")
start = s.index("void App::eraseAt(Pt world) {")
end = s.index("void App::startLaser(Pt screen) {", start)
new = r'''void App::eraseAt(Pt world) {
    const float tol = eraserSize_ / 2.0f;
    const float tol2 = tol * tol;
    std::vector<Item> out;
    out.reserve(page().items.size() + 4);
    bool changed = false;

    for (const Item& item : page().items) {
        if (item.kind == ItemKind::Stroke && item.points.size() > 1) {
            std::vector<Pt> run;
            std::vector<std::vector<Pt>> pieces;
            bool cut = false;
            for (const Pt& p : item.points) {
                float dx = p.x - world.x, dy = p.y - world.y;
                if (dx * dx + dy * dy <= tol2) {
                    cut = true;
                    if (run.size() >= 2) pieces.push_back(run);
                    run.clear();
                } else {
                    run.push_back(p);
                }
            }
            if (!cut) {
                out.push_back(item);
                continue;
            }
            if (run.size() >= 2) pieces.push_back(run);
            changed = true;
            for (auto& piece : pieces) {
                Item ni = item;
                ni.id = nextId_++;
                ni.points = std::move(piece);
                out.push_back(std::move(ni));
            }
            continue;
        }

        if (item.kind == ItemKind::Shape && item.shape == ShapeKind::Line) {
            float ax = item.x1, ay = item.y1, bx = item.x2, by = item.y2;
            float vx = bx - ax, vy = by - ay;
            float len2 = vx * vx + vy * vy;
            if (len2 > 0.0001f) {
                float t = ((world.x - ax) * vx + (world.y - ay) * vy) / len2;
                t = clampf(t, 0.0f, 1.0f);
                float cx = ax + vx * t, cy = ay + vy * t;
                float dx = cx - world.x, dy = cy - world.y;
                if (dx * dx + dy * dy <= tol2) {
                    changed = true;
                    float len = std::sqrt(len2);
                    float half = std::sqrt(std::max(0.0f, tol2 - (dx * dx + dy * dy))) / len;
                    float t1 = std::max(0.0f, t - half);
                    float t2 = std::min(1.0f, t + half);
                    auto makeLine = [&](float u1, float u2) {
                        if ((u2 - u1) * len <= 2.0f) return;
                        Item ni = item;
                        ni.id = nextId_++;
                        ni.shape = ShapeKind::Line;
                        ni.x1 = ax + vx * u1; ni.y1 = ay + vy * u1;
                        ni.x2 = ax + vx * u2; ni.y2 = ay + vy * u2;
                        out.push_back(std::move(ni));
                    };
                    makeLine(0.0f, t1);
                    makeLine(t2, 1.0f);
                    continue;
                }
            }
            out.push_back(item);
            continue;
        }

        if (item.hit(world.x, world.y, tol)) {
            changed = true;
            continue;
        }
        out.push_back(item);
    }

    if (!changed) return;
    if (!eraserChanged_ && pendingSnapshotValid_) {
        pushUndoSnapshot(pendingSnapshot_);
        eraserChanged_ = true;
    }
    page().items = std::move(out);
    page().touch();
    sel_ = -1;
    markDirty();
    invalidate();
}

'''
s = s[:start] + new + s[end:]
p.write_text(s, encoding="utf-8", newline="\n")
print("partial eraser implemented")
