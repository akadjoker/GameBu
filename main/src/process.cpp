#include "bindings.hpp"
#include "engine.hpp"
#include "math.hpp"
#include "interpreter.hpp"
#include <raylib.h>
extern GraphLib gGraphLib;
extern Scene gScene;

namespace BindingsProcess
{

    double get_distx(double a, double d)
    {
        double angulo = (double)a * RAD;
        return ((double)(cos(angulo) * d));
    }

    double get_disty(double a, double d)
    {
        double angulo = (double)a * RAD;
        return (-(double)(sin(angulo) * d));
    }

    int native_advance(Interpreter *vm, Process *proc, int argCount, Value *args)
    {

        if (argCount != 1)
        {
            Error("advance expects 1 argument speed");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("advance expects a number argument speed ");
            return 0;
        }

        double distance = args[0].asNumber();

        double x = proc->privates[0].asNumber();
        double y = proc->privates[1].asNumber();
        double angle = -proc->privates[4].asNumber();

        x += get_distx(angle, distance);
        y += get_disty(angle, distance);

        proc->privates[0] = vm->makeDouble(x);
        proc->privates[1] = vm->makeDouble(y);
        //  proc->privates[4] = vm->makeDouble(angle);

        return 0;
    }

    int native_xadvance(Interpreter *vm, Process *proc, int argCount, Value *args)
    {

        if (argCount != 2)
        {
            Error("xadvance expects 2 arguments speed and angle");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("xadvance expects 2 number arguments speed and angle");
            return 0;
        }
        double distance = args[0].asNumber();
        double angle = -args[1].asNumber();

        double x = proc->privates[0].asNumber();
        double y = proc->privates[1].asNumber();

        x += get_distx(angle, distance);
        y += get_disty(angle, distance);

        proc->privates[0] = vm->makeDouble(x);
        proc->privates[1] = vm->makeDouble(y);
        //        proc->privates[4] = vm->makeDouble(angle);

        return 0;
    }

    int native_get_point(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("get_point expects 1 argument pointIndex");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }
        if (!args[0].isNumber())
        {
            Error("get_point expects a number argument pointIndex");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }

        int pointIndex = (int)args[0].asNumber();

        if (proc->userData == nullptr)
        {
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }

        Entity *entity = (Entity *)proc->userData;
        int graphId = entity->graph;

        Graph *g = gGraphLib.getGraph(graphId);
        if (!g || pointIndex < 0 || pointIndex >= (int)g->points.size())
        {
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }
        Vector2 point = g->points[pointIndex];

