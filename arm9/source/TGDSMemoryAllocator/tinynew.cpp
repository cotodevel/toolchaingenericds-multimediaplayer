//Coto: What this very very important file does, is to redirect new / delete operators from C++'s standard dmalloc, 
//into XMEM malloc which is 100% stable and known to work correctly.
//Which also happens to be the ARM9 C++ environment.
/*
The MIT License (MIT)

Copyright (c) 2014 ARMstrap Community

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */
#include <new>
#include <malloc.h>
#include "libutilsShared.h"
#include "posixHandleTGDS.h"

void* operator new(std::size_t size) {
    return TGDSARM9Malloc(size);
}

void* operator new[](std::size_t size) {
    return TGDSARM9Malloc(size);
}

void operator delete(void* ptr) {
    TGDSARM9Free(ptr);
}

void operator delete[](void* ptr) {
    TGDSARM9Free(ptr);
}

/* Optionally you can override the 'nothrow' versions as well.
   This is useful if you want to catch failed allocs with your
   own debug code, or keep track of heap usage for example,
   rather than just eliminate exceptions.
 */

void* operator new(std::size_t size, const std::nothrow_t&) {
    return TGDSARM9Malloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) {
    return TGDSARM9Malloc(size);
}

void operator delete(void* ptr, const std::nothrow_t&) {
    TGDSARM9Free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) {
    TGDSARM9Free(ptr);
}

extern "C" void __cxa_pure_virtual()
{
	while(1);
}