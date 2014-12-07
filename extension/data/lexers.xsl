<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
Name:      lexers.xsl
Purpose:   Stylesheet for lexers.xml
Author:    Anton van Wezenbeek
Copyright: (c) 2014 Anton van Wezenbeek
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <!-- match on root and do for all children -->
  <xsl:template match="/">
    <html>
      <body>
        <table border="1">
          <xsl:apply-templates select="lexers"/>
          <xsl:apply-templates select="//global"/>
          <xsl:apply-templates select="//macro"/>
          <xsl:apply-templates select="//keyword"/>
        </table>
      </body>
    </html>
  </xsl:template>
  
  <!-- lexer elements -->
  <xsl:template match="lexers">
    <tr><td><h2>lexers</h2>
      <table border="1" frame="box" rules="all">
        <tr bgcolor="#9acd32">
          <th>name</th>
          <th>display</th>
          <th>extensions</th>
          <th>macro</th>
          <th>comments</th>
          <th>properties</th>
          <th width="100">keywords</th>
        </tr>
        <xsl:apply-templates select="lexer"/>
      </table>
    </td></tr>
  </xsl:template>

  <!-- global elements -->
  <xsl:template match="global">
    <tr><td><h2>global</h2>
      <table border="1">
        <tr bgcolor="#9acd32">
          <th>marker no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="marker"/>
        
        <tr bgcolor="#9acd32">
          <th>style no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="style"/>
      
        <tr bgcolor="#9acd32">
          <th>hex no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="hex"/>
      
        <tr bgcolor="#9acd32">
          <th>indicator no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="indicator"/>
      
        <tr bgcolor="#9acd32">
          <th>property</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="lexers/global/property"/>
      </table>
    </td></tr>
  </xsl:template>
  
  <!-- macro elements -->
  <xsl:template match="macro">
    <tr><td><h2>macros</h2>
      <table border="1" frame="box" rules="all">
        <tr bgcolor="#9acd32">
          <th>name</th>
          <th>no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="def/def"/>
      </table>
    </td></tr>
  </xsl:template>
  
  <!-- keyword sets -->
  <xsl:template match="keyword">
    <tr><td><h2>keyword sets</h2>
      <table border="1">
        <tr bgcolor="#9acd32">
          <th>keyword set</th>
          <th>value</th>
        </tr>
      <xsl:apply-templates select="set"/>
      </table>
    </td></tr>
  </xsl:template>
  
  <!-- match on elements -->  
  <xsl:template match="hex">
    <tr>
      <td><xsl:value-of select="@no"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
  
  <xsl:template match="indicator">
    <tr>
      <td><xsl:value-of select="@no"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
  
  <xsl:template match="marker">
    <tr>
      <td><xsl:value-of select="@no"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
  
  <xsl:template match="style">
    <tr>
      <td><xsl:value-of select="@no"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
  
  <xsl:template match="global/properties/property">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
  
  <xsl:template match="lexers/lexer">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:value-of select="@display"/></td>
      <td><xsl:value-of select="@extensions"/></td>
      <td><xsl:value-of select="@macro"/></td>
      <td>
        <xsl:value-of select="comments/@begin1"/>
        <xsl:value-of select="comments/@begin2"/>
        <xsl:value-of select="comments/@end1"/>
        <xsl:value-of select="comments/@end2"/>
      </td>
      <td>
        <xsl:apply-templates select="properties"/>
      </td>
      <td>
        <xsl:value-of select="keywords"/>
        <xsl:apply-templates select="keywords/@*"/>
      </td>
    </tr>
  </xsl:template>
  
  <xsl:template match="lexers/lexer/property">
    <xsl:value-of select="@name"/><br/>
  </xsl:template>
  
  <xsl:template match="lexers/macro/def/def">
    <tr>
      <td><xsl:value-of select="./../@name"/></td>
      <td><xsl:value-of select="@no"/></td>
      <td>
        <xsl:number level="single" count="def"/>
        <xsl:value-of select="."/>
      </td>
    </tr>
  </xsl:template>
  
  <xsl:template match="keyword/set">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
    
</xsl:stylesheet>
