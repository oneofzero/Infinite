/*      aes.c         */
/*标准 256位AES (Rijndael算法) 例子
Cipher: AES (Rijndael)
Mode: CBC
Key: 256 bits (64 bytes)
IV: 16 bytes
Max File Size: 2GB              // 因为fsize是int，最大 2GB
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
// 为优化I/O操作的最佳缓冲区大小 64K
#define BLOCKSIZE  (64 * 1024) // the optimal buffer size for sequential I/O on Windows NT/2k/XP

void gen_iv(unsigned char *buf, int size)
{
    while(--size >= 0) buf[size] += rand();       
}
/*=====================================================================*/
//         密钥文件                源文件   加解密后的文件
int crypt(char *key,int encrypt, char *src, char *dst)  // 参数2代表加密(T)或解密(F)
{
    keyInstance keyInst;
    cipherInstance cipherInst;
    FILE *fkey, *fsrc, *fdst;
    u8 keyMaterial[MAX_KEY_SIZE], membuf[BLOCKSIZE + 32], initiv[MAX_IV_SIZE];   // 64 64k 16
	//  key                         src            
    int i, fsize, sread = 0, ErrorCode =0;
    
    if((fkey = fopen(key, "rb")) == NULL)return ErrorCode = -1;          // 密钥文件不可读
    if((fsrc = fopen(src, "rb")) == NULL)return ErrorCode = -2;          // 源文件不可读  
    if((fdst = fopen(dst, "wb")) == NULL)
    {
        ErrorCode = -3;                                                  // 无法创建目标文件
        goto quit;
    }
    // 获得 key (64位)  
    if(MAX_KEY_SIZE != fread(keyMaterial, 1, MAX_KEY_SIZE, fkey)) 
    {
        ErrorCode = -4;                                                  // 获得密钥失败(文件长度不够)                            
        goto quit;
    }

    // 给cipherInst.IV 赋值
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
        fseek(fsrc, - MAX_IV_SIZE, SEEK_END);                    // 读文件末尾的MAX_IV_SIZE个字节到 cipherInst.IV
        if(MAX_IV_SIZE != fread(cipherInst.IV, 1, MAX_IV_SIZE, fsrc))
        {
            ErrorCode = -5;                     // 读密文文件错误 ，可能是文件长度不够
            goto quit;
        }
    }
    fsize = ftell(fsrc);                        // 获得文件大小
    rewind(fsrc);                               // 重新指向开头
    if(!encrypt && (fsize < 32 || fsize % 16))  // 如果解密时 fsize<32 或者 不是16的倍数错误
    {
        ErrorCode = -6;                         // 解密时，密文文件长度错误
        goto quit;
    }

    // makeKey错误
    if(makeKey(&keyInst,encrypt?0:1, MAX_KEY_SIZE * 4, keyMaterial)< 1)
    {
        ErrorCode = -7;                      // makeKey错误
        goto quit;
    }
    cipherInst.mode = MODE_CBC;
    
    while((sread = fread(membuf, 1, BLOCKSIZE, fsrc)) > 0) // 一次读64K
    {
        fsize -= sread;
        if(encrypt) 
        {   
			// 如果读到的数据是64k,调用block(整块加密)，同时生成新的cipherInst.IV 
            if(fsize)
            {
                blockEncrypt(&cipherInst, &keyInst, membuf, sread, membuf);
                memcpy(cipherInst.IV, membuf + BLOCKSIZE - MAX_IV_SIZE, MAX_IV_SIZE);
            }      //如果读到的不足64k或刚好数据没有了,调用pad
            else 
				sread = padEncrypt(&cipherInst, &keyInst, membuf, sread, membuf);
        }
        else
        {  
			// 解密 
			// 如果读到 64k,并且剩余的>32字节，调用 block 解密
            if((sread == BLOCKSIZE) && (fsize > 32))
                blockDecrypt(&cipherInst, &keyInst, membuf, sread, membuf);
            else       // 最后一次解密
            {
                if(fsize)   // 如果读到了 64k,还有剩余，但小于32字节,把剩下的一起读了，2次算一次
                {
                    if(fsize != fread(membuf + BLOCKSIZE, 1, fsize, fsrc))
                    {
                        ErrorCode = -8;      // 解密时，读取密文文件发生错误
                        goto quit;
                    }
                    sread += fsize;
                }
				// 最后一次 pad
                if((sread = padDecrypt(&cipherInst, &keyInst, membuf, sread - MAX_IV_SIZE, membuf))<=0)
                {
                    ErrorCode = -9;    // 解密时，读取密文文件发生错误
                    goto quit;
                }
            }
        }
		// 将加解密数据写入文件
        if(sread != fwrite(membuf, 1, sread, fdst))
        {
            ErrorCode = -10;              // 写入目标文件错误，可能磁盘空间不足
            goto quit;
        }
    }


    if(encrypt)    // 如果加密的话，将 initiv 写入文件末尾
    {
        if(MAX_IV_SIZE != fwrite(initiv, 1, MAX_IV_SIZE, fdst))
        {
            ErrorCode = -10;     // 写入目标文件错误，可能磁盘空间不足
            goto quit;
        }
    }

quit:
    fclose(fkey);
    if(fsrc) fclose(fsrc);
    if(fdst)
    {
        fclose(fdst);
        if(ErrorCode && remove(dst)) ErrorCode-= 100;       // 删除dst失败;
    }
	// 清除数据
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
	// 给cipherInst.IV 赋值
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

