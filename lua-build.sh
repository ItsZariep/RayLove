source ~/emsdk-3.1.29/emsdk_env.sh

curl -LO https://www.lua.org/ftp/lua-5.2.4.tar.gz
tar xzf lua-5.2.4.tar.gz
cd lua-5.2.4/src

for f in *.c; do
	case $f in lua.c|luac.c) continue;; esac
	emcc -O2 -DLUA_COMPAT_ALL -c $f
done
emar rcs liblua.a *.o
ls -la liblua.a
