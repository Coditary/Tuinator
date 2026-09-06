#include <tuinator/core/inline_view.hpp>

#include <cstdio>
#include <iostream>

namespace tuinator {

void flush_cli_output() {
    std::cout.flush();
    std::cerr.flush();
    std::fflush(stdout);
    std::fflush(stderr);
}

namespace {

class InlineRegionShell : public Widget {
public:
    explicit InlineRegionShell(std::unique_ptr<Widget> child) { add_child(std::move(child)); }

    Size preferred_size() const override { return children_.front()->preferred_size(); }
    bool wants_full_screen() const override { return true; }

    void layout(Rect bounds) override {
        bounds_ = bounds;
        children_.front()->layout(bounds);
    }

    void paint(PaintContext& ctx) const override { children_.front()->paint(ctx); }

    bool handle_event(const Event& event) override { return children_.front()->handle_event(event); }

    void collect_focusable(std::vector<Widget*>& out) override {
        children_.front()->collect_focusable(out);
    }

    void for_each_child(const std::function<void(Widget*)>& visitor) override {
        visitor(children_.front().get());
    }

    void for_each_descendant(const std::function<void(Widget*)>& visitor) override {
        children_.front()->for_each_descendant(visitor);
    }

    Widget* hit_test(Point point) override { return children_.front()->hit_test(point); }
    Widget* hit_test_focusable(Point point) override {
        return children_.front()->hit_test_focusable(point);
    }
};

} // namespace

InlineView::InlineView(InlineBackendOptions options)
    : app_(InlineTerminalBackend::create(std::move(options))) {}

InlineView::~InlineView() {
    finish();
}

void InlineView::set_root(std::unique_ptr<Widget> root) {
    app_.set_root(std::make_unique<InlineRegionShell>(std::move(root)));
}

void InlineView::begin_inline_output() {
    if (output_committed_) {
        return;
    }

    flush_cli_output();
    output_committed_ = true;
}

void InlineView::start() {
    begin_inline_output();
    app_.present();
}

void InlineView::present() {
    begin_inline_output();
    app_.present();
}

int InlineView::run() {
    begin_inline_output();
    return app_.run();
}

void InlineView::finish() {
    if (finished_) {
        return;
    }

    finished_ = true;
    app_.shutdown_terminal();
}

int InlineView::anchor_row() const {
    if (const auto* inline_backend = dynamic_cast<const InlineTerminalBackend*>(&app_.backend())) {
        return inline_backend->anchor_row();
    }
    return 0;
}

} // namespace tuinator
