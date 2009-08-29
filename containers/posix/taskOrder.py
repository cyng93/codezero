# -*- mode: python; coding: utf-8; -*-

#  Codezero -- a microkernel for embedded systems.
#
#  Copyright © 2009  B Labs Ltd
#
#  This program is free software: you can redistribute it and/or modify it under the terms of the GNU
#  General Public License as published by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
#  License for more details.
#
#  You should have received a copy of the GNU General Public License along with this program.  If not, see
#  <http://www.gnu.org/licenses/>.
#
#  Author: Russel Winder

# A sequence determining the order of tasks in the packing.

####
####  TODO: Why do the tests only run when the load order is mm0, fs0, test0 -- any other order and the tests
####  do not run.  Worse if test0 is not last then there are problems with the compilation du to issues with
####  linker scripts.
####

taskOrder = ('mm0', 'fs0', 'test0')
#taskOrder = ('fs0', 'mm0', 'test0')