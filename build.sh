#!/bin/bash
skip_pkgs="bash,bzip2-libs,c-ares,cmake,coreutils,diffutils,eglibc,elfutils-libelf,elfutilslibs,elfutils,fdupes,file,findutils,gawk,gmp,libacl,libattr,libcap,libcurl,libfile,libgcc,liblua,libstdc++,make,mpc,mpfr,ncurses-libs,nodejs,nspr,nss-softokn-freebl,nss,openssl,patch,popt,rpmlibs,rpm-build,sed,sqlite,tar,xz-libs,binutils,gcc,filesystem,aul,libmm-sound,libtool,syspopup,notification,libva,libzypp-bindings"
gbs build -A armv7l --threads=16 --include-all --exclude=${skip_pkgs} --clean-once # --clean-repos
