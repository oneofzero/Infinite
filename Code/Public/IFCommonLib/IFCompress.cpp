#include "stdafx.h"
#include "IFCompress.h"
#include "zlib.h"
#include "IFArray.h"
#include "IFStream.h"
#include "IFMemStream.h"
#include "unzip.h"
#include "zip.h"
#include "IFUtility.h"

int  uncompress2 (
	Bytef *dest,
	uLongf *destLen,
	const Bytef *source,
	uLong sourceLen);

void IFCompress::compress( IFStream* pSrc, IFStream* pDest )
{
	//IFSimpleArray<Bytef> buf(128*1024);
	const int bufsize = 64*1024;
	Bytef srcbuf[bufsize];
	Bytef destbuf[bufsize];
	IFI64 nSrcSize = pSrc->size()-pSrc->tell();
	while (nSrcSize)
	{
		int nRead = IFMin((int)nSrcSize, bufsize);
		pSrc->read(srcbuf,nRead);
		nSrcSize-=nRead;
		uLongf destlen = bufsize;
		::compress(destbuf, &destlen, srcbuf, nRead );
		pDest->writeUI32(destlen);
		pDest->write(destbuf,destlen);
	}
}

bool IFCompress::decompress( IFStream* pSrc, IFStream* pDest )
{
	const int bufsize = 64*1024;
	Bytef srcbuf[bufsize];
	Bytef destbuf[bufsize];
	try
	{
		IFI64 nSrcSize = pSrc->size()-pSrc->tell();
		while (nSrcSize)
		{
			uLongf srclen = pSrc->readUI32();
			if (srclen>bufsize)
				return false;
			pSrc->read(srcbuf, srclen);
			nSrcSize-=srclen+4;
			uLongf destlen = bufsize;
			if (::uncompress(destbuf, &destlen, srcbuf, srclen)!=Z_OK)
				return false;
			pDest->write(destbuf,destlen);
		}
	}
	catch(IFStreamReadException& e)
	{
		return false;
	}

	return true;
}

class IFUnZipStream : public IFStream
{
public:

};

class IFMinUnZip: public IFUnZip
{
public:

	IFMinUnZip()
		:m_unZipFile(NULL)
		,m_bHaveDirectory(false)
	{
		m_Filefun.opaque = this;
		m_Filefun.zopen64_file = ifs_open;
		m_Filefun.zread_file = ifs_read;
		m_Filefun.ztell64_file = ifs_tell;
		m_Filefun.zseek64_file = ifs_seek;
		m_Filefun.zclose_file = ifs_close;

	}

	IFI64 m_startoffset = 0;

	~IFMinUnZip()
	{
		close();
	}
	static voidpf ifs_open(voidpf opaque, const void* filename, int mode)
	{
		IFMinUnZip* spThis = (IFMinUnZip*)opaque;
		return spThis->m_spStream;
	}

	static uLong ifs_read(voidpf opaque, voidpf stream, void* buf, uLong size)
	{
		IFMinUnZip* spThis = (IFMinUnZip*)opaque;
		return spThis->m_spStream->read(buf,size);
	}

	static ZPOS64_T ifs_tell(voidpf opaque, voidpf stream)
	{
		IFMinUnZip* spThis = (IFMinUnZip*)opaque;
		return spThis->m_spStream->tell() - spThis->m_startoffset;
	}	

