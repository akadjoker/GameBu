#pragma once

#include "interpreter.hpp"

namespace Bindings
{
    void registerAll(Interpreter &vm);
}
namespace BindingsInput
{
    void registerAll(Interpreter &vm);
}

namespace BindingsProcess
{
    void registerAll(Interpreter &vm);
}

namespace BindingsDraw
{
    void registerAll(Interpreter &vm);
    void RenderWorldCommands();
    void RenderScreenCommands();
    void resetDrawCommands();
   
    void addLineCommand(int x1, int y1, int x2, int y2, bool screenSpace);
    void addTextCommand(String *text, int x, int y, int size, bool screenSpace);
    void addCircleCommand(int centerX, int centerY, int radius, bool fill, bool screenSpace);
    void addRectangleCommand(int x, int y, int width, int height, bool fill, bool screenSpace);
}

namespace BindingsParticles
{
    void registerAll(Interpreter &vm);
}