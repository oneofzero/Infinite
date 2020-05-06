/*
The MIT License (MIT)
Copyright © 2014 Huang Cong

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the “Software”), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
*/
#include "stdafx.h"
#include "IFRandom.h"


IFRandom::IFRandom(IFUI32 nSeed)
	:m_nIndex(0)
{
	m_Buffer.resize(624);
	setSeed(nSeed);
}


IFRandom::~IFRandom(void)
{
}

IFUI32 IFRandom::rand(IFUI32 nMax)
{
	return rand()%nMax;
}

IFUI32 IFRandom::rand()
{
	if (m_nIndex == 0)
	{
		for (int i = 0; i < m_Buffer.size(); i ++)
		{
			int y = (m_Buffer[i]&0x80000000)+ (m_Buffer[(i+1)%m_Buffer.size()]&0x7fffffff);
			m_Buffer[i] = m_Buffer[(i+397)%m_Buffer.size()] ^ (y>>1);
			if (y%2)
			{
				m_Buffer[i] ^= 2567483615;
			}
		}
	}

	int y = m_Buffer[m_nIndex];
	y ^= (y >> 11);
	y ^= (y <<  7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);
	m_nIndex = (m_nIndex+1)%m_Buffer.size();
	return y;
}

IFUI32 IFRandom::rand( IFUI32 nMin, IFUI32 nMax )
{
	return rand(nMax-nMin)+nMin;
}


void IFRandom::setSeed( IFUI32 nSeed )
{
	m_Buffer[0] = nSeed;
	for (int i=1; i < m_Buffer.size(); i ++)
	{
		m_Buffer[i] = ( 1812433253UL * ( m_Buffer[i-1] ^ (m_Buffer[i-1] >> 30) ) + i ) & 0xffffffffUL;
	}

}

////创建一个长度为624的数组来存储发生器的状态
//int[0..623] MT
//	int index = 0
//
//	//用一个种子初始化发生器
//	function initialize_generator(int seed) {
//i := 0
//	   MT[0] := seed
//	   for i from 1 to 623 { // 遍历剩下的每个元素
//		   MT[i] := last 32 bits of(1812433253 * (MT[i-1] xor (right shift by 30 bits(MT[i-1]))) + i) // 0x6c078965
//	   }
//}
//
//// Extract a tempered pseudorandom number based on the index-th value,
//// calling generate_numbers() every 624 numbers
//function extract_number() {
//	if index == 0 {
//		generate_numbers()
//	}
//
//	int y := MT[index]
//y := y xor (right shift by 11 bits(y))
//y := y xor (left shift by 7 bits(y) and (2636928640)) // 0x9d2c5680
//y := y xor (left shift by 15 bits(y) and (4022730752)) // 0xefc60000
//y := y xor (right shift by 18 bits(y))
//
//index := (index + 1) mod 624
//	   return y
//}
//
//// Generate an array of 624 untempered numbers
//function generate_numbers() {
//	for i from 0 to 623 {
//		int y := (MT[i] & 0x80000000)                       // bit 31 (32nd bit) of MT[i]
//			+ (MT[(i+1) mod 624] & 0x7fffffff)   // bits 0-30 (first 31 bits) of MT[...]
//			MT[i] := MT[(i + 397) mod 624] xor (right shift by 1 bit(y))
//			if (y mod 2) != 0 { // y is odd
//				MT[i] := MT[i] xor (2567483615) // 0x9908b0df
//			}
//	}
//}