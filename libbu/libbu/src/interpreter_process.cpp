#include "interpreter.hpp"
#include "pool.hpp"

static uint64_t PROCESS_IDS = 0;

void ProcessDef::finalize()
{

    for (int i = 0; i < totalFibers; i++)
    {
        Fiber *fiber = &fibers[i];

        if (fiber->frameCount > 0)
        {
            for (int j = 0; j < fiber->frameCount; j++)
            {
                CallFrame *frame = &fiber->frames[j];
                if (frame->func && frame->ip == nullptr)
                {
                    frame->ip = frame->func->chunk->code;
                }
            }

            if (fiber->ip == nullptr && fiber->frames[0].func)
            {
                fiber->ip = fiber->frames[0].func->chunk->code;
            }
        }
    }
}
void ProcessDef::release()
{
    if (fibers)
    {
        free(fibers);
        fibers = nullptr;
    }
}

void Process::reset()
{
    this->id = 0;
    this->exitCode = 0;
    this->initialized = false;

    if (fibers)
    {
        for (int i = 0; i < totalFibers; i++)
        {
            fibers[i].state = FiberState::DEAD;
            fibers[i].stackTop = fibers[i].stack;
            fibers[i].frameCount = 0;
            fibers[i].ip = nullptr;
            fibers[i].resumeTime = 0;
            fibers[i].gosubTop = 0;
        }
    }
    totalFibers = 0;
    name = nullptr;

    state = FiberState::DEAD; // Estado do PROCESSO (frame)
    resumeTime = 0.0f;        // Quando acorda (frame)
    nextFiberIndex = 0;
    currentFiberIndex = 0;
    current = nullptr;
}

int Interpreter::getProcessPrivateIndex(const char *name)
{
    
    switch (name[0])
    {
    case 'x':
        return (name[1] == '\0') ? 0 : -1;
    case 'y':
        return (name[1] == '\0') ? 1 : -1;
    case 'z':
        return (name[1] == '\0') ? 2 : -1;
    case 'g':
        if (strcmp(name, "graph") == 0) return 3;
        if (strcmp(name, "green") == 0) return 10;
        if (strcmp(name, "group") == 0) return 16;
        return -1;
    case 'a':
        if (strcmp(name, "angle") == 0) return 4;
        if (strcmp(name, "alpha") == 0) return 12;
        return -1;
    case 's':
        if (strcmp(name, "size") == 0) return 5;
        if (strcmp(name, "state") == 0) return 14;
        if (strcmp(name, "speed") == 0) return 15;
        return -1;
    case 'f':
        if (strcmp(name, "flags") == 0) return 6;
        if (strcmp(name, "father") == 0) return 8;
        return -1;
    case 'i':
        return (strcmp(name, "id") == 0) ? 7 : -1;
    case 'r':
        return (strcmp(name, "red") == 0) ? 9 : -1;
    case 'b':
        return (strcmp(name, "blue") == 0) ? 11 : -1;
    case 't':
        return (strcmp(name, "tag") == 0) ? 13 : -1;
    }
    // 'g' handles both "graph" and "green" and "group"
    return -1;
}

ProcessDef *Interpreter::addProcess(const char *name, Function *func, int totalFibers)
{
    String *pName = createString(name);
    ProcessDef *existing = nullptr;
    if (processesMap.get(pName, &existing))
    {

        return existing;
    }

    ProcessDef *proc = new ProcessDef();

    if (proc == nullptr)
    {
        runtimeError("Critical: Out of memory creating process!");
        return nullptr;
    }

    proc->name = pName;
    proc->index = processes.size();

    proc->privates[0] = makeDouble(0); // x
    proc->privates[1] = makeDouble(0); // y
    proc->privates[2] = makeInt(0); // z
    proc->privates[3] = makeInt(-1);    // graph
    proc->privates[4] = makeInt(0);    // angle
    proc->privates[5] = makeInt(100);  // size
    proc->privates[6] = makeInt(0);    // flags
    proc->privates[7] = makeInt(-1);   // id
    proc->privates[8] = makeInt(-1);   // father
    proc->privates[9] = makeInt(255);  // red
    proc->privates[10] = makeInt(255); // green
    proc->privates[11] = makeInt(255); // blue
    proc->privates[12] = makeInt(255); // alpha
    proc->privates[13] = makeInt(0);   // tag
    proc->privates[14] = makeInt(0);   // state
    proc->privates[15] = makeDouble(0); // speed
    proc->privates[16] = makeInt(0);   // group
    proc->totalFibers = totalFibers;

    proc->fibers = (Fiber *)calloc(proc->totalFibers, sizeof(Fiber));

    if (proc->fibers == nullptr)
    {
        runtimeError("Critical: Out of memory creating process!");
        return nullptr;
    }

    for (int i = 0; i < proc->totalFibers; i++)
    {
        proc->fibers[i].state = FiberState::DEAD;
        proc->fibers[i].resumeTime = 0;
        proc->fibers[i].stackTop = proc->fibers[i].stack;
        proc->fibers[i].frameCount = 0;
        proc->fibers[i].ip = nullptr;
        proc->fibers[i].gosubTop = 0;
    }

    initFiber(&proc->fibers[0], func);

    processesMap.set(pName, proc);
    processes.push(proc);
    return proc;
}

