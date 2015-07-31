float co2 = 0, tvoc = 0;
uint8_t reactivity = 0;
// repeat until successful read: TODO: add timeout for safety
while (!vz89.read(&co2, &reactivity, &tvoc))
	_delay_ms(1500);
			