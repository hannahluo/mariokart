// Simple routines for doing delays

// Written by Bernie Roehl, February 2017

static __inline void DelayTwoMicroseconds() {
	int i;
	for (i = 0; i < 1; ++i);
}

static __inline void DelayOneMillisecond() {
	int i;
	for (i = 0; i < 2200; ++i);
}

static __inline void DelayMilliseconds(unsigned int n) {
	int i;
	for (i = 0; i < n; ++i)
		DelayOneMillisecond();
}
