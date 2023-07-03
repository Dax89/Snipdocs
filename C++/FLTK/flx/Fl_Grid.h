#include <FL/Fl_Table.H>

namespace flx {

class Fl_Grid: public Fl_Table
{
public:
    Fl_Grid(int x, int y, int w, int h);

protected:
    void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0) override;
};

} // namespace flx
