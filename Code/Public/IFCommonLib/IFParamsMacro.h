﻿/*
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
#undef IF_TEMPALTE_PARAM_DEFINE0
#undef IF_TEMPALTE_PARAM_DEFINE1
#undef IF_TEMPALTE_PARAM_DEFINE2
#undef IF_TEMPALTE_PARAM_DEFINE3
#undef IF_TEMPALTE_PARAM_DEFINE4
#undef IF_TEMPALTE_PARAM_DEFINE5
#undef IF_TEMPALTE_PARAM_DEFINE6
#undef IF_TEMPALTE_PARAM_DEFINE7
#undef IF_TEMPALTE_PARAM_DEFINE8
#undef IF_TEMPALTE_PARAM_DEFINE9

#undef IF_FUNCTION_PARAM_DEFINE0
#undef IF_FUNCTION_PARAM_DEFINE1
#undef IF_FUNCTION_PARAM_DEFINE2
#undef IF_FUNCTION_PARAM_DEFINE3
#undef IF_FUNCTION_PARAM_DEFINE4
#undef IF_FUNCTION_PARAM_DEFINE5
#undef IF_FUNCTION_PARAM_DEFINE6
#undef IF_FUNCTION_PARAM_DEFINE7
#undef IF_FUNCTION_PARAM_DEFINE8
#undef IF_FUNCTION_PARAM_DEFINE9

#undef IF_FUNCTION_PARAM_CALL0
#undef IF_FUNCTION_PARAM_CALL1
#undef IF_FUNCTION_PARAM_CALL2
#undef IF_FUNCTION_PARAM_CALL3
#undef IF_FUNCTION_PARAM_CALL4
#undef IF_FUNCTION_PARAM_CALL5
#undef IF_FUNCTION_PARAM_CALL6
#undef IF_FUNCTION_PARAM_CALL7
#undef IF_FUNCTION_PARAM_CALL8
#undef IF_FUNCTION_PARAM_CALL9

#define IF_TEMPALTE_PARAM_DEFINE0(TYPE) typename TYPE##0
#define IF_TEMPALTE_PARAM_DEFINE1(TYPE) IF_TEMPALTE_PARAM_DEFINE0(TYPE), typename TYPE##1
#define IF_TEMPALTE_PARAM_DEFINE2(TYPE) IF_TEMPALTE_PARAM_DEFINE1(TYPE), typename TYPE##2
#define IF_TEMPALTE_PARAM_DEFINE3(TYPE) IF_TEMPALTE_PARAM_DEFINE2(TYPE), typename TYPE##3
#define IF_TEMPALTE_PARAM_DEFINE4(TYPE) IF_TEMPALTE_PARAM_DEFINE3(TYPE), typename TYPE##4
#define IF_TEMPALTE_PARAM_DEFINE5(TYPE) IF_TEMPALTE_PARAM_DEFINE4(TYPE), typename TYPE##5
#define IF_TEMPALTE_PARAM_DEFINE6(TYPE) IF_TEMPALTE_PARAM_DEFINE5(TYPE), typename TYPE##6
#define IF_TEMPALTE_PARAM_DEFINE7(TYPE) IF_TEMPALTE_PARAM_DEFINE6(TYPE), typename TYPE##7
#define IF_TEMPALTE_PARAM_DEFINE8(TYPE) IF_TEMPALTE_PARAM_DEFINE7(TYPE), typename TYPE##8
#define IF_TEMPALTE_PARAM_DEFINE9(TYPE) IF_TEMPALTE_PARAM_DEFINE8(TYPE), typename TYPE##9


#define IF_FUNCTION_PARAM_DEFINE0(TYPE, PP)  TYPE##0 PP##0
#define IF_FUNCTION_PARAM_DEFINE1(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE0(TYPE,PP),TYPE##1 PP##1
#define IF_FUNCTION_PARAM_DEFINE2(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE1(TYPE,PP),TYPE##2 PP##2
#define IF_FUNCTION_PARAM_DEFINE3(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE2(TYPE,PP),TYPE##3 PP##3
#define IF_FUNCTION_PARAM_DEFINE4(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE3(TYPE,PP),TYPE##4 PP##4
#define IF_FUNCTION_PARAM_DEFINE5(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE4(TYPE,PP),TYPE##5 PP##5
#define IF_FUNCTION_PARAM_DEFINE6(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE5(TYPE,PP),TYPE##6 PP##6
#define IF_FUNCTION_PARAM_DEFINE7(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE6(TYPE,PP),TYPE##7 PP##7
#define IF_FUNCTION_PARAM_DEFINE8(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE7(TYPE,PP),TYPE##8 PP##8
#define IF_FUNCTION_PARAM_DEFINE9(TYPE, PP) 	IF_FUNCTION_PARAM_DEFINE8(TYPE,PP),TYPE##9 PP##9

#define IF_FUNCTION_PARAM_CALL0(PP) PP##0
#define IF_FUNCTION_PARAM_CALL1(PP) IF_FUNCTION_PARAM_CALL0(PP),PP##1
#define IF_FUNCTION_PARAM_CALL2(PP) IF_FUNCTION_PARAM_CALL1(PP),PP##2
#define IF_FUNCTION_PARAM_CALL3(PP) IF_FUNCTION_PARAM_CALL2(PP),PP##3
#define IF_FUNCTION_PARAM_CALL4(PP) IF_FUNCTION_PARAM_CALL3(PP),PP##4
#define IF_FUNCTION_PARAM_CALL5(PP) IF_FUNCTION_PARAM_CALL4(PP),PP##5
#define IF_FUNCTION_PARAM_CALL6(PP) IF_FUNCTION_PARAM_CALL5(PP),PP##6
#define IF_FUNCTION_PARAM_CALL7(PP) IF_FUNCTION_PARAM_CALL6(PP),PP##7
#define IF_FUNCTION_PARAM_CALL8(PP) IF_FUNCTION_PARAM_CALL7(PP),PP##8
#define IF_FUNCTION_PARAM_CALL9(PP) IF_FUNCTION_PARAM_CALL8(PP),PP##9

#undef IF_FUNCTION_PARAM_DEFINE
#undef IF_FUNCTION_PARAM_CALL
#undef IF_TEMPALTE_PARAM_DEFINE
#undef IF_FUNCTION_PARAM_SPLIT

#if		IF_FUNCTION_PARAM_NUM <= 0 
#define IF_FUNCTION_PARAM_SPLIT 
#else
#define IF_FUNCTION_PARAM_SPLIT ,

#endif

#if		IF_FUNCTION_PARAM_NUM <= 0 
#define IF_FUNCTION_PARAM_DEFINE(PP,pp) 
#define IF_FUNCTION_PARAM_CALL(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 1
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE0(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL0(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE0(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 2
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE1(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL1(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE1(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 3
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE2(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL2(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE2(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 4
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE3(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL3(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE3(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 5
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE4(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL4(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE4(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 6
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE5(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL5(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE5(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 7
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE6(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL6(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE6(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 8
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE7(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL7(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE7(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 9
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE8(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL8(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE8(P) 

#elif	IF_FUNCTION_PARAM_NUM <= 10
#define IF_FUNCTION_PARAM_DEFINE(PP,pp)		IF_FUNCTION_PARAM_DEFINE9(PP,pp)
#define IF_FUNCTION_PARAM_CALL(P)			IF_FUNCTION_PARAM_CALL9(P) 
#define IF_TEMPALTE_PARAM_DEFINE(P)			IF_TEMPALTE_PARAM_DEFINE9(P) 
#else
#error	NOT SUPPORT PARAM NUM > 10
#endif