	static long ifs_seek(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
	{
		IFMinUnZip* spThis = (IFMinUnZip*)opaque;
		if (origin == SEEK_SET)
			offset += spThis->m_startoffset;
		//auto oldp = spThis->m_spStream->tell();
		spThis->m_spStream->seek(offset, origin);
		//auto curp = spThis->m_spStream->tell();
		return 0;
	}

	static int ifs_close(voidpf opaque, voidpf stream)
	{
		return 0;
	}

	virtual bool open(IFRefPtr<IFStream> spStream)
	{
		m_startoffset = spStream->tell();

		m_spStream = spStream;
		m_unZipFile = unzOpen2_64("",&m_Filefun);

		unz_global_info64 gi;

		if (unzGetGlobalInfo64(m_unZipFile, &gi) != UNZ_OK)
		{
			m_bHaveDirectory = false;
			char buf[1024];
			//m_spZipStream = pStream;
			spStream->seek(m_startoffset, IFStream::ISSF_BEGIN);
			while (spStream->tell() < spStream->size())
			{
				IFUI32 header = spStream->readUI32();
				if (header == 0x04034b50)
				{
					ZipFileData* zdata = (ZipFileData*)(buf);

					if (spStream->read(buf+4,sizeof(ZipFileData)-4)!=sizeof(ZipFileData)-4)
						break;
					if (zdata->f_len <= 0)
						break;
					IFString fileName(IFString::EC_UTF8);

					fileName.resize(zdata->f_len);
					spStream->read((void*)fileName.c_str(), zdata->f_len);

					//pStream->seek(zdata.f_len, IFStream::ISSF_CUR);
					if(zdata->e_len > 0)
						spStream->seek(zdata->e_len,IFStream::ISSF_CUR);

					auto& zipinfo = m_FileList[fileName];//.first.pos_in_zip_directory = spStream->tell();
					zipinfo.first.pos_in_zip_directory = spStream->tell() - m_startoffset;
					zipinfo.second = *zdata;

					if(zdata->d_len > 0 && zdata->d_len!=0xffffffff)
						spStream->seek(zdata->d_len,IFStream::ISSF_CUR);

				}
				else if (header == 0x02014b50)
				{
					ZipDirData* zdata = (ZipDirData*)(buf);

					if (spStream->read(buf+4,sizeof(ZipDirData)-4)!=sizeof(ZipDirData)-4)
						break;
					if (zdata->f_len <= 0)
						break;
					IFString fileName;

					fileName.resize(zdata->f_len);
					spStream->read((void*)fileName.c_str(), zdata->f_len);

					//pStream->seek(zdata.f_len, IFStream::ISSF_CUR);
					if(zdata->e_len > 0)
						spStream->seek(zdata->e_len,IFStream::ISSF_CUR);

					//if(zdata->d_len > 0)
					//	pStream->seek(zdata->d_len,IFStream::ISSF_CUR);

					if(zdata->fc_len > 0)
						spStream->seek(zdata->fc_len,IFStream::ISSF_CUR);
					//m_Files[fileName] = *zdata;
				}
				else if (header == 0x06054b50)
					break;
				else if (header == 0x08074b50)
				{
					ZipFileDataDesc da;
					spStream->read(&da,sizeof(da)-4);
				}
				else
					break;


			}
			return true;
		}
		else
		{
			unzGoToFirstFile(m_unZipFile);
			unz_file_info64 FileInfo;
			char file[1024] = {0};
			char ext[1024] = {0};

			char com[1024] = {0};
			for (int i = 0; i < gi.number_entry; ++i)
			{

				if (unzGetCurrentFileInfo64(m_unZipFile, &FileInfo,file,sizeof(file),ext,256,com,1024)!= UNZ_OK)
					return false;
				unzGetFilePos64 (m_unZipFile, &m_FileList[file].first);

				unzGoToNextFile(m_unZipFile);
			}
			m_bHaveDirectory = true;


		}
		return true;
	}

	virtual void close()
	{
		if (m_unZipFile)
		{
			unzClose(m_unZipFile);
		}
		m_spStream = NULL;
		m_FileList.clear();
	}

	virtual int getFileCount()
	{
		return m_FileList.size();
	}

	virtual void enumFileNames(IFRefPtr<IFFunctor<void(const IFString& file)>> spFun)
	{
		for (auto& fn:m_FileList)
		{
			(*spFun)(fn.first);
		}
	}

	virtual bool openFile(const IFString& sFileName, IFStream* pStream, const IFString& sPassword)
	{
		auto sname = UStandardUnixPath(sFileName);
		FileList::iterator it = m_FileList.find(sname);
		if (it==m_FileList.end())
			return false;
		if (m_bHaveDirectory)
		{
			unzGoToFilePos64(m_unZipFile, &it->second.first );

			int result = 0;
			if (sPassword.length() == 0)
				result=unzOpenCurrentFile(m_unZipFile);/* ÎÞÃÜÂë */
			else
				result=unzOpenCurrentFilePassword(m_unZipFile, sPassword.c_str()); /* ÓÐÃÜÂë */
			if (result != 0)
				return false;
			char buf[32*1024];
			while (1)
			{
				int nLen = unzReadCurrentFile(m_unZipFile, buf, sizeof(buf));
				if (nLen>0)
					pStream->write(buf, nLen);
				else
					break;
			}

			unzCloseCurrentFile(m_unZipFile);

			pStream->seek(0,IFStream::ISSF_BEGIN);
		}
		else
		{
			m_spStream->seek(it->second.first.pos_in_zip_directory, IFStream::ISSF_BEGIN);
			ZipFileData& zdata = it->second.second;
			if (zdata.len == zdata.d_len)
			{
				IFRefPtr<IFMemStream> spStream = IFNew IFMemStream((void*)NULL,zdata.d_len);
				m_spStream->read((void*)spStream->getBuffer(), zdata.len);
				spStream->seek(0, IFStream::ISSF_BEGIN);
				return true;
			}
			else
			{
				short s=0x9c78;
				//s = 0x8b1f;
				IFSimpleArray<Bytef> buf(zdata.d_len+2);
				memcpy(buf, &s, 2);
				IFRefPtr<IFMemStream> spStream = IFNew IFMemStream((void*)NULL,zdata.len,zdata.len);
				//m_spStream->read(buf+2,10);
				m_spStream->read(buf+2,zdata.d_len);
				//zdata.crc = 0x400c4404;
				//memcpy(buf+zdata.d_len+2, &zdata.crc, 8);

				uLongf destlen = zdata.len;
				int isok = uncompress((Bytef*)spStream->getBuffer(),&destlen, buf, buf.size() );
				if (isok==Z_OK)
				{
					spStream->seek(0, IFStream::ISSF_BEGIN);
					return true;
				}
				return false;

			}
		}


		return true;
	}
	zlib_filefunc64_def m_Filefun;
	unzFile m_unZipFile;
	IFRefPtr<IFStream> m_spStream;

	typedef IFMap<IFString, IFPair<unz64_file_pos,ZipFileData>> FileList;
	FileList m_FileList;
	bool m_bHaveDirectory;
};



int  uncompress2 (
	Bytef *dest,
uLongf *destLen,
const Bytef *source,
uLong sourceLen)
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit2(&stream,32);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
			return Z_DATA_ERROR;
		return err;
	}
	*destLen = stream.total_out;

	err = inflateEnd(&stream);
	return err;
}
//
//IFUnZip::IFUnZip()
//{
//
//}
//
//
//IFUnZip::~IFUnZip()
//{
//
//}


