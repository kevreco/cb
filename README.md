> [!CAUTION]
> This project is in a work in progress, the API will change a lot until its first release.

# Table of Contents

- [Introduction](#introduction)
- [Why?](#Why)
- [Examples](#examples)
- [How it works](#how-it-works)
- [Features](#features)
- [Limitations](#limitations)
- [Pros/Cons](#pros-cons)

# CB

CB is an header-only library build system that use the C language to compile C code.

# Introduction

With CB you can describe your C project in a .c file (cb.c) which will be compiled into an executable (cb.exe) that will compile your project (my_project.exe).

Using C language to compile C code prevent to use of heavy dependencies like cmake, premake or any other build system that brings its own language.

Of course, you still need some shell or .bat, .sh file to invoke the C compiler the first time.

# Why?

I used cmake, qmake, qbs, premake, and none of them satisfied me for various reasons.
Sometimes it's because the syntax is a mess (cmake), sometime it's very difficult to make simple API, they use their own language and your spend a significant amount of time fighting the abstraction over the underlaying toolchain.

# Examples

## Static library

Here is a simple example to create a static library.

```c
#define CB_IMPLEMENTATION
#include <cb/cb.h>

int main()
{
    cb_init();

    /* Create "example" project. Every following properties will be assigned to it */
    cb_project("example");
    
    /* Make it a static library. */
    cb_set("binary_type", "static_library");

    /* Add source files. */
    cb_add("files","src/foo.c");
    cb_add("files","src/bar.c");
    
    /* Build the current project based on the default toolchain (C toolchain).
       The resulting binary will be called "example.lib" (msvc) or "libexample.a" (gcc) */
    if(!cb_bake())
    {
        return -1;
    }

    cb_destroy();

    return 0;
}
```

## Dynamic library

Creating a dynamic library is basically the same with a different "binary_type" value.

```c
#define CB_IMPLEMENTATION
#include <cb/cb.h>

int main()
{
    cb_init();

    /* Create "example" project. Every following properties will be assigned to it */
    cb_project("example");
    
    /* Make it a shared library. */
    cb_set("binary_type", "shared_library");

    /* Add source files. */
    cb_add("files","src/foo.c");
    cb_add("files","src/bar.c");
    
    /* Build the current project based on the default toolchain (C toolchain).
       The resulting binary will be called "example.lib" (msvc) or "libexample.a" (gcc) */
    if(!cb_bake())
    {
        return -1;
    }

    cb_destroy();

    return 0;
}
```

## Practical Example

You can see [how CB was added to GLFW](https://github.com/kevreco/cb-glfw/commit/111f6b91dcea890a0bcb9cd195da08379801b00a#diff-12565d127026e73b2981cbe146807bfa33e45d585f9bd08a287b320124bd1b7dR1), which can be used to create the shared or the static library.

## More examples

For more examples you can also look at the [tests/](tests/projects/) folder.

# How it works?

Properties are added to a "project" (source files, c flags, include directories, etc) and those properties are processed depending on the toolchain used.

It should be fairly simple to create a new toolchain that generate a Visual Studio .sln project or a cmake project without changing any of the properties.

# Features

- Support Microsoft Windows with MSVC toolchain.
- Support Linux with gcc toolchain.
- Can build static library, shared library and executable.
- Handle utf-8 names. (not supported for .dlls on Windows, see limitations below).
- Handle extensions.

# Limitations

- This is designed to do full compilation everytime. Maybe we can create and extension in the future to only compile what is needed but it's not the priority at all.
- .dlls on Windows can only contains ANSI characters.

# Pros/Cons

## Pros

- It's using C language. It's debuggable and the availability of C compilers is very high on all common platforms.
- You can use your own C libraries.

## Cons

- It's using C language.