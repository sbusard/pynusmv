<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2006/xpath-functions">
<xsl:output method="text" />

	<xsl:template match="/">
\documentclass{article}
		
\usepackage{moreverb}
		
\title{Documentation}
\author{Auto-generated document}
		
\begin{document}
	\maketitle
			
	\tableofcontents
			
		<xsl:if test="count(functionlist/directory | functionlist/file) > 0">
			<xsl:apply-templates select="functionlist/directory" />
			<xsl:apply-templates select="functionlist/file" />
		</xsl:if>
		
\end{document}
	</xsl:template>
	
	<xsl:template match="directory">
	\section{<xsl:value-of select="replace(./attribute::path,'_','\\_')" />}
		<xsl:if test="count(directory | file) > 0">
			<xsl:apply-templates select="directory" />
			<xsl:apply-templates select="file" />
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="file">
	\subsection{<xsl:value-of select="replace(./attribute::path,'_','\\_')" />}
		<xsl:if test="count(func) > 0">
			<xsl:apply-templates select="func" />
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="func">
	\texttt{<xsl:value-of select="normalize-space(replace(sig,'_','\\_'))" />}
	\begin{verbatimtab}
		<xsl:value-of select="doc" />
	\end{verbatimtab}
	</xsl:template>

</xsl:stylesheet>