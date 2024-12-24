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
#include "IFRSA.h"
#include "global.h"
#include "rsaref.h"
#include "math.h"
#include "stdlib.h"
#include "IFSystemAPI.h"
#include "IFMemStream.h"


extern "C" {
#include "rsa.h"
}


IFRSA::IFRSA(void)
{
}


IFRSA::~IFRSA(void)
{
}

bool IFRSA::generateKey(IFRefPtr<IFMemStream> pPublicKey, IFRefPtr<IFMemStream> pPrivateKey, int nBits)
{
	R_RANDOM_STRUCT randomstruct;

	//unsigned char data[] = "12345678";
	//if (IFNativeSystemAPI::getSingletonPtr())
	srand(IFNativeSystemAPI::getTickCount());
	R_RandomInit(&randomstruct);
	for (int  i = 0; i < nBits/8; i ++ )
	{
		int ndata = rand();
		R_RandomUpdate(&randomstruct, (unsigned char*)&ndata, 4);
	}
	R_RandomFinal(&randomstruct);

	R_RSA_PUBLIC_KEY pubk;
	R_RSA_PRIVATE_KEY prik;
	R_RSA_PROTO_KEY prok;
	prok.bits = nBits;
	prok.useFermat4 = 0;
	int status =  R_GeneratePEMKeys(&pubk, &prik, &prok, &randomstruct);
	if (status != 0)
	{
		return false;
	}
	//int nPublicSize = pubk.bits / 8 + ((pubk.bits % 8) ? 1 : 0);
	//int nPrivateSize = prik.bits / 8 + ((prik.bits % 8) ? 1 : 0);

	pPublicKey->write(&pubk, sizeof(pubk));
	pPrivateKey->write(&prik, sizeof(prik));

	return true;
}

bool IFRSA::encryptPrivate( IFRefPtr<IFStream> pOutPut, IFRefPtr<IFStream> pInput, IFRefPtr<IFMemStream> pPrivateKey )
{
	unsigned char buf[64];
	unsigned char bufout[128];
	R_RSA_PRIVATE_KEY privatekey;

	memcpy(&privatekey, pPrivateKey->getBuffer(), pPrivateKey->size());
	int nBufSize = privatekey.bits/16;
	int nOutBufSize = nBufSize*2;
	while (int nReadNum = pInput->read(buf,nBufSize))
	{
		unsigned int nEncryptSize = 0;
		int status = RSAPrivateEncrypt(bufout, &nEncryptSize, buf, nReadNum, &privatekey);

		if (status != 0)
			return false;
		if (nEncryptSize != nOutBufSize)
			return false;
		pOutPut->write(bufout, nEncryptSize);

	}
	return true;
}

bool IFRSA::decryptPrivate( IFRefPtr<IFStream> pOutPut, IFRefPtr<IFStream> pInput, IFRefPtr<IFMemStream> pPrivateKey )
{
	unsigned char buf[128];
	unsigned char bufout[256];
	R_RSA_PRIVATE_KEY privatekey;
	memcpy(&privatekey, pPrivateKey->getBuffer(), pPrivateKey->size());

	while (int nReadNum = pInput->read(buf,sizeof(buf)))
	{
		unsigned int nDecryptSize = 0;
		int status = RSAPrivateDecrypt(bufout, &nDecryptSize, buf, nReadNum, &privatekey);

		if (status != 0)
			return false;
		if (nDecryptSize > 128)
			return false;
		pOutPut->write(bufout, nDecryptSize);

	}
	return true;
}

bool IFRSA::encryptPublic( IFRefPtr<IFStream> pOutPut, IFRefPtr<IFStream> pInput, IFRefPtr<IFMemStream> pPublicKey )
{
	unsigned char buf[64];
	unsigned char bufout[128];
	R_RANDOM_STRUCT randomstruct;

	R_RandomInit(&randomstruct);
	for (int  i = 0; i < 32; i ++ )
	{
		long long ndata = rand();
		R_RandomUpdate(&randomstruct, (unsigned char*)&ndata, 8);
	}
	R_RSA_PUBLIC_KEY publickey;
	memcpy(&publickey, pPublicKey->getBuffer(), pPublicKey->size());

	while (int nReadNum = pInput->read(buf,sizeof(buf)))
	{
		unsigned int nEncryptSize = 0;
		int status = RSAPublicEncrypt(bufout, &nEncryptSize, buf, nReadNum, &publickey, &randomstruct);

		if (status != 0)
			return false;
		if (nEncryptSize < nReadNum)
			return false;
		pOutPut->write(bufout, nEncryptSize);

	}
	return true;
}

bool IFRSA::decryptPublic( IFRefPtr<IFStream> pOutPut, IFRefPtr<IFStream> pInput, IFRefPtr<IFMemStream> pPublicKey )
{
	unsigned char buf[128];
	unsigned char bufout[128];
	R_RSA_PUBLIC_KEY publickey;


	memcpy(&publickey, pPublicKey->getBuffer(), pPublicKey->size());
	int nBufSize = publickey.bits/8;
	int nOutBufSize = nBufSize;

	while (int nReadNum = pInput->read(buf,nBufSize))
	{
		unsigned int nDecryptSize = 0;
		int status = RSAPublicDecrypt(bufout, &nDecryptSize, buf, nReadNum, &publickey);

		if (status != 0)
			return false;
		if (nDecryptSize > nOutBufSize)
			return false;
		pOutPut->write(bufout, nDecryptSize);

	}
	return true;
}
