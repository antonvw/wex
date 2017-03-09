<?xml version="1.0" encoding="ISO-8859-1"?>
<!--
Name:      lexers.xsl
Purpose:   Stylesheet for lexers.xml
Author:    Anton van Wezenbeek
Copyright: (c) 2017 Anton van Wezenbeek
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <!-- match on root and do for all children -->
  <xsl:template match="/">
    <html>
      <head>
        <style>
        table, th, td 
        {
          border: 1px solid black;
          border-collapse: collapse;
          vertical-align: top
        }
        th
        {
          background-color: #9acd32;
        }
        </style>      
      </head>
      <body>
        <table>
          <xsl:apply-templates select="lexers"/>
          <xsl:apply-templates select="//themes"/>
          <xsl:apply-templates select="//macro"/>
          <xsl:apply-templates select="//global"/>
          <xsl:apply-templates select="//keyword"/>
        </table>
      </body>
    </html>
  </xsl:template>
  
  <!-- lexer elements -->
  <xsl:template match="lexers">
    <tr><td><h2>lexers</h2>
      <table>
        <tr>
          <th>name</th>
          <th style="width: 100px">extensions</th>
          <th>macro</th>
          <th>comments</th>
          <th>properties</th>
          <th style="width: 350px">keywords</th>
        </tr>
        <xsl:apply-templates select="lexer">
          <xsl:sort select="@name"/>
        </xsl:apply-templates>
      </table>
    </td></tr>
  </xsl:template>

  <!-- themes elements -->
  <xsl:template match="themes">
    <tr><td><h2>themes</h2>
      <table>
        <tr>
          <th>name</th>
          <th>def</th>
        </tr>
        
        <xsl:for-each select="theme">
          <td><xsl:value-of select="@name"/></td>
          <td>
            <table>
              <tr>
                <th>no</th>
                <th>value</th>
              </tr>

              <xsl:for-each select="def">
                <tr>
                  <td><xsl:value-of select="@style"/></td>
                  <td>
                    <xsl:value-of select="."/>
                  </td>
                </tr>
              </xsl:for-each>
            </table>
            
          </td>
        </xsl:for-each>
  
      </table>
    </td></tr>
  </xsl:template>
  
  <!-- global elements -->
  <xsl:template match="global">
    <tr><td><h2>global</h2>
      <table>
        <tr>
          <th>marker no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="marker"/>
        
        <tr>
          <th>style no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="style"/>
      
        <tr>
          <th>hex no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="hex"/>
      
        <tr>
          <th>indicator no</th>
          <th>value</th>
        </tr>
        <xsl:apply-templates select="indicator"/>
      
        <tr>
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
      <table>
        <tr>
          <th>name</th>
          <th>def</th>
        </tr>
        
        <xsl:for-each select="def">
          <td><xsl:value-of select="@name"/></td>
          <td>
            <table>
              <tr>
                <th>no</th>
                <th>value</th>
              </tr>

              <xsl:variable name="offset">-1
              </xsl:variable>
          
              <xsl:for-each select="def">
                <tr>
                  <td><xsl:value-of select="@no"/></td>
                  <td>
                    <xsl:variable name="num">
                      <xsl:number level="single" count="def"/>
                    </xsl:variable>
                    <xsl:number value="$num + $offset"/>
                    <xsl:value-of select="."/>
                  </td>
                </tr>
              </xsl:for-each>
            </table>
          </td>
        </xsl:for-each>
  
      </table>
    </td></tr>
  </xsl:template>
  
  <!-- keyword sets -->
  <xsl:template match="keyword">
    <tr><td><h2>keyword sets</h2>
      <table>
        <tr>
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
      <td>
        <xsl:value-of select="@name"/>
        <xsl:if test="@display">
          <xsl:text> (</xsl:text>
          <xsl:value-of select="@display"/>
          <xsl:text>)</xsl:text>
        </xsl:if>
      </td>
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
        <xsl:for-each select="keywords/@*">
          set: <xsl:value-of select="."/> <xsl:text> </xsl:text>
        </xsl:for-each> 
        <br/>
        <xsl:value-of select="keywords"/>
      </td>
    </tr>
  </xsl:template>
  
  <xsl:template match="lexers/lexer/properties/property">
    <xsl:value-of select="@name"/>
    <xsl:value-of select="."/><br/>
  </xsl:template>
  
  <xsl:template match="keyword/set">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:value-of select="."/></td>
    </tr>
  </xsl:template>
    
</xsl:stylesheet>
