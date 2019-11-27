//
// Created by 赵明明 on 2019/11/27.
//

#ifndef _LINKAGE_H
#define _LINKAGE_H

#define SYMBOL_NAME(X) X

#define SYMBOL_NAME_LABEL(X) X##:

#define ENTRY(name)         \
.global SYMBOL_NAME(name);  \
SYMBOL_NAME_LABEL(name)

#endif //_LINKAGE_H
