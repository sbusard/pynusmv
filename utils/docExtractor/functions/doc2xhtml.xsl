<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2006/xpath-functions">
<xsl:output indent="no" />

	<xsl:template match="/">
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
				<title>Documentation</title>
				<link rel="stylesheet" type="text/css" href="html/htmlDoc.css" />
				<script src="html/jquery-1.7.1.min.js" type="text/javascript">skip;</script>
				<script src="html/htmlDoc.js" type="text/javascript">skip;</script>				
			</head>
			<body>
				<xsl:if test="count(functionlist/directory | functionlist/file) > 0">			
					<ul id="functionlist">
						<xsl:apply-templates select="functionlist/directory" />
						<xsl:apply-templates select="functionlist/file" />
					</ul>
				</xsl:if>
			</body>			
		</html>
	</xsl:template>
	
	<xsl:template match="directory">
		<li>
			<span class="expander directory">
				<xsl:value-of select="./attribute::path" />
			</span>
			<xsl:if test="count(directory | file) > 0">
				<ul class="expandable">
					<xsl:apply-templates select="directory" />
					<xsl:apply-templates select="file" />
				</ul>
			</xsl:if>
		</li>
	</xsl:template>
	
	<xsl:template match="file">
		<li>
			<span class="file expander">
				<xsl:value-of select="./attribute::path" />
			</span>
			<xsl:if test="count(func) > 0">
				<ul class="funcs expandable">
					<xsl:apply-templates select="func" />
				</ul>
			</xsl:if>
		</li>
	</xsl:template>
	
	<xsl:template match="func">
		<li class="function">
			<pre class="expander signature">
				<xsl:value-of select="sig" />
			</pre>
			<pre class="expandable specs">
				<xsl:value-of select="doc" />
			</pre>
		</li>
	</xsl:template>

</xsl:stylesheet>