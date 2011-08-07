MODULE_NAME = uClibcxx
MODULE_DESCRIPTION = This module provides a minimalistic c++ runtime library for use in other modules. This does not provide functionality by itself, instead it is used to allow for the c++ language to be used in other modules. Please note that rtti and exceptions has both been disabled. 
MODULE_AUTHOR = 
MODULE_VERSION = "0.2.2"
MODULE_COMPAT_VERSION = "0.2.2"
MODULE_START = $(MODULE_NAME)_start

DIR = uClibc++
# sstream fstream
MODULE_OBJS = uClibc++ abi algorithm associative_base bitset \
		del_op del_opnt del_opv del_opvnt \
		deque eh_alloc eh_globals exception \
		func_exception iomanip  char_traits \
		iterator limits list locale \
		map new_handler new_op new_opnt new_opv \
		new_opvnt numeric queue set \
		stack stdexcept streambuf string typeinfo \
		utility valarray vector support \
		ios iostream istream ostream 
#temporarily disabled for ppc 
I386_OBJS = complex

include ../MakeInc.dir