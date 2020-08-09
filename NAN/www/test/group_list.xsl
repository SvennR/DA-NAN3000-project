<?xml version="1.0" ?>
<xsl:stylesheet version="2.0"   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                                >
<!--xsl:output method="html"/> -->
<xsl:template match="/">
    <html>
    <body>
    <center><h2><i>Groups</i><br/>in various courses</h2></center>
    
    <xsl:for-each select="GROUPLIST/GROUP">
        <p>
            <xsl:value-of select="purpose"/>
        </p>
    </xsl:for-each>
    
    <table border="1" bgcolor="#ffffff">
    <tr>
        <th bgcolor="#c0c0c0" bordercolor="#000000">Course (attr)</th>
        <th bgcolor="#c0c0c0" bordercolor="#000000">Purpose</th>
    </tr>

    <xsl:for-each select="GROUPLIST/GROUP">
        <tr>
            <td bordercolor="#c0c0c0"><xsl:value-of select="@course"/></td>
            <td bordercolor="#c0c0c0"><xsl:value-of select="purpose"/></td>
        </tr>
    </xsl:for-each>
    </table>
    
    <table border="1" bgcolor="#ffffff">
    <tr>
        <th bgcolor="#c0c0c0" bordercolor="#000000">Course</th>
        <th bgcolor="#c0c0c0" bordercolor="#000000">Members</th>
        <th bgcolor="#c0c0c0" bordercolor="#000000">Role</th>
    </tr>

    <xsl:for-each select="GROUPLIST/GROUP/member">
        <tr>
            <td bordercolor="#c0c0c0"><xsl:value-of select="../@course"/></td>
            <td bordercolor="#c0c0c0"><xsl:value-of select="name/fname"/>
                                      <xsl:text> </xsl:text>   
                                      <xsl:value-of select="name/lname"/></td>
            <td bordercolor="#c0c0c0"><xsl:value-of select="role"/></td>
        </tr>
    </xsl:for-each>
    </table>
    
    </body>
    </html>
</xsl:template>
</xsl:stylesheet>