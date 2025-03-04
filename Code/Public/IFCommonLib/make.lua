﻿
return
{
	name="IFCommonLib",
	src_files = "./*.cpp;./rsa/*.c;./aes/*.c;./zlib/*.c",
	--compiler = "g++",
	
	base = function(proj)
			proj:RemoveSrc("IFComPort;CrashDump;IFAPKFileProvider;IFCoroutine;IFNetCoreIOCP;IFNetCoreSelect;gzio.c;gzlib.c;gzread.c;gzwrite.c;iowin32.c;minigzip.c;aes.c;example.c;gzclose.c");
			proj:RemoveSrc("IFWIN32FileStream.cpp")
			proj:AddIncludePath("./aes;./zlib;./rsa;../3rd/include");
		
	end,

	targets =
	{

		IFCommonLib = function(proj)
			proj.base(proj);
			proj:RemoveSrc("IFNetCoreWebSocket.cpp")
			proj:SetTargetType("so");
			proj:AddLinkFlag("-pthread");
			proj:AddLib("uuid");
			
			proj:AddLib("ssl")
			proj:AddLib("crypto");
			proj:SetOutputPath("../../../bin/libIFCommonLib.so");
			proj.linker = "g++";
		end,
		win = function(proj)
			require("win_config")
			
			proj:base()
			win_config(proj)
			proj:AddDefine("IFCOMMONLIB_EXPORTS")
			proj:AddDefine("USEIFMEMPOOLALLOC")
			proj:AddDefine("UNICODE")
			proj:AddDefine("_UNICODE")
			proj:AddFlag("/Zc:wchar_t")
			proj:AddFlag("/MD")
			proj.use_pch = "stdafx.h"
			proj.create_pch = "./stdafx.cpp"
			proj.no_pch_list = {["./dllmain.cpp"]=true}

			--proj:AddDefine("DEBUG")

			proj:AddSrc("IFNetCoreIOCP.cpp;IFComPort.cpp;CrashDump.cpp")
			proj:RemoveSrc("IFNetCoreEPOLL.cpp")
			proj:AddLib("Ws2_32.lib;crypto.lib;ssl.lib;Setupapi.lib")

			for k,v in ipairs(proj.src_files) do
				if getfileextname(v) =="c" then
					proj.no_pch_list[v] = true
				end
			end

			if proj.win_target == "x64" then
				proj:AddLibPath("../3rd/lib/x64");
				proj:SetOutputPath("../../../Bin64/IFCommonLib.dll");
			elseif proj.win_target == "x86" then
				proj:AddLibPath("../3rd/lib/x86");
				proj:SetOutputPath("../../../Bin/IFCommonLib.dll");
			else
				error("unknown platform" .. tostring(proj.win_target ))
			end
			
		end,
		linux = function(proj)
			proj.targets.IFCommonLib(proj);
			proj:AddLib("dl");
			proj:AddLibPath("../3rd/lib/linux/");
			proj:AddFlag("-fPIC");
			proj:AddDefine("_REENTRANT")
		end,
		linux_debug = function(proj)
			proj.targets.IFCommonLib(proj);
			proj:AddLib("dl");
			proj:AddDefine("DEBUG");
			proj:AddDefine("_DEBUG");
			proj:AddDefine("_REENTRANT")
			proj:AddLibPath("../3rd/lib/linux/");
			proj:SetOutputPath("../../../binD/libIFCommonLib.so");

		end,
		linux_static = function(proj)
			proj.targets.IFCommonLib(proj);
			proj:SetTargetType("a");
			proj:AddDefine("IFCOMMON_STATIC");
			proj:AddDefine("_REENTRANT")
			proj:AddFlag("-fPIC");
			proj:SetOutputPath("../../../bin/libIFCommonLib.a");

		end,
		linux_static_debug = function(proj)
			proj.targets.IFCommonLib(proj);
			proj:AddDefine("DEBUG")
			proj:SetTargetType("a");
			proj:AddDefine("IFCOMMON_STATIC");
			proj:AddDefine("_REENTRANT")
			proj:AddFlag("-fPIC");
			proj:SetOutputPath("../../../binD/libIFCommonLib.a");

		end,
		mac = function(proj)
			proj.targets.IFCommonLib(proj);
			proj:AddCXXFlag("-std=gnu++11")
			proj:RemoveSrc("IFNetCoreEPOLL.cpp")
			proj:AddLib("dl");
			proj:AddLibPath("../3rd/lib/mac/");
			proj:SetOutputPath("../../../binD/libIFCommonLib.so");
		end,

		android = function (proj, targettype)
			
			require "android_config"
			targettype = targettype or "so";
		
			proj.targets.IFCommonLib(proj);
			proj:SetTargetType(targettype);
			proj:AddLib("android");
			proj:AddSrc("IFAPKFileProvider.cpp");
			proj:AddSrc("android-ifarrds/ifaddrs.c")
			proj:AddFlag("-Wno-inconsistent-missing-override -Wno-parentheses -Wno-undefined-var-template")
		end,
		
		android_arm = function (proj, targettype  )
			proj.targets.android(proj,targettype);
			android_config(proj);
			--proj:AddDefine("DEBUG")
			proj:AddLibPath("../3rd/lib/android/arm/");
		end,
		android_arm64 = function (proj, targettype  )
			proj.targets.android(proj,targettype);
			android_config(proj, "android-21", "arm64", nil);		
			proj:AddLibPath("../3rd/lib/android/arm64/");
		end,
		android_x86 = function (proj,targettype)
			proj.targets.android(proj,targettype);
			android_config(proj, nil, "x86", nil);
			proj:AddLibPath("../3rd/lib/android/x86/");
		end,

		android_arm_static = function(proj)
			proj:AddDefine("IFCOMMON_STATIC");
			proj:AddDefine("DONT_USE_SSL")
			proj.targets.android_arm(proj, "a");
			
		end,
		android_arm64_static = function (proj  )
			proj:AddDefine("IFCOMMON_STATIC");
			proj:AddDefine("DONT_USE_SSL")
			proj.targets.android_arm64(proj,"a");
						
		end,
		android_x86_static = function(proj)
			proj:AddDefine("IFCOMMON_STATIC");
			proj.targets.android_x86(proj, "a");
		end,
		android_arm_unity = function(proj)
			proj:AddDefine("IFCOMMON_STATIC");
			proj:AddDefine("DONT_USE_SSL")
			proj:AddDefine("IFCOMMON_UNITY_SUPPORT");
			proj.targets.android_arm(proj, "a");
			
		end,
		android_arm64_unity = function(proj)
			proj:AddDefine("IFCOMMON_STATIC");
			proj:AddDefine("IFCOMMON_UNITY_SUPPORT");
			proj:AddDefine("DONT_USE_SSL")
			proj.targets.android_arm64(proj, "a");
		end,
		web = function(proj)
			require("web_config")(proj);
			--proj:AddCXXFlag("-pthread");
			proj:AddDefine("DONT_USE_SSL")
			proj:AddDefine("NDEBUG")
			proj:AddFlag("-Oz")
			proj:RemoveSrc("IFNetCoreEPOLL.cpp;IFNetCoreSelect.cpp;IFPipe.cpp;IFPipeCore.cpp")
			proj:RemoveSrc("IFFileLogStream.cpp;IFRemoteLogStream.cpp;IFWIN32FileStream.cpp;IFCFileStream.cpp")
			proj:RemoveSrc("IFHttpDownloader.cpp;IFThread.cpp;IFServerFrame.cpp;IFRedis.cpp;IFThreadSyncObj.cpp")

			proj:SetOutputPath("../../../web/libIFCommonLib.a");
			proj:AddDefine("IFCOMMON_STATIC");
		end,
		web_debug = function (proj)
			proj.targets.web(proj)
			proj:AddDefine("DEBUG")
			proj:RemoveDefine("NDEBUG")
		end,
	},
}