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
#include "IFImageCodecMgr.h"

IF_DEFINERTTIROOT(IFSourceImageBuffer);
IF_DEFINERTTIROOT(IFImageCodec );
IF_DEFINERTTIROOT(IFImageCodecMgr);

IF_DEFINESINGLETON(IFImageCodecMgr);
static IFImageCodecMgr s_IFImageCodecMgr;

IFSourceImageBuffer::IFSourceImageBuffer():
m_eImageType(ITF_A8R8G8B8)
{

}

IFSourceImageBuffer::~IFSourceImageBuffer()
{
clear();
}

void IFSourceImageBuffer::clear()
{
	for( int i = 0 ; i < (int)m_FrameList.size(); i ++ )
	{
		if( m_FrameList[i].pData )
		{
			IFAlloc::Dealloc(m_FrameList[i].pData);
			//delete[] m_FrameList[i].pData;
		}
	}
	m_FrameList.clear();
}

void IFSourceImageBuffer::init(IFTextureFormat eType, int nFrameCount)
{
	m_eImageType = eType;
	clear();
	m_FrameList.resize( nFrameCount );
}

bool IFSourceImageBuffer::setData(const void* pData, IFUI32 nDataSize, int nFrame,  IFUI32 nW, IFUI32 nH, int nXOffset, int nYOffset, float fFrameDelay, IFUI32 nOriW, IFUI32 nOriH )
{
	FrameInfo frameInfo;
	IFUI32 nWantDataSize = nW * nH * IFGetPixelBits(m_eImageType)/8;
	if( nDataSize && nDataSize != nWantDataSize)
		return false;
	else
		nDataSize = nWantDataSize;
	
	//if( nFrame >= (int)m_FrameList.size() )
	//	return false;

	frameInfo.pData = IFAlloc::Alloc(nDataSize);//new char[nDataSize];
	if(pData && nDataSize)
		memcpy( frameInfo.pData, pData, nDataSize );
	frameInfo.nWidth = nW;
	frameInfo.nHeight = nH;
	frameInfo.nOriW = nOriW;
	frameInfo.nOriH = nOriH;
	frameInfo.nXOffset = nXOffset;
	frameInfo.nYOffset = nYOffset;
	frameInfo.fFrameDelay = fFrameDelay;
	if( nFrame >= (int)m_FrameList.size() )
		m_FrameList.resize( nFrame + 1 );
	
	m_FrameList[nFrame] = frameInfo;

	return true;

}

IFUI32 IFSourceImageBuffer::getFrameCount() const
{
	return m_FrameList.size();
}

IFUI32 IFSourceImageBuffer::getWidth(int nFrame) const
{
	return m_FrameList[nFrame].nWidth;
}

IFUI32 IFSourceImageBuffer::getHeight(int nFrame) const
{
	return m_FrameList[nFrame].nHeight;
}

IFUI32 IFSourceImageBuffer::getOriginalWidth(int nFrame) const
{
	return m_FrameList[nFrame].nOriW;
}

IFUI32 IFSourceImageBuffer::getOriginalHeight(int nFrame) const
{
	return m_FrameList[nFrame].nOriH;
}

const void* IFSourceImageBuffer::getData(int nFrame /* = 0 */) const
{
	return m_FrameList[nFrame].pData;
}

void* IFSourceImageBuffer::getData( int nFrame )
{
	return m_FrameList[nFrame].pData;
}

float IFSourceImageBuffer::getFrameDelay(int nFrame /* = 0  */) const
{
	return m_FrameList[nFrame].fFrameDelay;
}

int IFSourceImageBuffer::getXOffset(int nFrame) const
{
	return m_FrameList[nFrame].nXOffset;
}

int IFSourceImageBuffer::getYOffset(int nFrame /* = 0 */) const
{
	return m_FrameList[nFrame].nYOffset;
}

IFTextureFormat IFSourceImageBuffer::getImageType() const
{
	return m_eImageType;
}

void IFSourceImageBuffer::setExternalInfo( const IFString& key, const IFString& val )
{
	extenalInfo[key] = val;
}

bool IFSourceImageBuffer::getExternalInfo( const IFString& key, IFString& val ) const
{
	IFMap<IFString,IFString>::const_iterator it = extenalInfo.find(key);
	if(it!=extenalInfo.end())
	{

		val = it->second;
		return true;
	}

	return false;
}

