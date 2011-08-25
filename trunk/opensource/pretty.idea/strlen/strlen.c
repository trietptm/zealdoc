<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>strlen.c</title>
<style type="text/css">
.enscript-comment { font-style: italic; color: rgb(178,34,34); }
.enscript-function-name { font-weight: bold; color: rgb(0,0,255); }
.enscript-variable-name { font-weight: bold; color: rgb(184,134,11); }
.enscript-keyword { font-weight: bold; color: rgb(160,32,240); }
.enscript-reference { font-weight: bold; color: rgb(95,158,160); }
.enscript-string { font-weight: bold; color: rgb(188,143,143); }
.enscript-builtin { font-weight: bold; color: rgb(218,112,214); }
.enscript-type { font-weight: bold; color: rgb(34,139,34); }
.enscript-highlight { text-decoration: underline; color: 0; }
</style>
</head>
<body id="top">
<h1 style="margin:8px;" id="f1">strlen.c&nbsp;&nbsp;&nbsp;<span style="font-weight: normal; font-size: 0.5em;">[<a href="?txt">plain text</a>]</span></h1>
<hr/>
<div></div>
<pre>
<span class="enscript-comment">/*-
 * Copyright (c) 2009 Xin LI &lt;<a href="mailto:delphij@FreeBSD.org">delphij@FreeBSD.org</a>&gt;
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */</span>

#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;sys/cdefs.h&gt;</span>
<span class="enscript-function-name">__FBSDID</span>(<span class="enscript-string">&quot;$FreeBSD: src/lib/libc/string/strlen.c,v 1.7 2009/01/26 07:31:28 delphij Exp $&quot;</span>);

#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;sys/limits.h&gt;</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;sys/types.h&gt;</span>
#<span class="enscript-reference">include</span> <span class="enscript-string">&lt;string.h&gt;</span>

<span class="enscript-comment">/*
 * Portable strlen() for 32-bit and 64-bit systems.
 *
 * Rationale: it is generally much more efficient to do word length
 * operations and avoid branches on modern computer systems, as
 * compared to byte-length operations with a lot of branches.
 *
 * The expression:
 *
 *	((x - 0x01....01) &amp; ~x &amp; 0x80....80)
 *
 * would evaluate to a non-zero value iff any of the bytes in the
 * original word is zero.  However, we can further reduce ~1/3 of
 * time if we consider that strlen() usually operate on 7-bit ASCII
 * by employing the following expression, which allows false positive
 * when high bit of 1 and use the tail case to catch these case:
 *
 *	((x - 0x01....01) &amp; 0x80....80)
 *
 * This is more than 5.2 times as fast as the raw implementation on
 * Intel T7300 under long mode for strings longer than word length.
 */</span>

<span class="enscript-comment">/* Magic numbers for the algorithm */</span>
#<span class="enscript-reference">if</span> <span class="enscript-variable-name">LONG_BIT</span> == 32
<span class="enscript-type">static</span> <span class="enscript-type">const</span> <span class="enscript-type">unsigned</span> <span class="enscript-type">long</span> mask01 = 0x01010101;
<span class="enscript-type">static</span> <span class="enscript-type">const</span> <span class="enscript-type">unsigned</span> <span class="enscript-type">long</span> mask80 = 0x80808080;
#<span class="enscript-reference">elif</span> <span class="enscript-variable-name">LONG_BIT</span> == 64
<span class="enscript-type">static</span> <span class="enscript-type">const</span> <span class="enscript-type">unsigned</span> <span class="enscript-type">long</span> mask01 = 0x0101010101010101;
<span class="enscript-type">static</span> <span class="enscript-type">const</span> <span class="enscript-type">unsigned</span> <span class="enscript-type">long</span> mask80 = 0x8080808080808080;
#<span class="enscript-reference">else</span>
#<span class="enscript-reference">error</span> <span class="enscript-variable-name">Unsupported</span> <span class="enscript-variable-name">word</span> <span class="enscript-variable-name">size</span>
#<span class="enscript-reference">endif</span>

#<span class="enscript-reference">define</span>	<span class="enscript-variable-name">LONGPTR_MASK</span> (sizeof(long) - 1)

<span class="enscript-comment">/*
 * Helper macro to return string length if we caught the zero
 * byte.
 */</span>
#<span class="enscript-reference">define</span> <span class="enscript-function-name">testbyte</span>(x)				\
	<span class="enscript-keyword">do</span> {					\
		<span class="enscript-keyword">if</span> (p[x] == <span class="enscript-string">'\0'</span>)		\
		    <span class="enscript-keyword">return</span> (p - str + x);	\
	} <span class="enscript-keyword">while</span> (0)

size_t
<span class="enscript-function-name">strlen</span>(<span class="enscript-type">const</span> <span class="enscript-type">char</span> *str)
{
	<span class="enscript-type">const</span> <span class="enscript-type">char</span> *p;
	<span class="enscript-type">const</span> <span class="enscript-type">unsigned</span> <span class="enscript-type">long</span> *lp;

	<span class="enscript-comment">/* Skip the first few bytes until we have an aligned p */</span>
	<span class="enscript-keyword">for</span> (p = str; (uintptr_t)p &amp; LONGPTR_MASK; p++)
	    <span class="enscript-keyword">if</span> (*p == <span class="enscript-string">'\0'</span>)
		<span class="enscript-keyword">return</span> (p - str);

	<span class="enscript-comment">/* Scan the rest of the string using word sized operation */</span>
	<span class="enscript-keyword">for</span> (lp = (<span class="enscript-type">const</span> <span class="enscript-type">unsigned</span> <span class="enscript-type">long</span> *)p; ; lp++)
	    <span class="enscript-keyword">if</span> ((*lp - mask01) &amp; mask80) {
		p = (<span class="enscript-type">const</span> <span class="enscript-type">char</span> *)(lp);
		testbyte(0);
		testbyte(1);
		testbyte(2);
		testbyte(3);
#<span class="enscript-reference">if</span> (<span class="enscript-variable-name">LONG_BIT</span> &gt;= 64)
		testbyte(4);
		testbyte(5);
		testbyte(6);
		testbyte(7);
#<span class="enscript-reference">endif</span>
	    }

	<span class="enscript-comment">/* NOTREACHED */</span>
	<span class="enscript-keyword">return</span> (0);
}

</pre>
<hr />
</body></html>