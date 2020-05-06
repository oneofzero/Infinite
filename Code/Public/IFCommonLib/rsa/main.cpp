#include "global.h"
#include "rsaref.h"
#include "math.h"
#include "stdlib.h"

extern "C" {

#include "rsa.h"
}

int main(int argc, char **argv)
{
	R_RANDOM_STRUCT randomstruct;

	unsigned char data[] = "12345678";
	srand();
	for (int  i = 0; i < 32; i ++ )
	{
		R_RandomUpdate(&randomstruct, data, 8);
	}
//	R_RandomFinal(&randomstruct);

	R_RSA_PUBLIC_KEY pubk;
	R_RSA_PRIVATE_KEY prik;
	R_RSA_PROTO_KEY prok;
	prok.bits = 512;
	prok.useFermat4 = 0;
	int status =  R_GeneratePEMKeys(&pubk, &prik, &prok, &randomstruct);
	
	unsigned char src[]="hello ras !!!";
	unsigned char encrypts[512];
	unsigned int nLen = 0;

	R_RandomInit(&randomstruct);
	for (int  i = 0; i < 32; i ++ )
	{
		long long ndata = rand();
		R_RandomUpdate(&randomstruct, (unsigned char*)&ndata, 8);
	}

	status = RSAPublicEncrypt(encrypts, &nLen, src, 14, &pubk, &randomstruct );
	unsigned char unencrpty[512];
	unsigned int nLen2;
	status = RSAPrivateDecrypt(unencrpty, &nLen2, encrypts, nLen, &prik);
	return 0;
}