//void IFUnZip::open( IFRefPtr<IFStream> pStream )
//{
//
//}
//
//void IFUnZip::enumFileNames( IFRefPtr<IFFunctor<void(const IFString& file)>> spFun )
//{
//	for (auto& f:m_Files)
//	{
//		(*spFun)(f.first);
//	}
//}
//
//IFRefPtr<IFStream> IFUnZip::openFile( const IFString& sFileName )
//{
//	ZipFileList::iterator it = m_Files.find(sFileName);
//	if (it==m_Files.end())
//		return NULL;
//	ZipFileData& zdata = it->second.first;
//	m_spZipStream->seek(it->second.second, IFStream::ISSF_BEGIN);
//	if (zdata.len == zdata.d_len)
//	{
//		IFRefPtr<IFMemStream> spStream = IFNew IFMemStream((void*)NULL,zdata.d_len);
//		m_spZipStream->read((void*)spStream->getBuffer(), zdata.len);
//		spStream->seek(0, IFStream::ISSF_BEGIN);
//		return spStream;
//	}
//	else
//	{
//		short s=0x9c78;
//		IFSimpleArray<Bytef> buf(zdata.d_len+2);
//		memcpy(buf, &s, 2);
//		IFRefPtr<IFMemStream> spStream = IFNew IFMemStream((void*)NULL,zdata.len);
//		m_spZipStream->read(buf+2,zdata.d_len);
//		uLongf destlen = zdata.len;
//		int isok = uncompress((Bytef*)spStream->getBuffer(),&destlen, buf, buf.size() );
//		if (isok==Z_OK)
//		{
//			spStream->seek(0, IFStream::ISSF_BEGIN);
//			return spStream;
//		}
//
//	}
//	return NULL;
//}
//


