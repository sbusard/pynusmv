/#.*/				{ next }
/\/\*\*Function/ 	{ 
					  if(s == 1) {
						print "]]></sig></func>" ;
					  }
					  f = 1 ;
					  s = 1 ;
					  print "<func>" ; 
					  print "<doc><![CDATA[" ;
					}
					{ if (s == 1) {
						if (f == 0) {
							sub(/^[ \t]+/, "", $0) ;
							sub(/[ \t]+$/, "", $0) ;
							if(index($0,"\{") > 0) {
								if(length($0) > 1) {
									print substr($0, 1, index($0,"\{") - 1)
								}
							} else {
								print
							}
						} else {
							print
						}
					  }
					}
/\*\// 				{ if(f == 1) {
						f = 0 ;
					  	if (s == 1) {
							print "]]></doc>" ;
							print "<sig><![CDATA["
					  	}
					  }
					}
/[^\{]*\{/			{ if (f == 0) {
						if (s == 1) {
							print "]]></sig>"
							print "</func>"
						}
						s = 0
					  }
					}