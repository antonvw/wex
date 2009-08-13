<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
Name:      lexers.xsl
Purpose:   Stylesheet for lexers.xml
Author:    Anton van Wezenbeek
Created:   2009-08-13
RCS-ID:    $Id$
Copyright: (c) 2009 Anton van Wezenbeek
-->

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
  <html>
  <body>
  <table border="1">
  <tr>
    
    <td>
    <h2>Lexer</h2>
    <table border="1">
    <tr bgcolor="#9acd32">
      <th align="left">Variable</th>
      <th align="left">Value</th>
    </tr>
    <xsl:for-each select="lexers/lexer">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
    </xsl:for-each>
    </table>
    </td>
      
    <td>
    <h2>Global</h2>
    <table border="1">
    <tr bgcolor="#9acd32">
      <th align="left">Marker</th>
      <th align="left">Value</th>
    </tr>
    
    <xsl:for-each select="lexers/global">
    <tr>
      <td><xsl:value-of select="@marker"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
    </xsl:for-each>
    
    </table>
    </td>
    
  </tr>
  </table>
  </body>
  </html>
  
</xsl:template>

</xsl:stylesheet>