/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <errno.h>

struct data
{
	unsigned long head;
	unsigned short pk;
	unsigned short q;
	unsigned short type;
	unsigned short time;
	unsigned short date;
	unsigned long crc;
	unsigned long d_len;
	unsigned long len;
	unsigned short f_len;
	unsigned short e_len;
};

void error_quit(const char *msg)
{
	perror(msg);
	exit(-1);
}

void memcat(char *tmp,char *buf,unsigned long len)
{
	int i=2,j=0;

	while(len)
	{
		tmp[i]=buf[j];

		++i;
		++j;
		--len;
	}
}

int get_zip_file_num(char *zip_file)
{
	int i=0;
	FILE *fp;
	struct data zip;

	if((fp=fopen(zip_file,"rb"))==NULL)
		return -1;

	while(!feof(fp))
	{
		fread(&zip,sizeof(struct data),1,fp);
		if(zip.f_len <= 0)
			break;
		IFString sFileName;
		sFileName.resize()
		//fseek(fp,zip.f_len,SEEK_CUR);

		if(zip.e_len > 0)
			fseek(fp,zip.e_len,SEEK_CUR);

		if(zip.d_len > 0)
			fseek(fp,zip.d_len,SEEK_CUR);

		++i;
	}

	fclose(fp);

	return i;
}

void unzip(char *zip_file)
{
	struct data zip;
	FILE *fp,*out;
	char *buf,*source,*name,*tmp;
	unsigned short s=0x9c78;
	unsigned long blen;

	if((fp=fopen(zip_file,"rb"))==NULL)
		error_quit("Open File");

	while(!feof(fp))
	{
		fread(&zip,sizeof(struct data),1,fp);
		if(zip.f_len <= 0)
			break;
		name=malloc(zip.f_len+1);
		bzero(name,zip.f_len+1);
		fread(name,zip.f_len,1,fp);

		if(zip.e_len > 0)
			fseek(fp,zip.e_len,SEEK_CUR);

		if(zip.d_len == 0)
			mkdir(name,0777);
		else if(zip.d_len == zip.len)
		{
			if((out=fopen(name,"w"))==NULL)
				error_quit("Create File");

			printf("Unzip %s . . .\n",name);

			buf=malloc(zip.d_len+1);
			bzero(buf,zip.d_len+1);

			fread(buf,zip.d_len,1,fp);
			fwrite(buf,zip.d_len,1,out);

			fclose(out);
			free(buf);

			printf("Unzip %s Successed . . .\n",name);
		}
		else
		{
			buf=malloc(zip.d_len+1);
			bzero(buf,zip.d_len+1);
			fread(buf,zip.d_len,1,fp);

			source=malloc(zip.len+1);
			bzero(source,zip.len+1);

			tmp=malloc(zip.d_len+3);
			bzero(tmp,zip.d_len+3);
			memcpy(tmp,&s,sizeof(s));
			memcat(tmp,buf,zip.d_len);

			if((out=fopen(name,"w"))==NULL)
				error_quit("Create File");

			printf("Unzip %s . . .\n",name);

			blen=compressBound(zip.len);
			uncompress(source,&blen,tmp,zip.len);

			fwrite(source,zip.len,1,out);

			fclose(out);
			free(source);
			free(tmp);
			free(buf);

			printf("Unzip %s Successed . . .\n",name);
		}

		free(name);
	}

	fclose(fp);
}

