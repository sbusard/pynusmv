<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output indent="yes" />

	<xsl:template match="/">
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
				<title>Documentation</title>
				<script src="jquery.js" type="text/javascript">skip;</script> 
				<script src="htmlDoc.js" type="text/javascript">skip;</script>
			</head>
			<body>				
				<ul id="functionlist">
					<xsl:apply-templates select="functionlist/directory" />
					<xsl:apply-templates select="functionlist/file" />
				</ul>
			</body>			
		</html>
	</xsl:template>
	
	<xsl:template match="directory">
		<li>
			<span class="path expander">
				<xsl:value-of select="./attribute::path" />
			</span>
			<ul class="directory expandable">
				<xsl:apply-templates select="directory" />
				<xsl:apply-templates select="file" />
			</ul>
		</li>
	</xsl:template>
	
	<xsl:template match="file">
		<li>
			<xsl:value-of select="./attribute::path" />
		</li>
	</xsl:template>

</xsl:stylesheet>