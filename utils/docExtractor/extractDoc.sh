#! /usr/bin/env sh

function extract {
	echo "<file path=\"$2\">"
	awk -f computeFile.awk $1
	echo "</file>"
}

function extractAll {
	for p in `ls $1` ;
	do
		if [ -d "$1/$p" ]; then
			echo "<directory path=\"$p\">"
			extractAll "$1/$p"
			echo "</directory>"
		else
			if [ `echo "$p" | grep ".c$"` ] ; then
				extract "$1/$p" "$p"
			fi
		fi
	done
}

tmp="file$RANDOM"
path=$1
if [[ "${#path}" -gt "1" && "${path:${#path}-1:1}" = "/" ]] ; then
	path=${path:0:${#path}-1}
fi
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > $tmp
echo "<functionlist>" >> $tmp
extractAll $path >> $tmp
echo "</functionlist>" >> $tmp
iconv -f MacRoman -t UTF-8 $tmp | sed -e "s/©/(c)/g"
rm $tmp