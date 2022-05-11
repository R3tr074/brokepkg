#ifndef UTILS_H
#define UTILS_H

#include "config.h"

#ifdef DEBUG
#define PR_DEBUG(...) pr_debug(__VA_ARGS__)
#define PR_INFO(...) pr_info(__VA_ARGS__)
#else
#define PR_DEBUG
#define PR_INFO
#endif

#endif