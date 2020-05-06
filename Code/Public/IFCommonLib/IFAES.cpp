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
#include "IFAES.h"
#include "crypt_aes.h"
#include "IFMemStream.h"
#include "openssl/des.h"

IF_DEFINERTTI(IFAES, IFRefObj)

IFAES::IFAES( const IFString& sUserKey )
{
	unsigned char keybuf[32];
	int nKeyLen = 32;
	if (sUserKey.size() < 16)
		nKeyLen = 16;
	else if (sUserKey.size() < 24)
		nKeyLen = 24;

	for (int i = 0; i < nKeyLen; i ++)
	{
		keybuf[i]=sUserKey[i%sUserKey.size()];
	}
	m_pEnKey = IFAlloc::Alloc(sizeof(aes_ks_t));
	m_pDeKey = IFAlloc::Alloc(sizeof(aes_ks_t));
	aes_setks_encrypt(keybuf,8*nKeyLen,(aes_ks_t*)m_pEnKey);
	aes_setks_decrypt(keybuf,8*nKeyLen,(aes_ks_t*)m_pDeKey);

}

IFAES::~IFAES()
{
	IFAlloc::Dealloc(m_pEnKey);
	IFAlloc::Dealloc(m_pDeKey);
}

void IFAES::encrypt( IFStream* pInStream, IFStream* pOutStream ) const
{
	unsigned char sIn[AES_BLOCK_SIZE];
	unsigned char sOut[AES_BLOCK_SIZE];
	pOutStream->writeUI32((IFUI32)pInStream->size());
	while (int nReadSize = pInStream->read(sIn,AES_BLOCK_SIZE))
	{
		aes_ecb_encrypt(sIn, sOut, (aes_ks_t*)m_pEnKey);
		pOutStream->write(sOut,AES_BLOCK_SIZE);
	}
}

void IFAES::decrypt( IFStream* pInStream, IFStream* pOutStream ) const
{
	unsigned char sIn[AES_BLOCK_SIZE];
	unsigned char sOut[AES_BLOCK_SIZE];
	int nSrcLen = pInStream->readUI32();
	
	while (int nReadSize = pInStream->read(sIn,AES_BLOCK_SIZE))
	{
		aes_ecb_decrypt(sIn, sOut, (aes_ks_t*)m_pDeKey);
		int nWriteSize = IFMin(AES_BLOCK_SIZE, nSrcLen);
		nSrcLen-=nWriteSize;
		pOutStream->write(sOut,nWriteSize);
	}
}

IF_DEFINERTTI(IFDES, IFRefObj)

IFDES::IFDES(const IFString& sUserKey)
{
	m_des.resize(sizeof(des_key_schedule));
	des_key_schedule* pdes = (des_key_schedule*)(char*)m_des;
	des_cblock key;
	
	unsigned char key_56[8];
	ZeroMemory(key_56, sizeof(key_56));
	for (int i = 0; i < 8 && i < (int)sUserKey.length(); i++)
	{
		key_56[i] = sUserKey[i];
	}
	//char key[8];

	key[0] = key_56[0];
	key[1] = ((key_56[0] << 7) & 0xFF) | (key_56[1] >> 1);
	key[2] = ((key_56[1] << 6) & 0xFF) | (key_56[2] >> 2);
	key[3] = ((key_56[2] << 5) & 0xFF) | (key_56[3] >> 3);
	key[4] = ((key_56[3] << 4) & 0xFF) | (key_56[4] >> 4);
	key[5] = ((key_56[4] << 3) & 0xFF) | (key_56[5] >> 5);
	key[6] = ((key_56[5] << 2) & 0xFF) | (key_56[6] >> 6);
	key[7] = (key_56[6] << 1) & 0xFF;

	des_set_odd_parity(&key);
	des_set_key(&key, *pdes);
}

IFDES::~IFDES()
{

}

void IFDES::encrypt(IFStream* pInStream, IFStream* pOutStream) const
{
	des_key_schedule& ks = *(des_key_schedule*)(char*)m_des;
	des_cblock block;
	des_cblock blockout;
	pOutStream->writeUI32((IFUI32)pInStream->size());

	while (int nReadSize = pInStream->read(block, AES_BLOCK_SIZE))
	{
		if (nReadSize != sizeof(block))
		{
			for (int i = nReadSize; i < sizeof(block); i++)
			{
				block[i] = 0;
			}
		}
		des_ecb_encrypt(&block, &blockout, ks, DES_ENCRYPT);
		pOutStream->write(blockout, AES_BLOCK_SIZE);
	}

}

void IFDES::encrypt(char* in_block, char* out_block) const
{
	des_key_schedule& ks = *(des_key_schedule*)(char*)m_des;
	des_ecb_encrypt((des_cblock*)in_block, (des_cblock*)out_block, ks, DES_ENCRYPT);

}
void IFDES::decrypt(char* in_block, char* out_block) const
{
	des_key_schedule& ks = *(des_key_schedule*)(char*)m_des;
	des_ecb_encrypt((des_cblock*)in_block, (des_cblock*)out_block, ks, DES_DECRYPT);

}
void IFDES::decrypt(IFStream* pInStream, IFStream* pOutStream) const
{
	des_key_schedule& ks = *(des_key_schedule*)(char*)m_des;
	des_cblock block;
	des_cblock blockout;
	int nSrcLen = pInStream->readUI32();

	while (int nReadSize = pInStream->read(block, AES_BLOCK_SIZE))
	{

		des_ecb_encrypt(&block, &blockout, ks, DES_DECRYPT);
		int nWriteSize = IFMin((int)sizeof(block), nSrcLen);
		nSrcLen -= nWriteSize;

		pOutStream->write(blockout, nWriteSize);
	}
}