Process *Interpreter::spawnProcess(ProcessDef *blueprint)
{
    Process *instance = ProcessPool::instance().create();

    if (instance == nullptr)
    {
        runtimeError("Critical: Out of memory spawning process!");
        return nullptr;
    }

    instance->name = blueprint->name;
    instance->id = PROCESS_IDS++;
    instance->state = FiberState::RUNNING;
    instance->resumeTime = 0;
    instance->nextFiberIndex = 1;
    instance->currentFiberIndex = 0;
    instance->current = nullptr;
    instance->initialized = false;
    instance->exitCode = 0;
  

    if (instance->fibers == nullptr || instance->totalFibers != blueprint->totalFibers)
    {
        if (instance->fibers) 
        {
            free(instance->fibers);
        }
        instance->totalFibers = blueprint->totalFibers;
        instance->fibers = (Fiber*)calloc(instance->totalFibers, sizeof(Fiber));        
        if (!instance->fibers)
        {
            runtimeError("Failed to allocate fibers!");
            ProcessPool::instance().recycle(instance);
            return nullptr;
        }
    }
    else
    {
        // REUTILIZA! 
        // Fibers já estão null por reset()
    }

    // Clona privates
    for (int i = 0; i < MAX_PRIVATES; i++)
    {
        instance->privates[i] = blueprint->privates[i];
    }

    for (int i = 0; i < blueprint->totalFibers; i++)
    {
        Fiber *srcFiber = &blueprint->fibers[i];
        Fiber *dstFiber = &instance->fibers[i];

        if (srcFiber->state == FiberState::DEAD)
        {
            dstFiber->state = FiberState::DEAD;
            dstFiber->stackTop = dstFiber->stack; // tack no início
            dstFiber->frameCount = 0;
            dstFiber->ip = nullptr;
            dstFiber->resumeTime = 0;
            dstFiber->gosubTop = 0;
            continue; // lixo!
        }

        // Copia estado
        dstFiber->state = srcFiber->state;
        dstFiber->resumeTime = srcFiber->resumeTime;
        dstFiber->frameCount = srcFiber->frameCount;

        // Copia stack
        size_t stackSize = srcFiber->stackTop - srcFiber->stack;
        if (stackSize > 0)
        {
            memcpy(dstFiber->stack, srcFiber->stack, stackSize * sizeof(Value));
        }
        dstFiber->stackTop = dstFiber->stack + stackSize;

        // Copia Gosub
        dstFiber->gosubTop = srcFiber->gosubTop;
        if (srcFiber->gosubTop > 0)
        {
            memcpy(dstFiber->gosubStack, srcFiber->gosubStack,
                   srcFiber->gosubTop * sizeof(uint8 *));
        }

        // Copia frames
        for (int j = 0; j < srcFiber->frameCount; j++)
        {
            dstFiber->frames[j].func = srcFiber->frames[j].func;
            dstFiber->frames[j].closure = srcFiber->frames[j].closure;

            // IP
            if (srcFiber->frames[j].ip != nullptr)
            {
                dstFiber->frames[j].ip = srcFiber->frames[j].ip;
            }
            else if (dstFiber->frames[j].func && dstFiber->frames[j].func->chunk)
            {
                dstFiber->frames[j].ip = dstFiber->frames[j].func->chunk->code;
            }
            else
            {
                dstFiber->frames[j].ip = nullptr;
            }

            // Slots
            ptrdiff_t offset = srcFiber->frames[j].slots - srcFiber->stack;
            dstFiber->frames[j].slots = dstFiber->stack + offset;
        }

        // IP da fiber
        if (dstFiber->frameCount > 0)
        {
            dstFiber->ip = dstFiber->frames[dstFiber->frameCount - 1].ip;
        }
        else
        {
            dstFiber->ip = nullptr;
        }
    }

    instance->current = &instance->fibers[0];
    aliveProcesses.push(instance);

    return instance;
}
uint32 Interpreter::getTotalProcesses() const
{
    return static_cast<uint32>(processes.size());
}

uint32 Interpreter::getTotalAliveProcesses() const
{\
    return uint32(aliveProcesses.size());
}