int main(int argc,char **argv)
{
	if(argc != 2)
		error_quit("Argument Error!");

	printf("%d files in %s\n",get_zip_file_num(argv[1]),argv[1]);

	unzip(argv[1]);

	return 0;
}
*/

class IFMinZip : public IFZip
{
public:
	IFI64 m_start_offset;
	~IFMinZip()
	{
		close();
	}
	IFMinZip()
		:m_zipFile(NULL)
	{
		m_Filefun.opaque = this;
		m_Filefun.zopen64_file = ifs_open;
		m_Filefun.zwrite_file = ifs_write;

		m_Filefun.zread_file = ifs_read;
		m_Filefun.ztell64_file = ifs_tell;
		m_Filefun.zseek64_file = ifs_seek;
		m_Filefun.zclose_file = ifs_close;
	}

	static voidpf ifs_open(voidpf opaque, const void* filename, int mode)
	{
		IFMinZip* spThis = (IFMinZip*)opaque;
		return spThis->m_spStream;
	}

	static uLong ifs_read(voidpf opaque, voidpf stream, void* buf, uLong size)
	{
		IFMinZip* spThis = (IFMinZip*)opaque;
		return spThis->m_spStream->read(buf,size);
	}

	static uLong ifs_write(voidpf opaque, voidpf stream, const void* buf, uLong size)
	{
		IFMinZip* spThis = (IFMinZip*)opaque;
		return spThis->m_spStream->write(buf,size);
	}

	static ZPOS64_T ifs_tell(voidpf opaque, voidpf stream)
	{
		IFMinZip* spThis = (IFMinZip*)opaque;
		return spThis->m_spStream->tell() - spThis->m_start_offset;
	}	

	static long ifs_seek(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
	{
		IFMinZip* spThis = (IFMinZip*)opaque;
		if (origin == SEEK_SET)
			offset += spThis->m_start_offset;
		spThis->m_spStream->seek(offset, origin);
		return  0;
	}

	static int ifs_close(voidpf opaque, voidpf stream)
	{
		return 0;
	}


	virtual bool open(IFRefPtr<IFStream> spStream)
	{
		m_spStream = spStream;
		m_start_offset = spStream->tell();
		m_zipFile = zipOpen2_64("",0,0, &m_Filefun);
		if (!m_zipFile)
			return false;
		return true;

	}
	virtual bool addFile(const IFString& sFileName, IFStream* pStream)
	{
		zip_fileinfo zipinfo;
		ZeroMemory(&zipinfo, sizeof(zipinfo));
		
		int r = zipOpenNewFileInZip64(m_zipFile, sFileName.c_str(), &zipinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 1);
		if (r!=0)
			return false;

		if (pStream)
		{
			char buf[32*1024];
			while (1)
			{
				int r = pStream->read(buf,sizeof(buf));
				if (r>0)
				{
					zipWriteInFileInZip(m_zipFile, buf, r);
				}
				else
					break;
			}
		}
		zipCloseFileInZip(m_zipFile);

		return true;
	}
	virtual void close()
	{
		if (m_zipFile)
		{
			zipClose(m_zipFile, "");
			m_zipFile = NULL;
		}
		m_spStream = NULL;


	}
	zlib_filefunc64_def m_Filefun;
	zipFile m_zipFile;
	IFRefPtr<IFStream> m_spStream;

};


IFRefPtr<IFUnZip> IFUnZip::create()
{
	return IFNew IFMinUnZip();
}

IFRefPtr<IFZip> IFZip::create()
{
	return IFNew IFMinZip();;
}

IF_DEFINERTTI(IFZip, IFRefObj);
IF_DEFINERTTI(IFUnZip, IFRefObj);