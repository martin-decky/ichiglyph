#
# Copyright (c) 2017 Martin Decky
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - The name of the author may not be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

BINARIES = brainfuck ichiglyph bf2ig ig2bf

.PHONY: all clean

all: $(BINARIES)

brainfuck:
	$(MAKE) -C interpreter/$@
	cp interpreter/$@/$@ ./$@

ichiglyph:
	$(MAKE) -C interpreter/$@
	cp interpreter/$@/$@ ./$@

bf2ig:
	$(MAKE) -C transpiler/$@
	cp transpiler/$@/$@ ./$@

ig2bf:
	$(MAKE) -C transpiler/$@
	cp transpiler/$@/$@ ./$@

clean:
	$(MAKE) -C interpreter/brainfuck clean
	$(MAKE) -C interpreter/ichiglyph clean
	$(MAKE) -C transpiler/bf2ig clean
	$(MAKE) -C transpiler/ig2bf clean
	rm -f $(BINARIES)
