#include "Fl_Grid.h"

namespace flx {

Fl_Grid::Fl_Grid(int x, int y, int w, int h): Fl_Table{x, y, w, h} { }

void Fl_Grid::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
    switch(context)
    {
        case Fl_Table::CONTEXT_RC_RESIZE: {
            int index = 0, cellx, celly, cellw, cellh;

            for(int r = 0; r < this->rows(); r++) {
                for(int c = 0; c < this->cols(); c++) {
                    if (index >= this->children()) break;
                    this->find_cell(CONTEXT_TABLE, r, c, cellx, celly, cellw, cellh);
                    this->child(index++)->resize(cellx, celly, cellw, cellh);
                }
            }

            this->init_sizes();  
            break;
        }

        default:
            break;
    }
}

} // namespace flx
