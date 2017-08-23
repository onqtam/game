#include "core/Application.h"

int main(int argc, char** argv) {
    Application app;
    return app.run(argc, argv);
}

// TESTING THE NEW CODEGEN

#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/imgui/imgui_stuff.h"

struct A {
    FIELD int omg;
    FIELD int 
        
        omg2;
};

#include <gen/main.cpp.inl>
