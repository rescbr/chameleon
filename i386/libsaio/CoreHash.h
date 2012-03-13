//
//  CoreHash.h
//
//  Created by Cadet-petit Armel <armelcadetpetit@gmail.com> on 08/03/12.
//  Copyright (c) 2012 . All rights reserved.
//
// CoreHash allows in few lines to take full advantages of hashable structures.

#ifndef CoreHash_h
#define CoreHash_h

#include "uthash.h"

//CoreHash Header and reserved fields
#define CoreHashHeader              \
        char name[10];              \
        int  id;                    \
        UT_hash_handle hh;          /* makes this structure hashable */


#define __CHFindStrVar(HObj)                                                                \
static struct HObj * HObj##_FindStrVar(const char *name, struct HObj *container) {          \
    struct HObj *var;                                                                       \
                                                                                            \
	if (setjmp(h_buf_error) == -1) {                                                        \
                                                                                            \
		return NULL;                                                                        \
	} else {                                                                                \
		HASH_FIND_STR( container, name, var );                                              \
	}                                                                                       \
    return var;                                                                             \
}

#define __CHNewStrVar(HObj)                                                                 \
static struct HObj * HObj##_NewStrVar(const char *name, struct HObj **container ) {         \
    struct HObj *var;                                                                       \
                                                                                            \
    var = (struct HObj*)malloc(sizeof(struct HObj));                                        \
    if (!var)                                                                               \
        return NULL;                                                                        \
                                                                                            \
    strlcpy(var->name, name, sizeof(var->name));											\
                                                                                            \
	if (setjmp(h_buf_error) == -1) {                                                        \
        																					\
		free(var);                                                                          \
		return NULL;                                                                        \
	} else {                                                                                \
		HASH_ADD_STR( *container, name, var );                                              \
																							\
	}                                                                                       \
	return var;																				\
}

#define __CHFindStrVarOrCreate(HObj)															\
static struct HObj * HObj##_FindStrVarOrCreate(const char *name, struct HObj *container )		\
{																								\
	struct HObj *var;																			\
																								\
        var = HObj##_FindStrVar(name,container);												\
																								\
    if (!var) {																					\
       HObj##_NewStrVar(name,&container);                                                       \
    }																							\
	return var;																					\
}

#define __CHDeleteStrVar(HObj)                                                  \
static void HObj##_DeleteStrVar(const char *name, struct HObj *container ) {    \
        struct HObj *var;                                                       \
                                                                                \
        var = HObj##_FindStrVar(name,container);                                \
                                                                                \
        if (!var) {                                                             \
            return;                                                             \
        }                                                                       \
                                                                                \
	if (setjmp(h_buf_error) == -1) {                                            \
		return;                                                                 \
	} else {                                                                    \
		HASH_DEL( container, var);                                              \
	}                                                                           \
    free(var);                                                                  \
}

#define CHInitStr(HObj)        \
__CHFindStrVar(HObj)           \
__CHNewStrVar(HObj)            \
__CHFindStrVarOrCreate(HObj)   \
__CHDeleteStrVar(HObj)         

       

//CoreHash Int Implementation

#define __CHFindIntVar(HObj)                                                                \
static struct HObj * HObj##_FindIntVar(int id, struct HObj *container) {					\
    struct HObj *var;                                                                       \
                                                                                            \
    if (setjmp(h_buf_error) == -1) {                                                        \
																							\
        return NULL;                                                                        \
                } else {                                                                    \
        HASH_FIND_INT( container, &id, var );                                               \
    }                                                                                       \
    return var;                                                                             \
}

#define __CHNewIntVar(HObj)                                                                 \
static struct HObj * HObj##_NewIntVar(int id, struct HObj **container ) {					\
     struct HObj *var;                                                                      \
																							\
     var = (struct HObj*)malloc(sizeof(struct HObj));                                       \
     if (!var)                                                                              \
            return NULL;                                                                    \
																							\
     var->id = id;																			\
																							\
     if (setjmp(h_buf_error) == -1) {                                                       \
																							\
            free(var);                                                                      \
            return NULL;                                                                    \
     } else {                                                                               \
            HASH_ADD_INT( *container, id, var );                                            \
                                                                                            \
     }                                                                                      \
     return var;																			\
}

#define __CHFindIntVarOrCreate(HObj)                                                        \
static struct HObj * HObj##_FindIntVarOrCreate(int id, struct HObj *container )				\
{                                                                                           \
    struct HObj *var;                                                                       \
                                                                                            \
    var = HObj##_FindIntVar(id,container);													\
                                                                                            \
    if (!var) {                                                                             \
        HObj##_NewIntVar(id,&container);                                                    \
    }                                                                                       \
    return var;                                                                             \
}

#define __CHDeleteIntVar(HObj)														\
static void HObj##_DeleteIntVar(int id, struct HObj *container ) {					\
   struct HObj *var;																\
																					\
   var = HObj##_FindIntVar(id,container);											\
																					\
   if (!var) {																		\
        return;																		\
   }																				\
																					\
   if (setjmp(h_buf_error) == -1) {													\
        return;																		\
   } else {																			\
        HASH_DEL( container, var);													\
   }																				\
   free(var);																		\
}

#define CHInitInt(HObj)            \
    __CHFindIntVar(HObj)           \
    __CHNewIntVar(HObj)            \
    __CHFindIntVarOrCreate(HObj)   \
    __CHDeleteIntVar(HObj)   
    

             

// CoreHash Common Implementation


#define __CHDebug(HObj)                                                                                 \
static void HObj##_Debug(struct HObj *container )                                                       \
{                                                                                                       \
	struct HObj *current_var;                                                                           \
	printf("Var list: \n");																				\
	for(current_var=container;current_var;current_var=(struct HObj*)(current_var->hh.next))             \
	{                                                                                                   \
		printf(" Name = %s , id = %d\n",current_var->name, current_var->id);                            \
	}                                                                                                   \
}


#define __CHDeleteAll(HObj)                                     \
static void HObj##_DeleteAll(struct HObj *container ) {         \
	struct HObj *current_var, *tmp;                             \
																\
	if (setjmp(h_buf_error) == -1) {                            \
	return;														\
	} else {                                                    \
			HASH_ITER(hh, container, current_var, tmp) {        \
			HASH_DEL(container,current_var);                    \
			free(current_var);                                  \
		}                                                       \
	}                                                           \
}

#define CHUnInit(HObj)		\
	__CHDeleteAll(HObj)  

#define CHDebug(HObj)       \
	__CHDebug(HObj)

#endif
