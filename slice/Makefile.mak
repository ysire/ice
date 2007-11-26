# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

root_dir	= ..

!include $(root_dir)/config/Make.rules.mak

SUBDIRS		= Freeze \
		  Glacier2 \
		  Ice \
		  IceBox \
		  IcePatch2 \
		  IceStorm \
		  IceGrid

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1