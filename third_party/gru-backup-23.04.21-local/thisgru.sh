
if [ "x${BASH_ARGV[0]}" = "x" ]; then
    if [ ! -f bin/thisgru.sh ]; then
        echo ERROR: must "cd where/GRU/is" before calling ". bin/thisgru.sh" for this version of bash!
        GRUDIR=; export GRUDIR
        return
    fi
    GRUDIR="$PWD"; export GRUDIR
else
    # get param to "."
    THIS=$(dirname ${BASH_ARGV[0]})
    GRUDIR=$(cd ${THIS};pwd); export GRUDIR
fi

if [ -z "${PATH}" ]; then
   PATH=$GRUDIR/bin; export PATH
else
   PATH=$GRUDIR/bin:$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$GRUDIR/lib; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH=$GRUDIR/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=$GRUDIR/lib; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=$GRUDIR/lib:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi