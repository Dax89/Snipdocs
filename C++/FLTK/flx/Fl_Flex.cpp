#include "Fl_Flex.h"
#include <cmath>

namespace flx {

Fl_Flex::Fl_Flex(int direction): Fl_Group{0, 0, 0, 0}, m_direction{direction} { }
Fl_Flex::Fl_Flex(int w, int h, int direction): Fl_Group{0, 0, w, h}, m_direction{direction} { }
Fl_Flex::Fl_Flex(int x, int y, int w, int h, int direction): Fl_Group{x, y, w, h}, m_direction{direction} { }

void Fl_Flex::direction(int d)
{
    if(m_direction == d) return;

    m_direction = d;
    this->resize(this->x(), this->y(), this->w(), this->h());
    this->redraw();
}

void Fl_Flex::resize(int x, int y, int w, int h)
{
    Fl_Group::resize(x, y, w, h);

    if(m_direction == Fl_Flex::COLUMN)
        this->resize_column();
    else
        this->resize_row();
}

void Fl_Flex::resize_column()
{
    int nc = this->children(), y = 0;
    if(nc <= 0) return;

    int h = std::ceil(this->h() / nc);

    for(int i = 0; i < nc; ++i)
    {
        Fl_Widget* c = this->child(i);

        c->resize(0, y, this->w(), h);
        y += h;
    }
}

void Fl_Flex::resize_row()
{
    int nc = this->children(), x = 0;
    if(nc <= 0) return;

    int w = std::ceil(this->w() / nc);

    for(int i = 0; i < nc; i++)
    {
        Fl_Widget* c = this->child(i);
        c->resize(x, 0, w, this->h());
        x += w;
    }
}

} // namespace flx
