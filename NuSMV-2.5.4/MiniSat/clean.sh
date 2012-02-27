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
tool_link=../$minisat_link

RM="rm -f"
RMDIR="rm -rf"

# ----------------------------------------------------------------------
if [ -d ${tool_dir} ] ; then 
  $RMDIR ${tool_dir}
fi

if [ -h ${tool_link} ] ; then             
   $RM ${tool_link}
elif [ -r ${tool_link} ] ; then
   # this is for windows
   $RMDIR ${tool_link}
fi

if [ -f ${patched_tag} ]; then
  $RM ${patched_tag}
fi
