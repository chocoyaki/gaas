<xsl:stylesheet version="1.0" 
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output omit-xml-declaration="yes" indent="yes"/>

    <xsl:param name="nb-vms" select="'1'"/>
	<xsl:param name="level" select="'4'"/>

    <xsl:template match="node()|@*">
        <xsl:copy>
            <xsl:apply-templates select="node()|@*"/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="node[@id='node-init-vms']/arg[@name='vm_count']/@value">
        <xsl:attribute name="value">
            <xsl:value-of select="$nb-vms"/>
        </xsl:attribute>
    </xsl:template>
    
    
    <xsl:template match="node[@id='node-ramses3d']/arg[@name='mpirun-np']/@value">
        <xsl:attribute name="value">
            <xsl:value-of select="$nb-vms"/>
        </xsl:attribute>
    </xsl:template>
    
    
	<xsl:template match="node[@id='node-grafic1']/arg[@name='level']/@value">
        <xsl:attribute name="value">
            <xsl:value-of select="$level"/>
        </xsl:attribute>
    </xsl:template>
    
</xsl:stylesheet>
