


local srcs = "s2_meth.c \
	s2_srvr.c \
	s2_clnt.c \
	s2_lib.c \
	s2_enc.c \
	s2_pkt.c \
	s3_meth.c \
	s3_srvr.c \
	s3_clnt.c \
	s3_lib.c \
	s3_enc.c \
	s3_pkt.c \
	s3_both.c \
	s23_meth.c \
	s23_srvr.c \
	s23_clnt.c \
	s23_lib.c \
	s23_pkt.c \
	t1_meth.c \
	t1_srvr.c \
	t1_clnt.c \
	t1_lib.c \
	t1_enc.c \
	t1_reneg.c \
	d1_meth.c \
	d1_srvr.c \
	d1_clnt.c \
	d1_lib.c \
	d1_pkt.c \
	d1_both.c \
	d1_enc.c \
	d1_srtp.c \
	ssl_lib.c \
	ssl_err2.c \
	ssl_cert.c \
	ssl_sess.c \
	ssl_ciph.c \
	ssl_stat.c \
	ssl_rsa.c \
	ssl_asn1.c \
	ssl_txt.c \
	ssl_algs.c \
	bio_ssl.c \
	ssl_err.c \
	kssl.c \
	tls_srp.c";
	
local function splitstring(s, spliter)
	local r = {};
	for a in string.gmatch(s, "([^"..spliter .."]+)") do
		r[#r+1] = a;
	end
	return r;
end 
	
return
{
	name="crypto",
	src_files = function()
		local r = splitstring(srcs," \
	");
		return r;
		
	end,
	
	base = function(proj)
			--proj:RemoveSrc("CrashDump;IFAPKFileProvider;IFCoroutine;IFNetCoreIOCP;IFStackDumper;gzio.c;iowin32.c;minigzip.c");
			proj:AddIncludePath("./;../;../crypto;../include;../include/openssl;");
			proj:AddDefine("NO_WINDOWS_BRAINDEATH;OPENSSL_THREADS;_REENTRANT;DSO_DLFCN;HAVE_DLFCN_H;L_ENDIAN;OPENSSL_NO_EC_NISTP_64_GCC_128;OPENSSL_NO_SCTP;OPENSSL_NO_CAPIENG;OPENSSL_NO_CMS;OPENSSL_NO_GMP;OPENSSL_NO_JPAKE;OPENSSL_NO_MD2;OPENSSL_NO_MDC2;OPENSSL_NO_RC5;OPENSSL_NO_SHA0;OPENSSL_NO_RFC3779;OPENSSL_NO_STORE;OPENSSL_NO_WHIRLPOOL");
			proj:AddDefine("OPENSSL_NO_ASM");
	end,

	targets =
	{

		crypto = function(proj)
			proj.base(proj);
			proj:SetTargetType("lib");
			proj:AddLinkFlag("-pthread");
			proj:AddLib("uuid");
			
			
		end,
		linux = function(proj)
			proj.targets.crypto(proj);
			proj:AddFlag("-fPIC");
			proj:SetOutputPath("../../../Public/3rd/lib/linux/libssl.a");
		end,
		android_arm = function (proj )
			require "android_config"
			proj.targets.crypto(proj);
			android_config(proj);
			proj.mid_path = ("./arm/");
			proj:AddLib("android");
			--proj:AddSrc("IFAPKFileProvider.cpp");
			proj:SetOutputPath("../../../Public/3rd/lib/android/arm/libssl.a");
		end,
		android_x86 = function (proj)
			require "android_config"
			proj.targets.crypto(proj);
			proj:AddLib("android");
			
			--proj:AddSrc("IFAPKFileProvider.cpp");
			android_config(proj, nil, "x86", nil);
			proj:SetOutputPath("../../../Public/3rd/lib/android/x86/libssl.a");
		end
	},
}