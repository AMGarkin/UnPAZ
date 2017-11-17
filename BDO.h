#pragma once

#include <stdint.h>
#include <cstring>


namespace BDO
{
	uint32_t decompress(uint8_t *in, uint8_t *out);
//	uint32_t calculatePackCRC(uint8_t *data, uint32_t length);
}
