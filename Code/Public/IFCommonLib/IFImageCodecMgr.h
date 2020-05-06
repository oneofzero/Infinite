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
#pragma once
#include "IFUtility.h"
#include "IFRefObj.h"
#include "IFTypes.h"
#include "IFMemStream.h"
#include "IFAttributeSet.h"

class IFCOMMON_API IFSourceImageBuffer : public IFRefObj
{
	IF_DECLARERTTI
public:
	IFSourceImageBuffer();


	IFUI32 getFrameCount() const;

	const void* getData(int nFrame = 0) const;

	IFUI32 getDataSize(int nFrame = 0) const ;

	void* getData(int nFrame = 0);

	IFUI32 getWidth(int nFrame = 0) const;
	IFUI32 getHeight(int nFrame = 0) const;
	IFUI32 getOriginalWidth(int nFrame = 0) const;
	IFUI32 getOriginalHeight(int nFrame = 0) const;

	float getFrameDelay(int nFrame = 0 ) const;
	int getXOffset(int nFrame = 0) const;
	int getYOffset(int nFrame = 0) const;
	IFTextureFormat getImageType() const;

	IFRefPtr<IFSourceImageBuffer> scale(float percent) const;

	void init(IFTextureFormat eType, int nFrameCount);	//will clear all data

	bool setData(const void* pData, IFUI32 nDataSize, int nFrame,  IFUI32 nW, IFUI32 nH, int nXOffset, int nYOffset,float fFrameDelay, IFUI32 nOriW, IFUI32 nOriH );

	void clear();

	void setExternalInfo(const IFString& key, const IFString& val);
	bool getExternalInfo(const IFString& key, IFString& val) const;
	const IFMap<IFString,IFString>& getExternalInfo() const;
protected:
	virtual ~IFSourceImageBuffer();

private:
	IFMap<IFString,IFString> extenalInfo;

	struct FrameInfo
	{
		FrameInfo():pData(NULL),fFrameDelay(0)
		{

		}
		~FrameInfo()
		{
		}
		void* pData;
		IFUI32 nWidth;
		IFUI32 nHeight;
		IFUI32 nOriW;
		IFUI32 nOriH;
		int nXOffset;
		int nYOffset;
		float fFrameDelay;
	};


	typedef IFArray<FrameInfo> FrameList;
	FrameList m_FrameList;
	//int m_nWidth;
	//int m_nHeight;

	IFTextureFormat m_eImageType;
	//char* m_pBuffer;

};



class IFCOMMON_API IFImageCodec : public IFAttributeSet
{
	IF_DECLARERTTI;
public:
	virtual IFRefPtr< IFSourceImageBuffer> decode( IFStream* pStream ) const = 0;
	virtual bool encode( IFStream* pStream, const IFSourceImageBuffer* pInBuf) const = 0;
	const IFString& getName()
	{
		return m_sName;
	}
protected:
	IFString m_sName;
};

class IFCOMMON_API IFImageCodecMgr : public IFSingleton<IFImageCodecMgr> , public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	typedef IFArray<IFRefPtr<IFImageCodec> > CodecList;

	IFImageCodecMgr(void);
	~IFImageCodecMgr(void);

	bool installCodecoder(IFImageCodec* pDecoder);
	bool uninstallCodecoder(IFImageCodec* pDecoder);

	IFRefPtr<IFSourceImageBuffer> decode(IFStream* pStream) const;
	bool encode(IFStream* pStream, IFSourceImageBuffer* pIn, const IFString& sCoderName);

	const CodecList& getCodecoderList();
private:


	CodecList m_CodecList;
};
