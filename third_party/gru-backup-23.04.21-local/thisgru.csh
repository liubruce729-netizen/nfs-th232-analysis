#/bin/csh
set ARGS=($_)

if ("$ARGS" != "") then
   set thisgru="`dirname ${ARGS[2]}`"
else
   # But $_ might not be set if the script is source non-interactively.
   # In [t]csh the sourced file is inserted 'in place' inside the
   # outer script, so we need an external source of information
   # either via the current directory or an extra parameter.
   if ( -e thisgru.csh ) then
      set thisgru=${PWD}
   else if ( "$1" != "" ) then
      else if ( -e ${1}/thisgru.csh ) then
         set thisgru=${1}
      else 
         echo "thisgru.csh: ${1} does not contain a GRU installation"
      endif 
   else
      echo 'Error: The call to "source where_gru_is/bin/thisgru.csh" can not determine the location of the GRU installation'
      echo "because it was embedded another script (this is an issue specific to csh)."
      echo "Use either:"
      echo "   cd where_gru_is; source bin/thisgru.csh"
      echo "or"
      echo "   source where_gru_is/bin/thisgru.csh where_root_is" 
   endif
endif

# echo " Definition of GRU context\n" 
if ($?thisgru) then 
  setenv GRUDIR "`(cd ${thisgru};pwd)`"

  if !($?LD_LIBRARY_PATH) then
        setenv LD_LIBRARY_PATH   $GRUDIR/lib/
  else
        setenv LD_LIBRARY_PATH   $GRUDIR/lib/:$LD_LIBRARY_PATH
  endif



  if ($?DYLD_LIBRARY_PATH) then
     setenv DYLD_LIBRARY_PATH $ROOTSYS/lib:$DYLD_LIBRARY_PATH  # Mac OS X
  else
     setenv DYLD_LIBRARY_PATH $ROOTSYS/lib
  endif


  if !($?PATH) then
        setenv PATH   $GRUDIR/bin/
  else
        setenv PATH   $GRUDIR/bin/:$PATH
  endif
endif
