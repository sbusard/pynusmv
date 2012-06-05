#! /bin/sh

tool=zchaff
tool_settings=${tool}-default

# loads config file (precedence to user-defined settings)
if [ -f $tool_settings ] ; then 
   echo Using user-defined $tool_settings
   . ./$tool_settings
elif [ -f $tool_settings.in ] ; then 
   . ./$tool_settings.in
fi
# ----------------------------------------------------------------------

tool_dir=$zchaff_dir
tool_zip=$zchaff_zip
tool_url=$zchaff_url
tool_patch=$zchaff_patch
tool_link=../$zchaff_link
tool_targets="libsat.a SAT_C.h"

. ../MiniSat/gen_maker.sh
