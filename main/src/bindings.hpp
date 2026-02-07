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
}

namespace BindingsParticles
{
    void registerAll(Interpreter &vm);
}