        vm->pushDouble(point.x);
        vm->pushDouble(point.y);
        return 2;
    }

    int native_get_real_point(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("get_real_point expects 1 argument pointIndex");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }
        if (!args[0].isNumber())
        {
            Error("get_real_point expects a number argument pointIndex");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }

        int pointIndex = (int)args[0].asNumber();

        if (proc->userData == nullptr)
        {
            vm->pushDouble(0);
            vm->pushDouble(0);

            return 2;
        }

        Entity *entity = (Entity *)proc->userData;
        Vector2 realPoint = entity->getRealPoint(pointIndex);

        //        Info("get_real_point: pointIndex=%d realX=%f realY=%f", pointIndex, realPoint.x, realPoint.y);

        //DrawCircleLines((int)realPoint.x, (int)realPoint.y, 5, RED);

        vm->pushDouble(realPoint.x);
        vm->pushDouble(realPoint.y);
        return 2;
    }

    static Entity *requireEntity(Process *proc, const char *funcName)
    {
        if (!proc || !proc->userData)
        {
            Error("%s process has no associated entity!", funcName);
            return nullptr;
        }
        return (Entity *)proc->userData;
    }

    int native_set_rect_shape(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 4)
        {
            Error("set_rect_shape expects 4 arguments (x, y, w, h)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber())
        {
            Error("set_rect_shape expects 4 number arguments (x, y, w, h)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_rect_shape");
        if (!entity)
            return 0;

        int x = (int)args[0].asNumber();
        int y = (int)args[1].asNumber();
        int w = (int)args[2].asNumber();
        int h = (int)args[3].asNumber();
        entity->setRectangleShape(x, y, w, h);
        return 0;
    }

    int native_set_circle_shape(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 1)
        {
            Error("set_circle_shape expects 1 argument (radius)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("set_circle_shape expects 1 number argument (radius)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_circle_shape");
        if (!entity)
            return 0;

        float radius = (float)args[0].asNumber();
        entity->setCircleShape(radius);
        return 0;
    }

    int native_set_collision_layer(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 1)
        {
            Error("set_collision_layer expects 1 argument (layer)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("set_collision_layer expects 1 number argument (layer)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_collision_layer");
        if (!entity)
            return 0;

        int layer = (int)args[0].asNumber();
        entity->setCollisionLayer(layer);
        return 0;
    }

    int native_set_collision_mask(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 1)
        {
            Error("set_collision_mask expects 1 argument (mask)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("set_collision_mask expects 1 number argument (mask)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_collision_mask");
        if (!entity)
            return 0;

        uint32 mask = (uint32)args[0].asNumber();
        entity->setCollisionMask(mask);
        return 0;
    }

    int native_add_collision_mask(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 1)
        {
            Error("add_collision_mask expects 1 argument (layer)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("add_collision_mask expects 1 number argument (layer)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "add_collision_mask");
        if (!entity)
            return 0;

        int layer = (int)args[0].asNumber();
        entity->addCollisionMask(layer);
        return 0;
    }

    int native_remove_collision_mask(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        if (argCount != 1)
        {
            Error("remove_collision_mask expects 1 argument (layer)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("remove_collision_mask expects 1 number argument (layer)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "remove_collision_mask");
        if (!entity)
            return 0;

        int layer = (int)args[0].asNumber();
        entity->removeCollisionMask(layer);
        return 0;
    }

    int native_set_static(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        (void)args;
        if (argCount != 0)
        {
            Error("set_static expects 0 arguments");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_static");
        if (!entity)
            return 0;

        entity->setStatic();
        return 0;
    }

    int native_enable_collision(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        (void)args;
        if (argCount != 0)
        {
            Error("enable_collision expects 0 arguments");
            return 0;
        }

        Entity *entity = requireEntity(proc, "enable_collision");
        if (!entity)
            return 0;

        entity->enableCollision();
        return 0;
    }

    int native_disable_collision(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        (void)vm;
        (void)args;
        if (argCount != 0)
        {
            Error("disable_collision expects 0 arguments");
            return 0;
        }

        Entity *entity = requireEntity(proc, "disable_collision");
        if (!entity)
            return 0;

        entity->disableCollision();
        return 0;
    }

    int native_place_free(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("place_free expects 2 arguments (x, y)");
            return 0;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("place_free expects 2 number arguments (x, y)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "place_free");
        if (!entity)
            return 0;

        double x = args[0].asNumber();
        double y = args[1].asNumber();
        bool free = entity->place_free(x, y);
        vm->pushBool(free);
        return 1;
    }

    int native_place_meeting(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("place_meeting expects 2 arguments (x, y)");
            vm->pushInt(-1);
            return 1;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("place_meeting expects 2 number arguments (x, y)");
             vm->pushInt(-1);
            return 1;
        }

        Entity *entity = requireEntity(proc, "place_meeting");
        if (!entity)
        {
             vm->pushInt(-1);
                return 1;
        }
         
        double x = args[0].asNumber();
        double y = args[1].asNumber();
        Entity *hit = entity->place_meeting(x, y);
        if (!hit)
        {
            vm->pushInt(-1);
            return 1;
        }

//        vm->push(hit->procID);
        vm->pushInt(hit->procID);

        return 1;
    }

    int native_atach(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("atach expects 2 arguments (childProcID,front)");
            return 0;
        }
        if (!args[0].isProcess())
        {
            Error("atach expects 1 integer argument (childProcID)  get %s", valueTypeToString(args[0].type));
            return 0;
        }

        bool front = args[1].asBool();
        

        
        
        Entity *entity = requireEntity(proc, "atach");
        if (!entity) return 0;
        
        int childProcID = args[0].as.integer;
        
         Process *childProc = vm->findProcessById(childProcID);
         if (!childProc)
         {
             Error("atach: no process found with ID %d", childProcID);
             return 0;
         }
         if (!childProc->userData)
         {
             Error("atach: child process %d has no associated entity!", childProcID);
             return 0;
         }

         Entity *childEntity = (Entity *)childProc->userData;
         gScene.moveEntityToParent(childEntity,entity,front);

        return 0;
    }

     int native_get_world_point(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("get_world_point expects 2 arguments (x, y)");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }
        if (!args[0].isNumber() || !args[1].isNumber())
        {
            Error("get_world_point expects 2 number arguments (x, y)");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;   
        }

        if (proc->userData == nullptr)
        {
            vm->pushDouble(0);
            vm->pushDouble(0);

            return 2;
        }

        Entity *entity = (Entity *)proc->userData;
        Vector2 worldPoint = entity->getWorldPoint(args[0].asNumber(), args[1].asNumber());

        vm->pushDouble(worldPoint.x);
        vm->pushDouble(worldPoint.y);
        return 2;
    }
     int native_get_local_point(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("get_local_point expects 2 arguments (x, y)");
            vm->pushDouble(0);
            vm->pushDouble(0);
            return 2;
        }
        

        if (proc->userData == nullptr)
        {
            vm->pushDouble(0);
            vm->pushDouble(0);

            return 2;
        }

        Entity *entity = (Entity *)proc->userData;
        
        Vector2 localPoint = entity->getLocalPoint(args[0].asNumber(), args[1].asNumber());

        //        Info("get_real_point: pointIndex=%d realX=%f realY=%f", pointIndex, realPoint.x, realPoint.y);

        //DrawCircleLines((int)localPoint.x, (int)localPoint.y, 5, RED);

        vm->pushDouble(localPoint.x);
        vm->pushDouble(localPoint.y);
        return 2;
    }

    int native_out_screen(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("out_screen expects 0 arguments");
            vm->pushBool(false);
            return 1;
        }

        Entity *entity = requireEntity(proc, "out_screen");
        if (!entity)
        {
            vm->pushBool(false);
            return 1;
        }

        vm->pushBool(gScene.IsOutOfScreen(entity));
        return 1;
    }

    int native_set_layer(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("set_layer expects 1 argument (layer)");
            return 0;
        }
        if (!args[0].isNumber())
        {
            Error("set_layer expects 1 number argument (layer)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_layer");
        if (!entity)
            return 0;

        int layer = (int)args[0].asNumber();
        entity->layer = layer;
        return 0;
    }

    int native_get_layer(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 0)
        {
            Error("get_layer expects 0 arguments");
            vm->pushInt(0);
            return 1;
        }

        Entity *entity = requireEntity(proc, "get_layer");
        if (!entity)
        {
            vm->pushInt(0);
            return 1;
        }

        vm->pushInt(entity->layer);
        return 1;
    }
 
    int native_mirror_vertical(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("flip_vertical expects 1 arguments");
            return 0;
        }

        Entity *entity = requireEntity(proc, "flip_vertical");
        if (!entity)
            return 0;

        bool flip = args[0].asBool();

        entity->flip_y= flip;
        return 0;
    }

    int native_mirror_horizontal(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("flip_horizontal expects 1 arguments");
            return 0;
        }

        Entity *entity = requireEntity(proc, "flip_horizontal");
        if (!entity)
            return 0;

        bool flip = args[0].asBool();

        entity->flip_x = flip;
        return 0;
    }

    int native_set_visible(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 1)
        {
            Error("set_visible expects 1 arguments");
            return 0;
        }

        Entity *entity = requireEntity(proc, "set_visible");
        if (!entity)
            return 0;

        bool visible = args[0].asBool();

        if (visible)
            entity->flags |= B_VISIBLE;
        else
            entity->flags &= ~B_VISIBLE;

        return 0;
    }

    int native_flip(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 2)
        {
            Error("flip expects 2 arguments (flipX, flipY)");
            return 0;
        }

        Entity *entity = requireEntity(proc, "flip");
        if (!entity)
            return 0;

        bool flipX = args[0].asBool();
        bool flipY = args[1].asBool();

        entity->flip_x = flipX;
        entity->flip_y = flipY;

        return 0;
    }
 
    


 


    int native_collision(Interpreter *vm, Process *proc, int argCount, Value *args)
    {
        if (argCount != 3 || !args[0].isString() || !args[1].isNumber() || !args[2].isNumber())
        {
            vm->pushInt(-1);
            return 1;
        }

        Entity *entity = requireEntity(proc, "collision");
        if (!entity || !entity->shape || !(entity->flags & B_COLLISION) || !entity->ready)
        {
            vm->pushInt(-1);
            return 1;
        }

      
        double x = args[1].asNumber();
        double y = args[2].asNumber();

        // Guarda posição original
        double old_x = entity->x, old_y = entity->y;
        entity->x = x;
        entity->y = y;
        entity->markTransformDirty();
        entity->updateBounds();

        // Itera processos vivos do tipo pedido
        const auto &alive = vm->getAliveProcesses();
        for (size_t i = 0; i < alive.size(); i++)
        {
            Process *other = alive[i];
            if (!other || other == proc) continue;
            if (!other->userData) continue;
            if (!compare_strings(other->name, args[0].asString())) continue;

            Entity *otherEntity = (Entity *)other->userData;
            if (!otherEntity || !otherEntity->shape) continue;
            if (!(otherEntity->flags & B_COLLISION)) continue;
            if (otherEntity->flags & B_DEAD) continue;

            if (CheckCollisionRecs(entity->getBounds(), otherEntity->getBounds()))
            {
                if (entity->intersects(otherEntity))
                {
                    entity->x = old_x;
                    entity->y = old_y;
                    entity->markTransformDirty();
                    entity->bounds_dirty = true;
                    vm->pushInt(other->id);
                    return 1;
                }
            }
        }

        // Restaura posição
        entity->x = old_x;
        entity->y = old_y;
        entity->markTransformDirty();
        entity->bounds_dirty = true;

        vm->pushInt(-1);
        return 1;
    }

    void registerAll(Interpreter &vm)
    {
        vm.registerNativeProcess("advance", native_advance, 1);
        vm.registerNativeProcess("xadvance", native_xadvance, 2);
        vm.registerNativeProcess("get_point", native_get_point, 1);
        vm.registerNativeProcess("get_real_point", native_get_real_point, 1);
        vm.registerNativeProcess("set_rect_shape", native_set_rect_shape, 4);
        vm.registerNativeProcess("get_local_point", native_get_local_point, 2);
        vm.registerNativeProcess("get_world_point", native_get_world_point, 2);
        vm.registerNativeProcess("set_circle_shape", native_set_circle_shape, 1);
        vm.registerNativeProcess("set_collision_layer", native_set_collision_layer, 1);
        vm.registerNativeProcess("set_collision_mask", native_set_collision_mask, 1);
        vm.registerNativeProcess("add_collision_mask", native_add_collision_mask, 1);
        vm.registerNativeProcess("remove_collision_mask", native_remove_collision_mask, 1);
        vm.registerNativeProcess("set_static", native_set_static, 0);
        vm.registerNativeProcess("enable_collision", native_enable_collision, 0);
        vm.registerNativeProcess("disable_collision", native_disable_collision, 0);
        vm.registerNativeProcess("place_free", native_place_free, 2);
        vm.registerNativeProcess("place_meeting", native_place_meeting, 2);
        vm.registerNativeProcess("collision", native_collision, 3);
        vm.registerNativeProcess("atach", native_atach, 2);
        vm.registerNativeProcess("out_screen", native_out_screen, 0);
        vm.registerNativeProcess("set_layer", native_set_layer, 1);
        vm.registerNativeProcess("get_layer", native_get_layer, 0);

        vm.registerNativeProcess("set_collision_layer", native_set_collision_layer, 1);
        vm.registerNativeProcess("set_collision_mask", native_set_collision_mask, 1);
        vm.registerNativeProcess("add_collision_mask", native_add_collision_mask, 1);
        vm.registerNativeProcess("remove_collision_mask", native_remove_collision_mask, 1);
        vm.registerNativeProcess("set_static", native_set_static, 0);
        vm.registerNativeProcess("enable_collision", native_enable_collision, 0);
        vm.registerNativeProcess("disable_collision", native_disable_collision, 0);

        vm.registerNativeProcess("flip_vertical", native_mirror_vertical, 1);
        vm.registerNativeProcess("flip_horizontal", native_mirror_horizontal, 1);
        vm.registerNativeProcess("set_visible", native_set_visible, 1);
        vm.registerNativeProcess("flip", native_flip, 2);



    }
}
