
extern "C" {
	void cosim_6502(Vcentipede_verilator *top, unsigned long long _time,
			int reset, unsigned int pc, unsigned int sp, unsigned int sr,
			unsigned int a, unsigned int x, unsigned int y);

	void cosim_int_event(unsigned int pc, int on);
}