const IFMap<IFString,IFString>& IFSourceImageBuffer::getExternalInfo() const
{
	return extenalInfo;
}

IFUI32 IFSourceImageBuffer::getDataSize( int nFrame /*= 0*/ ) const
{
	int nPixelSize = IFGetPixelBits(m_eImageType);

	 return m_FrameList[nFrame].nWidth * m_FrameList[nFrame].nHeight * nPixelSize/8;
	 

}


#define GET_PIXEL(px,py) (&pData[((px)*nOW/nDW)+((py)*nOH/nDH)*nOW])
IFRefPtr<IFSourceImageBuffer> IFSourceImageBuffer::scale( float percent ) const
{
	if (m_eImageType != ITF_A8R8G8B8)
		return NULL;
	if (percent==0.0f || percent==1.0f)
		return this;
	IFRefPtr<IFSourceImageBuffer> spDSIB = IFNew IFSourceImageBuffer;
	spDSIB->init(getImageType(), getFrameCount());


	for (int i = 0; i < m_FrameList.size(); i ++ )
	{
		IFUI32* pData = (IFUI32*)m_FrameList[i].pData;
		int nOW = m_FrameList[i].nWidth;
		int nOH = m_FrameList[i].nHeight;

		int nDW = int(nOW*percent);
		int nDH = int(nOH*percent);

		spDSIB->setData(0,0,i, nDW, nDH, m_FrameList[i].nXOffset*percent, m_FrameList[i].nYOffset*percent, m_FrameList[i].fFrameDelay, 
			m_FrameList[i].nOriW*percent,m_FrameList[i].nOriH*percent
			);

		IFUI32* pDestData = (IFUI32*)spDSIB->getData(i);
		if (nOW>nDW)
		{
			int nPixelSize = 1024*nOW/nDW;
			for (int y = 0; y < nDH; y ++ )
			{
				for (int x = 0; x < nDW; x++)
				{
					int dx = x*nOW/nDW;
					int dy = y*nOH/nDH;
					//int mx = (x*nDW%nOW);
					//int my = (y*nDH%nOH);

					pDestData[x+y*nDW] = pData[dx+dy*nOW];
					

				}
			}
		}

	}

	return spDSIB;
}





//////////////////////////////////////////////////////////////////////////

IFImageCodecMgr::IFImageCodecMgr(void)
{
}

IFImageCodecMgr::~IFImageCodecMgr(void)
{
}


bool IFImageCodecMgr::installCodecoder(IFImageCodec* pDecoder)
{
	
	CodecList::iterator it = m_CodecList.find(pDecoder);
	//CodecList::iterator it = std::find(m_CodecList.begin(), m_CodecList.end(), pDecoder );
	
	if( it != m_CodecList.end() )
		return false;

	m_CodecList.push_back( pDecoder );
	return true;
}
bool IFImageCodecMgr::uninstallCodecoder(IFImageCodec* pDecoder)
{
	CodecList::iterator it = m_CodecList.find( pDecoder );
	//CodecList::iterator it = std::find(m_CodecList.begin(), m_CodecList.end(), pDecoder );
	if( it != m_CodecList.end() )
	{
		m_CodecList.erase( it );
		return true;
	}
	return false;
}

IFRefPtr<IFSourceImageBuffer> IFImageCodecMgr::decode(IFStream* pStream) const
{
	IFI64 curPos = pStream->tell();
	for( int i = 0; i < (int)m_CodecList.size(); i ++ )
	{
		pStream->seek(curPos,SEEK_SET);
		IFRefPtr< IFSourceImageBuffer> spBuf = m_CodecList[i]->decode( pStream);
		if (spBuf)
			return spBuf;
	}
	return NULL;
}

const IFImageCodecMgr::CodecList& IFImageCodecMgr::getCodecoderList()
{
	return m_CodecList;
}

bool IFImageCodecMgr::encode( IFStream* pStream, IFSourceImageBuffer* pIn, const IFString& sCoderName )
{
	for( int i = 0; i < (int)m_CodecList.size(); i ++ )
	{
		if(m_CodecList[i]->getName() == sCoderName)
		{
			return m_CodecList[i]->encode(pStream, pIn);
		}
	}
	return false;
}
