<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  
    <xsl:template match="/">
      <html>
        <head>
		<link rel="stylesheet" type="text/css" href="/mp2.3/namexmlstyle.css"/>
        </head>
       <body>
 	<h1>Navnliste</h1>
 	<table border="1">
 	  <tr>
 	    <th> Navn</th> 
 	  </tr>
 	  <xsl:for-each select="root/message/name">
 	    <tr>
 	       <td><xsl:value-of select="."/></td>
 	    </tr>
 	  </xsl:for-each>
 	</table>
       </body>
     </html>
   </xsl:template>
 
 </xsl:stylesheet>
