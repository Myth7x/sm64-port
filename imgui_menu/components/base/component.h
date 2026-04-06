#pragma once
#include "imgui.h"

class Component {
public:
    virtual ~Component() = default;
    virtual void init() {}
    virtual void shutdown() {}
    virtual void compose_frame() = 0;
};