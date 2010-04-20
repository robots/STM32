#! gmake

#
# Copyright (C) 2006 Nicolas GRANDEMANGE
# 
# This file is part of canfestival, a library implementing the canopen
# stack
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 

all: manual

clean:
	cd en && \
	rm -f manual.aux manual.log manual.out manual.toc manual.pdf && \
	rm -rf html && \
	mkdir html

manual: clean
	cd en && \
	pdflatex manual.tex && \
	pdflatex manual.tex && \
	cp manual.pdf ../../../objdictgen/doc/manual_en.pdf && \
	latex2html -split 0 -dir html manual.tex
