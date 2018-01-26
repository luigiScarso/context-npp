
#ifndef _IDALLOCATOR_H
#define _IDALLOCATOR_H

#include "StdAfx.h"

/// Code was copied from python script plugin.
class IDAllocator
{
public:
	virtual bool allocate(size_t quantity, idx_t *start) = 0;
};

#endif