void Interpreter::killAliveProcess()
{
    if (aliveProcesses.size() == 1)
        return;

    for (size_t i = 1; i < aliveProcesses.size(); i++)
    {
        Process *proc = aliveProcesses[i];
        if (proc)
        {
            proc->state = FiberState::DEAD;
        }
    }
    return;
}

Process *Interpreter::findProcessById(uint32 id)
{
    for (size_t i = 0; i < aliveProcesses.size(); i++)
    {
        Process *proc = aliveProcesses[i];
        if (proc && proc->id == id)
            return proc;
    }
    return nullptr;
}

void Interpreter::update(float deltaTime)
{
    // if(    asEnded)
    //     return;
    currentTime += deltaTime;
    lastFrameTime = deltaTime;
    frameCount++;

    size_t i = 0;
    while (i < aliveProcesses.size())
    {
        Process *proc = aliveProcesses[i];

        // Suspended?
        if (proc->state == FiberState::SUSPENDED)
        {
            if (currentTime >= proc->resumeTime)
                proc->state = FiberState::RUNNING;
            else
            {
                i++;
                continue;
            }
        }

        // Dead? -> remove da lista
        if (proc->state == FiberState::DEAD)
        {
            // remove sem manter ordem
            //   Info(" Process (id=%u) is dead. Cleaning up. ",   proc->id);
            aliveProcesses[i] = aliveProcesses.back();
            cleanProcesses.push(proc);
            aliveProcesses.pop();
            continue;
        }

        currentProcess = proc;
        if (!currentProcess)
        {
            i++;
            continue;
        }

        run_process_step(proc);
        if (hooks.onUpdate)
            hooks.onUpdate(this,proc, deltaTime);

        i++;
    }

    for (size_t j = 0; j < cleanProcesses.size(); j++)
    {
        Process *proc = cleanProcesses[j];
        if (hooks.onDestroy)
            hooks.onDestroy(this,proc, proc->exitCode);

        if (currentProcess == proc)
        {
            currentProcess = nullptr;
        }
        if (currentFiber && proc->fibers &&
            currentFiber >= proc->fibers &&
            currentFiber < proc->fibers + proc->totalFibers)
        {
            currentFiber = nullptr;
        }

        ProcessPool::instance().recycle(proc);
    }
    cleanProcesses.clear();

    if (frameCount % 300 == 0)
    {
        size_t poolSize = ProcessPool::instance().size();
        
        if (poolSize > ProcessPool::MIN_POOL_SIZE * 2)
        {
            Info("Pool has %zu processes, shrinking...", poolSize);
            ProcessPool::instance().shrink();
        }
    }
}

void Interpreter::run_process_step(Process *proc)
{
    if (proc->fibers == nullptr || proc->state == FiberState::DEAD)
    {
        return;
    }

    Fiber *fiber = get_ready_fiber(proc);
    if (!fiber)
    {

        //   Warning("No ready fiber");
        proc->state = FiberState::DEAD;
        return;
    }

    currentProcess = proc;
    currentFiber = fiber;

    proc->current = fiber;
    FiberResult result = run_fiber(fiber, proc);

    // printf("  Executing Fiber %d\n", fiber - proc->fibers);
    //  Warning("  [run_process_step] result.reason=%d, instructions=%d",   (int)result.reason, result.instructionsRun);

   // currentFiber = nullptr;

    if (proc->state == FiberState::DEAD)
    {
        proc->initialized = false;
        return;
    }

    if (result.reason == FiberResult::FIBER_YIELD)
    {
        fiber->state = FiberState::SUSPENDED;
        fiber->resumeTime = currentTime + result.yieldMs / 1000.0f;
        return;
    }

    if (result.reason == FiberResult::PROCESS_FRAME)
    {
        proc->state = FiberState::SUSPENDED;
        //proc->resumeTime = currentTime;
        proc->resumeTime = currentTime + (lastFrameTime * (result.framePercent - 100) / 100.0f);

        // proc->frameCounter = result.framePercent / 100;
        if (!proc->initialized)
        {
            proc->initialized = true;
             if (hooks.onStart)
                hooks.onStart(this,proc);
        }

        return;
    }

    if (result.reason == FiberResult::FIBER_DONE)
    {
        fiber->state = FiberState::DEAD;
        // Warning("  [run_process_step] Fiber DONE");
        return;
    }
}

void Interpreter::render()
{
    if (!hooks.onRender)
        return;
    for (size_t i = 0; i < aliveProcesses.size(); i++)
    {
        Process *proc = aliveProcesses[i];
        if (proc->state != FiberState::DEAD && proc->initialized)
        {
            hooks.onRender(this,proc);
        }
    }
}
