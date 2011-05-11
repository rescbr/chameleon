/*
 * Cadet-petit Armel <armelcadetpetit@gmail.com>
 *
 *  rand & srand implementation
 */

#include "libsa.h"

#ifndef __RAND_H
#define __RAND_H
extern int rand (void);
extern void srand (unsigned int seed);
#endif /* !__RAND_H */