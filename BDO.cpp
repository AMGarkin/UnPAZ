///copied from https://github.com/kukdh1/PAZ-Unpacker/blob/master/Crypt.cpp created by Donghyun Gouk (kukdh1)

#include "BDO.h"

namespace BDO
{
	/// Decompress codes from quickbms, refined by kukdh1. Comments are mostly mine.
	uint32_t blackdesert_unpack_core(uint8_t *pInput, uint8_t *pOutput, uint32_t uiDecompressedLength, uint8_t *pOutput2, uint32_t pInput_size)
	{
		static const uint8_t dataLengthTable[] = {
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
		};

		uint8_t *pOutputIndex;
		uint8_t *pInputIndex;
		uint32_t uiBlockGroupHeader;
		uint32_t uiCompressedLength;
		uint32_t uiBlockHeader;
		uint8_t *pLastInputIndex;
		uint8_t *pLastOutputIndex;

		pOutputIndex = pOutput;
		uiBlockGroupHeader = 1;
		pLastOutputIndex = uiDecompressedLength + pOutput - 1;

		if (pInput[0] & 0x02) { ///if bit 1 is set, length is stored as long (4 bytes):  ???? ??1?
			uiCompressedLength = (*(uint32_t *)(pInput + 1));
			pInputIndex = pInput + 9; ///skip data header - ID (1 byte), compressed length (4 bytes), decompressed length (4 bytes)
		}
		else { ///if bit 1 is not set, length is stored as byte:                         ???? ??0?
			uiCompressedLength = pInput[1];
			pInputIndex = pInput + 3; ///skip data header - ID (1 byte), compressed length (1 byte), decompressed length (1 byte)
		}

		pLastInputIndex = pInput + uiCompressedLength - 1;

		while (1)
		{
			while (1)
			{
				uint32_t uiRepeatIndex;
				uint32_t uiBlockLength;

				if (uiBlockGroupHeader == 1) ///if block group header is 1, read the next block group header - 0000 0000 0000 0000 0000 0000 0000 0001
				{
					if (pInputIndex + 3 > pLastInputIndex)
						return -1; ///error: Truncated data

					/// get value of block group header
					uiBlockGroupHeader = *(uint32_t *)pInputIndex; ///group can have up to 31 blocks, each bit represents one block.
					pInputIndex += 4; ///skip block group header
				}

				if (pInputIndex + 3 > pLastInputIndex)  /// pInputIndex + 4 > pEndOfInput
					return -2; ///error: Truncated data

				uiBlockHeader = *(uint32_t *)pInputIndex;

				if (!(uiBlockGroupHeader & 1)) ///no data will be duplicated, block group header has not set bit 0 - ???? ???? ???? ???? ???? ???? ???? ???0 (Bits are numbered from right to left starting with 0, it means 31 <- 0)
					break;

				/// get data from compression header and jump to block data?
				if ((uiBlockHeader & 0x03) == 0x03) { ///if bits 0 and 1 are set                                             ???? ???? ???? ???? ???? ???? ???? ??11
					if ((uiBlockHeader & 0x7F) == 3)  ///if bits 1 and 0 are set and bits 2-6 are not set                    ???? ???? ???? ???? ???? ???? ?000 0011
					{
						uiRepeatIndex = uiBlockHeader >> 15; ///uiRepeatIndex is stored in uiBlockHeader here:               XXXX XXXX XXXX XXXX X--- ---- ---- ---- (3-131071) or 2 bytes, 65535?
						uiBlockLength = ((uiBlockHeader >> 7) & 0xFF) + 3; ///uiBlockLength is 3 + number in uiBlockHeader:  ---- ---- ---- ---- -XXX XXXX X--- ---- (1-255)
						pInputIndex += 4; ///header was 4 bytes long, move to the block data
					}
					else
					{
						uiRepeatIndex = (uiBlockHeader >> 7) & 0x1FFFF; ///uiRepeatIndex is stored in uiBlockHeader here:    ---- ---- XXXX XXXX XXXX XXXX X--- ---- (3-131071)
						uiBlockLength = ((uiBlockHeader >> 2) & 0x1F) + 2; ///uiBlockLength is 2 + number in uiBlockHeader:  ---- ---- ---- ---- ---- ---- -XXX XX-- (1-15)
						pInputIndex += 3; ///header was 3 bytes long, move to the block data
					}
				}
				else if ((uiBlockHeader & 0x03) == 0x02) { ///if bit 0 is not set and bit 1 is set                           ???? ???? ???? ???? ???? ???? ???? ??10
					uiRepeatIndex = (uint16_t)uiBlockHeader >> 6; ///convert to 16 bits, uiRepeatIndex is:                   XXXX XXXX XX-- ---- (3-1023)
					uiBlockLength = ((uiBlockHeader >> 2) & 0xF) + 3; ///uiBlockLength is 3 + number in uiBlockHeader:       ---- ---- --XX XX-- (1-15)
					pInputIndex += 2; ///header was 2 bytes long, move to the block data
				}
				else if ((uiBlockHeader & 0x03) == 0x01) { ///if bit 0 is set and bit 1 is not set                           ???? ???? ???? ???? ???? ???? ???? ??01
					uiRepeatIndex = (uint16_t)uiBlockHeader >> 2; ///convert to 16 bits, uiRepeatIndex is:                   XXXX XXXX XXXX XX-- (3-16383)
					uiBlockLength = 3; ///uiBlockLenght is always 3
					pInputIndex += 2; ///header was 2 bytes long, move to the block data
				}
				else { ///if bits 0 and 1 are not set                                                                        ???? ???? ???? ???? ???? ???? ???? ??00
					uiRepeatIndex = (uint8_t)uiBlockHeader >> 2; ///convert to 8 bits, uiRepeatIndex is:                     XXXX XX-- (3-63)
					uiBlockLength = 3; ///uiBlockLenght is always 3
					pInputIndex++; ///header was 1 byte long, move to the block data
				}

				/// uiRepeatIndex points out of the data array (< beginning of array) || uiRepeatIndex points to the same block? || uiBlockLength > left bytes - 3
				if (pOutputIndex - uiRepeatIndex < pOutput2 || uiRepeatIndex < 3u || uiBlockLength > pLastOutputIndex - pOutputIndex - 3u)
					return -3; ///error: Corrupted data.

				uint8_t *ptr = pOutputIndex;

				/// Copy duplicated data
				for (uint32_t i = 0; i < uiBlockLength; i += 3) { ///interesting is that this will read 4 bytes, but pointer is incremented by 3. It means that 4th byte will be always overwritten by the next cycle.
					*(uint32_t *)ptr = *(uint32_t *)(ptr - uiRepeatIndex);
					ptr += 3;
				}

				uiBlockGroupHeader >>= 1;
				pOutputIndex += uiBlockLength;
			}

			/// if 11 ~ 0 bytes left
			if (pOutputIndex >= pLastOutputIndex - 10)
				break;

			int validDataLength = dataLengthTable[uiBlockGroupHeader & 0xF];
			/** dataLengthTable tests last 4 bits of uiBlockGroupHeader as follows (index = uiBlockGroupHeader & 0xF):
			//if (index & 1)	///index = 1 (0001), 3 (0011), 5 (0101), 7 (0111), 9 (1001), 11 (1011), 13 (1101), 15 (1111)
			//	return 0;		///this can never happen, the 1st bit of uiBlockGroupHeader is used to determine if block is repeated, at this part of the code it must be always set to 0.
			//else
			if (index & 2)		///index = 2 (0010),  6 (0110), 10 (1010), 14 (1110)
				return 1;
			else if (index & 4)	///index = 4 (0100), 12 (1100)
				return 2;
			else if (index & 8)	///index = 8 (1000)
				return 3;
			else				///index = 0 (0000)
				return 4;
			**/
			*(uint32_t *)pOutputIndex = uiBlockHeader;
			uiBlockGroupHeader >>= validDataLength;
			pOutputIndex += validDataLength;
			pInputIndex += validDataLength;
		}

		/// Not finished (pOutputIndex == pLastOutputIndex means 1byte left)
		/// Just copy last data
		if (pOutputIndex <= pLastOutputIndex)
		{
			uint8_t *pEndOfInput = pLastInputIndex + 1;

			while (1)
			{
				if (uiBlockGroupHeader == 1)
				{
					pInputIndex += 4;
					uiBlockGroupHeader = 0x80000000;
				}

				if (pInputIndex >= pEndOfInput)
					break;

				*pOutputIndex++ = *pInputIndex++;

				uiBlockGroupHeader >>= 1;

				/// return written length
				if (pOutputIndex > pLastOutputIndex)
					return pOutputIndex - pOutput;
			}

			return -4; ///error: Decompressed data are larger then expected
		}

		/// return written length
		return pOutputIndex - pOutput;
	}

