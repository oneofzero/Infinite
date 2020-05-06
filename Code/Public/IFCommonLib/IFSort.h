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

template<class T>
class IFDefaultSmaller
{
public:
	bool operator () ( T& a,  T& b) const
	{
		return *a < *b;
	}
};

template<class T>
class IFDefaultBigger
{
public:
	bool operator () ( T& a,  T& b) const
	{
		return *b < *a;
	}
};

template<class T>
class IFDefaultSwapper
{
public:
	void operator () (T& a, T& b) const
	{
		a.swap(b);
	}
};


template<class Titer, class Comparer, class Swapper>
void IFSort(Titer a, Titer b,const Comparer& compSmaller = IFDefaultSmaller<Titer>(), const Swapper& swapper = IFDefaultSwapper<Titer>())
{
	if(a==b)
		return;

	Titer key = a;
	Titer l = a;
	Titer r = b;

	while(l!=r)
	{
		do
		{
			--r;
			if(compSmaller(r,key))
			{
				swapper(key,r);
				//key.swap(r);
				key = r;
				break;
			}
		} while (r!=l);

		while(l!=r)
		{
			if(compSmaller(key,l))
			{
				swapper(key, l);
				//key.swap(l);
				key = l;
				break;
			}
			++l;
		}
	}

	IFSort(a,r,compSmaller, swapper);
	IFSort(r+1,b,compSmaller, swapper);


}


template<class Titer>
void IFSort(Titer a, Titer b)
{
	IFSort(a, b, IFDefaultSmaller<Titer>(), IFDefaultSwapper<Titer>());
}

template<class Titer, class Comparer>
void IFSort(Titer a, Titer b, const Comparer& cmp)
{
	IFSort(a, b, cmp, IFDefaultSwapper<Titer>());
}