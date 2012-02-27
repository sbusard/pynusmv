#! /bin/sh

tool=minisat
tool_settings=${tool}-default

# loads config file (precedence to user-defined settings)
if [ -f $tool_settings ] ; then 
   echo Using user-defined $tool_settings
   . ./$tool_settings
elif [ -f $tool_settings.in ] ; then 
   . ./$tool_settings.in
fi
# ----------------------------------------------------------------------

tool_dir=$minisat_dir
tool_zip=$minisat_zip
tool_url=$minisat_url
tool_patch=$minisat_patch
tool_link=../$minisat_link
tool_targets=lib

. ./gen_maker.sh
