#! gmake

#
# Copyright (C) 2006 Laurent Bessard
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

all: objdictedit canfestival examples

examples: canfestival driver
	$(MAKE) -C examples all

documentation:
	$(MAKE) -C doc/doxygen

manual:
	$(MAKE) -C doc/manual

objdictedit:
	$(MAKE) -C objdictgen all

canfestival: driver
	$(MAKE) -C src $@

driver:
	$(MAKE) -C drivers $@

install: canfestival driver
	$(MAKE) -C drivers $@
	$(MAKE) -C src $@
	$(MAKE) -C examples $@
	$(MAKE) -C objdictgen $@
	ldconfig

uninstall:
	$(MAKE) -C drivers $@
	$(MAKE) -C src $@
	$(MAKE) -C examples $@
	$(MAKE) -C objdictgen $@

clean:
	$(MAKE) -C src $@
	$(MAKE) -C drivers $@
	$(MAKE) -C examples $@
	$(MAKE) -C objdictgen $@
	
mrproper: clean
	$(MAKE) -C src $@
	$(MAKE) -C drivers $@
	$(MAKE) -C examples $@
	$(MAKE) -C objdictgen $@

