#if !defined(_PCRANDOM_H_)
#define _PCRANDOM_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "PC_Lib.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//定义静态常量属性
#define PC_STATIC_PROPERTY(T, name, ...)	static const T name = (__VA_ARGS__)

/**
*@brief		高性能随机数类(使用struct为了兼容C)，使用Mersenne Twister 19937
*/
struct PCStruMT19937
{
public:
	//初始化随机数发生器
	void srand(unsigned int sd)
	{
		unsigned int prev = m_MTState[0] = sd;
		for (int i = 1; i < STATE_SIZE; ++i)
		{
			prev = m_MTState[i] = ((i + INIT_MULTIPLIER * (prev ^ (prev >> 30))) & F_MASK);
		}
		m_Index = 0;
	}

	//产生随机数
	unsigned int rand(void)
	{
		if (m_Index >= STATE_SIZE)
		{
			printf("FATAL！m_Index >= STATE_SIZE，abort");
			abort();
			return 0;
		}
		if (m_Index == 0) generate_numbers();

		unsigned int y = m_MTState[m_Index++];
		y ^= (y >> OUTPUT_U);
		y ^= (y << OUTPUT_S) & OUTPUT_B;
		y ^= (y << OUTPUT_T) & OUTPUT_C;
		y ^= (y >> OUTPUT_L);

		if (m_Index >= STATE_SIZE) m_Index = 0;
		return y;
	}
	
private:
	PC_STATIC_PROPERTY(unsigned int, MAX, (unsigned int)~0);

    PC_STATIC_PROPERTY(int, WORD_SIZE, sizeof(unsigned int) );                  // WORD的比特定义
	PC_STATIC_PROPERTY(int, MASK_BITS, WORD_SIZE - 1);							// WORD的低位BIT MASK
	PC_STATIC_PROPERTY(unsigned int, F_MASK, (unsigned int)~0);                 // 0xffffffff
	PC_STATIC_PROPERTY(unsigned int, H_MASK, (F_MASK << MASK_BITS) & F_MASK);   // 0x80000000
	PC_STATIC_PROPERTY(unsigned int, L_MASK, ~H_MASK & F_MASK);                 // 0x7fffffff

	PC_STATIC_PROPERTY(int, STATE_SIZE, 624);									// 产生随机数的state大小
	PC_STATIC_PROPERTY(int, MID_SIZE, 397);
	PC_STATIC_PROPERTY(int, MOD_SIZE, STATE_SIZE - MID_SIZE);

	PC_STATIC_PROPERTY(unsigned int, INIT_MULTIPLIER, 0x6c078965);
	PC_STATIC_PROPERTY(unsigned int, PARAMETER_A, 0x9908b0df);
	PC_STATIC_PROPERTY(unsigned int, OUTPUT_B, 0x9d2c5680);
	PC_STATIC_PROPERTY(unsigned int, OUTPUT_C, 0xefc60000);
	PC_STATIC_PROPERTY(unsigned int, OUTPUT_S, 7);
	PC_STATIC_PROPERTY(unsigned int, OUTPUT_T, 15);
	PC_STATIC_PROPERTY(unsigned int, OUTPUT_U, 11);
	PC_STATIC_PROPERTY(unsigned int, OUTPUT_L, 18);

	unsigned int	m_MTState[STATE_SIZE]; 
	int				m_Index;

	//产生m_MTState[STATE_SIZE]
	void generate_numbers(void)
	{
		/* 填充低位 */
		int i = 0;
		for (; i < STATE_SIZE - MID_SIZE; ++i)
		{
			unsigned int y = (m_MTState[i] & H_MASK) |  (m_MTState[i + 1] & L_MASK);   
			m_MTState[i] = m_MTState[i + MID_SIZE] ^ (y >> 1) ^ (y & 1 /* y是奇数 */ ? PARAMETER_A : 0);
		}
		/* 填充高位 */
		int n = 0;
		for (; i < STATE_SIZE - 1; ++i, ++n)
		{
			unsigned int y = (m_MTState[i] & H_MASK) |  (m_MTState[i + 1] & L_MASK);   
			m_MTState[i] = m_MTState[n] ^ (y >> 1) ^ (y & 1 /* y是奇数 */ ? PARAMETER_A : 0);
		}
		/* 最后一次round, 此时i == 623, n == 397 - 1 */
		{
			unsigned int y = (m_MTState[i] & H_MASK) |  (m_MTState[0] & L_MASK);       
			m_MTState[i] = m_MTState[n] ^ (y >> 1) ^ (y & 1 /* y是奇数 */ ? PARAMETER_A : 0);
		}
	}
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCRANDOM_H_)
