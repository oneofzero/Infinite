

#undef IF_FUNCTION_PARAM_NUM
#define IF_FUNCTION_PARAM_NUM IF_REPEAT_INCLUDE_INDEX

#include "IFParamsMacro.h"



#if IF_FUNCTION_PARAM_NUM > 0
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 0
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 1
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 1
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 2
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 2
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 3
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 3
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 4
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 4
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 5
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 5
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 6
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 6
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 7
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 7
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 8
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 8
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif

#if IF_FUNCTION_PARAM_NUM > 9
#define IF_DP_TARGET_FUNCTION_PARAM_NUM 9
#include "IFFunctorDefaultParamSpec.h"
#undef IF_DP_TARGET_FUNCTION_PARAM_NUM
#endif


#if IF_FUNCTION_PARAM_NUM > 0

template<IF_TEMPALTE_PARAM_DEFINE(P)>
IFFunctorParam<IF_FUNCTION_PARAM_CALL(P)> makeIFDefaultParam(IF_FUNCTION_PARAM_DEFINE(P,p))
{
	return IFFunctorParam<IF_FUNCTION_PARAM_CALL(P)>(IF_FUNCTION_PARAM_CALL(p));
}
#endif
