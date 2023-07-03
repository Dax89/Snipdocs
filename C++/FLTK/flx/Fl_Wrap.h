#pragma once

#include <FL/Fl_Widget.H>
#include <functional>

namespace flx {

template<typename Widget>
class FL_Wrap
{
public:
    using Callback = std::function<void(Fl_Widget*)>;

public:
    FL_Wrap() = default;
    FL_Wrap(int x, int y, int w, int h, const char* L = nullptr) : m_widget{new Widget(x, y, w, h, L)} { }
    Widget* operator->() const { return m_widget; }
    Widget* operator*() const { return m_widget; }
    operator bool() const { return m_widget; }

    void callback(Callback&& callback) {
        m_callback = std::move(callback);

        m_widget->callback([](Fl_Widget* sender, void* userdata) {
            auto* self = reinterpret_cast<FL_Wrap*>(userdata);
            if(self->m_callback) self->m_callback(sender);
        }, this);
    }

private:
    Widget* m_widget{nullptr};
    Callback m_callback;
};

template<typename Widget>
FL_Wrap<Widget> make_widget(int x, int y, int w, int h, const char* L = nullptr) { 
    return FL_Wrap<Widget>{x, y, w, h, L};
}

} // namespace flx