	/// Decompress codes from quickbms, refined by kukdh1
	uint32_t decompress(uint8_t *pInput, uint8_t *pOutput) {
		uint32_t length;

		if (pInput[0] & 0x02) { ///if data header has set bit 1, length in header is stored as long (4 bytes) - ???? ??1?
			length = (*(uint32_t *)(pInput + 5));
		}
		else { ///if data header has not set bit 1, length in header is stored as byte - ???? ??0?
			length = pInput[2];
		}

		if (pInput[0] & 0x01) { ///if data header has set bit 0, data are compressed and will be sent to internal decompression function - ???? ???1
			length = blackdesert_unpack_core(pInput, pOutput, length, pOutput, 0);
		}
		else { ///if data header has not set bit 0, data are not compressed. Skip header and copy data to pOutput. - ???? ???0
			memcpy(pOutput, pInput + (pInput[0] & 0x02 ? 9 : 3), length);
		}

		return length;
	}

/*
	/// CRC calculation codes from Blackdesert_Launcher.exe (KR Client)
	uint32_t calculatePackCRC(uint8_t * data, uint32_t length) {
		int v2; /// ST24_4@4
		unsigned int v3; /// ST20_4@4
		unsigned int v4; /// ST2C_4@4
		int v5; /// ST20_4@4
		unsigned int v6; /// ST24_4@4
		int v7; /// ST2C_4@4
		unsigned int v8; /// ST20_4@4
		int v9; /// ST24_4@4
		unsigned int v10; /// ST2C_4@4
		int v11; /// ST20_4@4
		unsigned int v12; /// ST24_4@4
		unsigned int result; /// eax@18
		int v14; /// ST24_4@23
		unsigned int v15; /// ST20_4@23
		unsigned int v16; /// ST2C_4@23
		int v17; /// ST20_4@23
		unsigned int v18; /// ST24_4@23
		int v19; /// ST2C_4@23
		unsigned int v20; /// ST20_4@23
		int v21; /// ST24_4@23
		unsigned int v22; /// ST2C_4@23
		int v23; /// ST20_4@23
		unsigned int v24; /// ST24_4@23
		int v25; /// ST24_4@41
		unsigned int v26; /// ST20_4@41
		unsigned int v27; /// ST2C_4@41
		int v28; /// ST20_4@41
		unsigned int v29; /// ST24_4@41
		int v30; /// ST2C_4@41
		unsigned int v31; /// ST20_4@41
		int v32; /// ST24_4@41
		unsigned int v33; /// ST2C_4@41
		int v34; /// ST20_4@41
		unsigned int v35; /// ST24_4@41
		unsigned int v36; /// ST20_4@56
		unsigned int v37; /// ST2C_4@56
		unsigned int v38; /// ST24_4@56
		unsigned int v39; /// ST20_4@56
		unsigned int v40; /// ST2C_4@56
		unsigned int v41; /// ST24_4@56
		int *pdwData0; /// [sp+Ch] [bp-24h]@39
		int nBeginValue0; /// [sp+20h] [bp-10h]@1
		unsigned int nBeginValue1; /// [sp+24h] [bp-Ch]@1
		int nBeginValue2; /// [sp+2Ch] [bp-4h]@1

		nBeginValue0 = length - 558228019;
		nBeginValue1 = length - 558228019;
		nBeginValue2 = length - 558228019;
		pdwData0 = (int *)data;
		if (!((uint32_t)data & 3))
		{
			while (length > 0xC)
			{
				v2 = pdwData0[1] + nBeginValue1;
				v3 = pdwData0[2] + nBeginValue0;
				v4 = (*pdwData0 + nBeginValue2 - v3) ^ ((v3 >> 28) | 16 * v3);
				v5 = v2 + v3;
				v6 = (v2 - v4) ^ ((v4 >> 26) | (v4 << 6));
				v7 = v5 + v4;
				v8 = (v5 - v6) ^ ((v6 >> 24) | (v6 << 8));
				v9 = v7 + v6;
				v10 = (v7 - v8) ^ ((v8 >> 16) | (v8 << 16));
				v11 = v9 + v8;
				v12 = (v9 - v10) ^ ((v10 >> 13) | (v10 << 19));
				nBeginValue2 = v11 + v10;
				nBeginValue0 = (v11 - v12) ^ ((v12 >> 28) | 16 * v12);
				nBeginValue1 = nBeginValue2 + v12;
				length -= 12;
				pdwData0 += 3;
			}
			switch (length)
			{
			case 0xCu:
				nBeginValue0 += pdwData0[2];
				nBeginValue1 += pdwData0[1];
				nBeginValue2 += *pdwData0;
				break;
			case 0xBu:
				nBeginValue0 += pdwData0[2] & 0xFFFFFF;
				nBeginValue1 += pdwData0[1];
				nBeginValue2 += *pdwData0;
				break;
			case 0xAu:
				nBeginValue0 += pdwData0[2] & 0xFFFF;
				nBeginValue1 += pdwData0[1];
				nBeginValue2 += *pdwData0;
				break;
			case 9u:
				nBeginValue0 += pdwData0[2] & 0xFF;
				nBeginValue1 += pdwData0[1];
				nBeginValue2 += *pdwData0;
				break;
			case 8u:
				nBeginValue1 += pdwData0[1];
				nBeginValue2 += *pdwData0;
				break;
			case 7u:
				nBeginValue1 += pdwData0[1] & 0xFFFFFF;
				nBeginValue2 += *pdwData0;
				break;
			case 6u:
				nBeginValue1 += pdwData0[1] & 0xFFFF;
				nBeginValue2 += *pdwData0;
				break;
			case 5u:
				nBeginValue1 += pdwData0[1] & 0xFF;
				nBeginValue2 += *pdwData0;
				break;
			case 4u:
				nBeginValue2 += *pdwData0;
				break;
			case 3u:
				nBeginValue2 += *pdwData0 & 0xFFFFFF;
				break;
			case 2u:
				nBeginValue2 += *pdwData0 & 0xFFFF;
				break;
			case 1u:
				nBeginValue2 += *pdwData0 & 0xFF;
				break;
			case 0u:
				return nBeginValue0;
			}
		}
		else if (!((uint32_t)data & 1))
		{
			while (length > 0xC)
			{
				v14 = nBeginValue1 + (pdwData0[3] << 16) + pdwData0[2];
				v15 = nBeginValue0 + (pdwData0[5] << 16) + pdwData0[4];
				v16 = (nBeginValue2 + (pdwData0[1] << 16) + *pdwData0 - v15) ^ ((v15 >> 28) | 16 * v15);
				v17 = v14 + v15;
				v18 = (v14 - v16) ^ ((v16 >> 26) | (v16 << 6));
				v19 = v17 + v16;
				v20 = (v17 - v18) ^ ((v18 >> 24) | (v18 << 8));
				v21 = v19 + v18;
				v22 = (v19 - v20) ^ ((v20 >> 16) | (v20 << 16));
				v23 = v21 + v20;
				v24 = (v21 - v22) ^ ((v22 >> 13) | (v22 << 19));
				nBeginValue2 = v23 + v22;
				nBeginValue0 = (v23 - v24) ^ ((v24 >> 28) | 16 * v24);
				nBeginValue1 = nBeginValue2 + v24;
				length -= 12;
				pdwData0 += 6;
			}
			switch (length)
			{
			case 0xCu:
				nBeginValue0 += (pdwData0[5] << 16) + pdwData0[4];
				nBeginValue1 += (pdwData0[3] << 16) + pdwData0[2];
				nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
				break;
			case 0xBu:
				nBeginValue0 += *((uint8_t *)pdwData0 + 10) << 16;
			case 0xAu:
				nBeginValue0 += pdwData0[4];
				nBeginValue1 += (pdwData0[3] << 16) + pdwData0[2];
				nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
				break;
			case 9u:
				nBeginValue0 += *((uint8_t *)pdwData0 + 8);
			case 8u:
				nBeginValue1 += (pdwData0[3] << 16) + pdwData0[2];
				nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
				break;
			case 7u:
				nBeginValue1 += *((uint8_t *)pdwData0 + 6) << 16;
			case 6u:
				nBeginValue1 += pdwData0[2];
				nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
				break;
			case 5u:
				nBeginValue1 += *((uint8_t *)pdwData0 + 4);
			case 4u:
				nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
				break;
			case 3u:
				nBeginValue2 += *((uint8_t *)pdwData0 + 2) << 16;
			case 2u:
				nBeginValue2 += *pdwData0;
				break;
			case 1u:
				nBeginValue2 += *(uint8_t *)pdwData0;
				break;
			case 0u:
				return nBeginValue0;
			}
		}
		else {
			while (length > 0xC)
			{
				v25 = nBeginValue1
					+ *((uint8_t *)pdwData0 + 4)
					+ (*((uint8_t *)pdwData0 + 5) << 8)
					+ (*((uint8_t *)pdwData0 + 6) << 16)
					+ (*((uint8_t *)pdwData0 + 7) << 24);
				v26 = nBeginValue0
					+ *((uint8_t *)pdwData0 + 8)
					+ (*((uint8_t *)pdwData0 + 9) << 8)
					+ (*((uint8_t *)pdwData0 + 10) << 16)
					+ (*((uint8_t *)pdwData0 + 11) << 24);
				v27 = (nBeginValue2
					+ *(uint8_t *)pdwData0
					+ (*((uint8_t *)pdwData0 + 1) << 8)
					+ (*((uint8_t *)pdwData0 + 2) << 16)
					+ (*((uint8_t *)pdwData0 + 3) << 24)
					- v26) ^ ((v26 >> 28) | 16 * v26);
				v28 = v25 + v26;
				v29 = (v25 - v27) ^ ((v27 >> 26) | (v27 << 6));
				v30 = v28 + v27;
				v31 = (v28 - v29) ^ ((v29 >> 24) | (v29 << 8));
				v32 = v30 + v29;
				v33 = (v30 - v31) ^ ((v31 >> 16) | (v31 << 16));
				v34 = v32 + v31;
				v35 = (v32 - v33) ^ ((v33 >> 13) | (v33 << 19));
				nBeginValue2 = v34 + v33;
				nBeginValue0 = (v34 - v35) ^ ((v35 >> 28) | 16 * v35);
				nBeginValue1 = nBeginValue2 + v35;
				length -= 12;
				pdwData0 += 3;
			}
			switch (length)
			{
			case 0u:
				result = nBeginValue0;
				break;
			case 0xCu:
				nBeginValue0 += *((uint8_t *)pdwData0 + 11) << 24;
			case 0xBu:
				nBeginValue0 += *((uint8_t *)pdwData0 + 10) << 16;
			case 0xAu:
				nBeginValue0 += *((uint8_t *)pdwData0 + 9) << 8;
			case 9u:
				nBeginValue0 += *((uint8_t *)pdwData0 + 8);
			case 8u:
				nBeginValue1 += *((uint8_t *)pdwData0 + 7) << 24;
			case 7u:
				nBeginValue1 += *((uint8_t *)pdwData0 + 6) << 16;
			case 6u:
				nBeginValue1 += *((uint8_t *)pdwData0 + 5) << 8;
			case 5u:
				nBeginValue1 += *((uint8_t *)pdwData0 + 4);
			case 4u:
				nBeginValue2 += *((uint8_t *)pdwData0 + 3) << 24;
			case 3u:
				nBeginValue2 += *((uint8_t *)pdwData0 + 2) << 16;
			case 2u:
				nBeginValue2 += *((uint8_t *)pdwData0 + 1) << 8;
			case 1u:
				nBeginValue2 += *(uint8_t *)pdwData0;
			}
		}

		v36 = (nBeginValue1 ^ nBeginValue0) - ((nBeginValue1 >> 18) | (nBeginValue1 << 14));
		v37 = (v36 ^ nBeginValue2) - ((v36 >> 21) | (v36 << 11));
		v38 = (v37 ^ nBeginValue1) - ((v37 >> 7) | (v37 << 25));
		v39 = (v38 ^ v36) - ((v38 >> 16) | (v38 << 16));
		v40 = (v39 ^ v37) - ((v39 >> 28) | 16 * v39);
		v41 = (v40 ^ v38) - ((v40 >> 18) | (v40 << 14));

		return (v41 ^ v39) - ((v41 >> 8) | (v41 << 24));
	}
*/

}
