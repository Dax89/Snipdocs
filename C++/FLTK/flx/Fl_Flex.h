#pragma once

#include <FL/Fl_Group.H>

namespace flx {

class Fl_Flex: public Fl_Group
{
public:
    static constexpr int ROW = 0;
    static constexpr int COLUMN = 0;

public:
    Fl_Flex(int direction);
    Fl_Flex(int w, int h, int direction);
    Fl_Flex(int x, int y, int w, int h, int direction);
    void resize(int x, int y, int w, int h) override;
    void direction(int d); 
    inline int direction() const { return m_direction; }

private:
    void resize_column();
    void resize_row();

private:
    int m_direction;
};

} // namespace flx
