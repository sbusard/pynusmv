<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2006/xpath-functions">
<xsl:output indent="yes" />

	<xsl:template match="/">
		<style type="text/css">
			li {
				list-style-type: none;
			}
		</style>
		<xsl:if test="count(functionlist/directory) > 0">
			<ul>
				<xsl:apply-templates select="functionlist/directory" />
			</ul>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="directory">
		<li>
			&gt; <xsl:value-of select="./attribute::path" />
			<xsl:if test="count(directory) > 0">
				<ul>
					<xsl:apply-templates select="directory" />
				</ul>
			</xsl:if>
		</li>
	</xsl:template>

</xsl:stylesheet>