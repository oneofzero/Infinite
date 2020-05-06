/*      aes.c         */
/*��׼ 256λAES (Rijndael�㷨) ����
Cipher: AES (Rijndael)
Mode: CBC
Key: 256 bits (64 bytes)
IV: 16 bytes
Max File Size: 2GB              // ��Ϊfsize��int����� 2GB
Cipher File Structure:
[cipherdata] + [padding <= 16 bytes] + [IV = 16 random bytes]


Website
-------
http://www.ozemail.com.au/~nulifetv/freezip/freeware/
http://freezip.cjb.net/freeware/
freezip(at)bigfoot,com
=======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "rijndael-api-fst.h"
// Ϊ�Ż�I/O��������ѻ�������С 64K
#define BLOCKSIZE  (64 * 1024) // the optimal buffer size for sequential I/O on Windows NT/2k/XP

void gen_iv(unsigned char *buf, int size)
{
    while(--size >= 0) buf[size] += rand();       
}
/*=====================================================================*/
//         ��Կ�ļ�                Դ�ļ�   �ӽ��ܺ���ļ�
int crypt(char *key,int encrypt, char *src, char *dst)  // ����2�������(T)�����(F)
{
    keyInstance keyInst;
    cipherInstance cipherInst;
    FILE *fkey, *fsrc, *fdst;
    u8 keyMaterial[MAX_KEY_SIZE], membuf[BLOCKSIZE + 32], initiv[MAX_IV_SIZE];   // 64 64k 16
	//  key                         src            
    int i, fsize, sread = 0, ErrorCode =0;
    
    if((fkey = fopen(key, "rb")) == NULL)return ErrorCode = -1;          // ��Կ�ļ����ɶ�
    if((fsrc = fopen(src, "rb")) == NULL)return ErrorCode = -2;          // Դ�ļ����ɶ�  
    if((fdst = fopen(dst, "wb")) == NULL)
    {
        ErrorCode = -3;                                                  // �޷�����Ŀ���ļ�
        goto quit;
    }
    // ��� key (64λ)  
    if(MAX_KEY_SIZE != fread(keyMaterial, 1, MAX_KEY_SIZE, fkey)) 
    {
        ErrorCode = -4;                                                  // �����Կʧ��(�ļ����Ȳ���)                            
        goto quit;
    }

    // ��cipherInst.IV ��ֵ
    if(encrypt)
    {
        for(i = (BLOCKSIZE / 4) - 1; i >= 0; i--) sread += ((int*)membuf)[i];
        srand(sread ^ time(NULL));
        gen_iv(cipherInst.IV, MAX_IV_SIZE);
        memcpy(initiv, cipherInst.IV, MAX_IV_SIZE);
        fseek(fsrc, 0, SEEK_END);                               
    }
    else
    {
        fseek(fsrc, - MAX_IV_SIZE, SEEK_END);                    // ���ļ�ĩβ��MAX_IV_SIZE���ֽڵ� cipherInst.IV
        if(MAX_IV_SIZE != fread(cipherInst.IV, 1, MAX_IV_SIZE, fsrc))
        {
            ErrorCode = -5;                     // �������ļ����� ���������ļ����Ȳ���
            goto quit;
        }
    }
    fsize = ftell(fsrc);                        // ����ļ���С
    rewind(fsrc);                               // ����ָ��ͷ
    if(!encrypt && (fsize < 32 || fsize % 16))  // �������ʱ fsize<32 ���� ����16�ı�������
    {
        ErrorCode = -6;                         // ����ʱ�������ļ����ȴ���
        goto quit;
    }

    // makeKey����
    if(makeKey(&keyInst,encrypt?0:1, MAX_KEY_SIZE * 4, keyMaterial)< 1)
    {
        ErrorCode = -7;                      // makeKey����
        goto quit;
    }
    cipherInst.mode = MODE_CBC;
    
    while((sread = fread(membuf, 1, BLOCKSIZE, fsrc)) > 0) // һ�ζ�64K
    {
        fsize -= sread;
        if(encrypt) 
        {   
			// ���������������64k,����block(�������)��ͬʱ�����µ�cipherInst.IV 
            if(fsize)
            {
                blockEncrypt(&cipherInst, &keyInst, membuf, sread, membuf);
                memcpy(cipherInst.IV, membuf + BLOCKSIZE - MAX_IV_SIZE, MAX_IV_SIZE);
            }      //��������Ĳ���64k��պ�����û����,����pad
            else 
				sread = padEncrypt(&cipherInst, &keyInst, membuf, sread, membuf);
        }
        else
        {  
			// ���� 
			// ������� 64k,����ʣ���>32�ֽڣ����� block ����
            if((sread == BLOCKSIZE) && (fsize > 32))
                blockDecrypt(&cipherInst, &keyInst, membuf, sread, membuf);
            else       // ���һ�ν���
            {
                if(fsize)   // ��������� 64k,����ʣ�࣬��С��32�ֽ�,��ʣ�µ�һ����ˣ�2����һ��
                {
                    if(fsize != fread(membuf + BLOCKSIZE, 1, fsize, fsrc))
                    {
                        ErrorCode = -8;      // ����ʱ����ȡ�����ļ���������
                        goto quit;
                    }
                    sread += fsize;
                }
				// ���һ�� pad
                if((sread = padDecrypt(&cipherInst, &keyInst, membuf, sread - MAX_IV_SIZE, membuf))<=0)
                {
                    ErrorCode = -9;    // ����ʱ����ȡ�����ļ���������
                    goto quit;
                }
            }
        }
		// ���ӽ�������д���ļ�
        if(sread != fwrite(membuf, 1, sread, fdst))
        {
            ErrorCode = -10;              // д��Ŀ���ļ����󣬿��ܴ��̿ռ䲻��
            goto quit;
        }
    }


    if(encrypt)    // ������ܵĻ����� initiv д���ļ�ĩβ
    {
        if(MAX_IV_SIZE != fwrite(initiv, 1, MAX_IV_SIZE, fdst))
        {
            ErrorCode = -10;     // д��Ŀ���ļ����󣬿��ܴ��̿ռ䲻��
            goto quit;
        }
    }

quit:
    fclose(fkey);
    if(fsrc) fclose(fsrc);
    if(fdst)
    {
        fclose(fdst);
        if(ErrorCode && remove(dst)) ErrorCode-= 100;       // ɾ��dstʧ��;
    }
	// �������
    if(!encrypt) memset(membuf, 0, sizeof(membuf)); 
    memset(keyMaterial, 0, sizeof(keyMaterial));
    makeKey(&keyInst, DIR_ENCRYPT, MAX_KEY_SIZE * 4, keyMaterial);
    return ErrorCode;
}
int  smallcrypt(void* key,int encrypt,void* src,void* dst,int size)
{
	keyInstance keyInst;
    cipherInstance cipherInst;
	int i,sread = 0 ;
	u8 *temp = (u8*)src + size - MAX_IV_SIZE;
	u8 keyMaterial[MAX_KEY_SIZE], membuf[BLOCKSIZE + 32], initiv[MAX_IV_SIZE];   // 64 64k 16
	if(size > BLOCKSIZE || size < 0 ) return 0;
	if(!encrypt && ((size < 32) || (size % 16))) return -1;
	
	memcpy(keyMaterial,key,MAX_KEY_SIZE);
	// ��cipherInst.IV ��ֵ
    if(encrypt)
    {
        for(i = (BLOCKSIZE / 4) - 1; i >= 0; i--) sread += ((int*)membuf)[i];
        srand(sread ^ time(NULL));
        gen_iv(cipherInst.IV, MAX_IV_SIZE);
        memcpy(initiv, cipherInst.IV, MAX_IV_SIZE);                          
    }
    else  memcpy(initiv,temp,MAX_IV_SIZE);
    if(makeKey(&keyInst,encrypt?0:1, MAX_KEY_SIZE * 4, keyMaterial)< 1) return -2;
    cipherInst.mode = MODE_CBC;
	temp = (u8*)src;
	if(encrypt)
	{
		sread = padEncrypt(&cipherInst, &keyInst,temp, size, membuf);
		if(sread <= 0) return -1;
		memcpy(dst,membuf,sread);
		memcpy(dst,initiv,MAX_IV_SIZE);
		return sread + MAX_IV_SIZE;
	}
	sread = padDecrypt(&cipherInst, &keyInst,temp,size - MAX_IV_SIZE,membuf);
	if(sread <= 0) return -2;
	memcpy(dst,membuf,sread);
	return sread;
}

