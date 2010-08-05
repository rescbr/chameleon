extern int load_dependency(const char*);

int start() {
	int returnValue = 1;
	if(returnValue) returnValue &= load_dependency("Symbols");
	return returnValue;
}
