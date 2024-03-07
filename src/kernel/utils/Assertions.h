#pragma once
#include <kernel/utils/kprintf.h>

void assertionFailed(const char* msg, const char* file, unsigned line, const char* func);

#define ASSERT(expression) (expression ? (void)0 : assertionFailed(# expression, __FILE__, __LINE__, __PRETTY_FUNCTION__))
