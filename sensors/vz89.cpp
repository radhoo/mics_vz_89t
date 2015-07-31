/*
    File:       VZ89.h
    Version:    0.1.0
    Date:       July, 2015
	License:	GPL v2

	Mics-VZ-89T sensor

    ****************************************************************************
    Copyright (C) 2015 Radu Motisan  <radu.motisan@gmail.com>

	http://www.pocketmagic.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    ****************************************************************************
 */


#include "VZ89.h"


// i2c write
void VZ89::writemem(uint8_t reg, uint8_t value) {
	i2c_start_wait(VZ89_ADDR | I2C_WRITE);
	i2c_write(reg);
	i2c_write(value);
	i2c_stop();
}

// i2c read
void VZ89::readmem(uint8_t reg, uint8_t buff[], uint8_t bytes) {
	uint8_t i =0;
	i2c_start_wait(VZ89_ADDR | I2C_WRITE);
	i2c_write(reg);
	i2c_rep_start(VZ89_ADDR | I2C_READ);
	for(i=0; i<bytes; i++) {
		if(i==bytes-1)
			buff[i] = i2c_readNak();
		else
			buff[i] = i2c_readAck();
	}
	i2c_stop();
}

void VZ89::readRaw(uint8_t rawData[6]) {
	static uint8_t buff[6];
	memset(buff, 0, sizeof(buff));

	//read raw temperature
	readmem(VZ89_CMD_GETSTATUS, rawData, 6);
}

/*
During “Functional Test Mode” only “Raw sensor” and “VOC_short” data are available. “VOC_short” is
an image of sensor reactivity and can then be used for functional test.
Out of this initial period, the device will have the I2C data CO2 equivalent [ppm] and tVOC equivalent
referred to the isobutylene sensitivity unit [ppb].

D1:Data_byte_1: CO2_equ: [13…242] -> CO2_equ [ppm] = (D1 -13) * (1600/229) + 400
D2: Data_byte_2: VOC_short [13…242]
D3: Data_byte_3: tVOC: [13…242] -> tVOC [ppb] = (D3 -13) * (1000/229)
D4: Data_byte_4: Raw sensor first byte (LSB)
D5: Data_byte_5: Raw sensor second byte
D6: Data_byte_6: Raw sensor third byte (MSB) -> Resistor value [W] = 10*(D4 + (256*D5) + (65536*D6))

return CO2 equivalent [ppm] and tVOC equivalent referred to the isobutylene sensitivity unit [ppb].
 *
 */
bool VZ89::read(float *co2, uint8_t *reactivity, float *tvoc) {
	uint8_t data[6];
	readRaw(data);
	if (data[0] < 13 || data[1] < 13 || data[2] < 13) return false;
	// convert data to meaningful values
	*co2 = (data[0] - 13) * (1600.0 / 229) + 400; // ppm: 400 .. 2000
	*reactivity = data[1];
	*tvoc = (data[2] - 13) * (1000.0/229); // ppb: 0 .. 1000

	//uint32_t resistor = 10 * (data[3] +256 * data[4] + 65536 * data[5]);
	return true;
}

