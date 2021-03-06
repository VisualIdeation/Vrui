########################################################################
# Makefile for the Vrui example programs.
# Copyright (c) 2003-2009 Oliver Kreylos
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
########################################################################

# Root directory of the Vrui software installation. This must match the
# same setting in Vrui's makefile. By default the directories match; if
# the installation directory was adjusted during Vrui's installation, it
# must be adjusted here as well.
VRUIDIR = /opt/local/Vrui-1.0

# Base installation directory for the example programs. If this is set
# to the default of $(PWD), the example programs do not have to be
# installed to be run. Created executables and resources will be
# installed in the bin and share directories under the given base
# directory, respectively.
# Important note: Do not use ~ as an abbreviation for the user's home
# directory here; use $(HOME) instead.
INSTALLDIR = $(shell pwd)

# Set up destination directories for compilation products:
OBJDIRBASE = o
BINDIRBASE = bin

# Set resource directory:
RESOURCEDIR = share

# Set up additional flags for the C++ compiler:
CFLAGS = 

# Include the Vrui application makefile fragment:
ifdef DEBUG
  # Build debug version of the applications, using the debug version of Vrui:
  include $(VRUIDIR)/etc/Vrui.debug.makeinclude
  CFLAGS += -g2 -O0
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)/debug
  BINDIR = $(BINDIRBASE)/debug
else
  # Build release version of the applications, using the release version of Vrui:
  include $(VRUIDIR)/etc/Vrui.makeinclude
  CFLAGS += -g0 -O3
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)
  BINDIR = $(BINDIRBASE)
endif

# Pattern rule to compile C++ sources:
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)//$(*D)
	@echo Compiling $<...
	@g++ -c -o $@ $(VRUI_CFLAGS) $(CFLAGS) $<

# Pattern rule to link executables:
$(BINDIR)/%: o/%.o
	@mkdir -p $(BINDIR)
	@echo Linking $@...
	@g++ -o $@ $^ $(VRUI_LINKFLAGS)
	
# List all project targets:
ALL = $(BINDIR)/VruiDemo \
      $(BINDIR)/VruiLocatorDemo \
      $(BINDIR)/VruiCustomToolDemo \
      $(BINDIR)/VruiCalibrator \
      $(BINDIR)/ShowEarthModel \
      $(BINDIR)/Jello \
      $(BINDIR)/ClusterJello \
      $(BINDIR)/SharedJelloServer \
      $(BINDIR)/SharedJello

.PHONY: all
all: $(ALL)

# Rule to remove all targets:
.PHONY: clean
clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(ALL)

# Rule to clean the source directory for packaging:
distclean:
	-rm -rf $(OBJDIRBASE)
	-rm -rf $(BINDIRBASE)

# List all build rules:
$(BINDIR)/VruiDemo: $(OBJDIR)/VruiDemo.o

$(BINDIR)/VruiLocatorDemo: $(OBJDIR)/VruiLocatorDemo.o

$(BINDIR)/VruiCustomToolDemo: $(OBJDIR)/VruiCustomToolDemo.o

$(BINDIR)/VruiCalibrator: $(OBJDIR)/VruiCalibrator.o

$(OBJDIR)/ShowEarthModel.o: CFLAGS += -DSHOWEARTHMODEL_IMAGEDIR='"$(INSTALLDIR)/$(RESOURCEDIR)"'

$(BINDIR)/ShowEarthModel: $(OBJDIR)/EarthFunctions.o \
                          $(OBJDIR)/PointSet.o \
                          $(OBJDIR)/SeismicPath.o \
                          $(OBJDIR)/EarthquakeSet.o \
                          $(OBJDIR)/EarthquakeTool.o \
                          $(OBJDIR)/ShowEarthModel.o

$(BINDIR)/Jello: $(OBJDIR)/JelloAtom.o \
                 $(OBJDIR)/JelloCrystal.o \
                 $(OBJDIR)/JelloRenderer.o \
                 $(OBJDIR)/Jello.o

$(BINDIR)/ClusterJello: $(OBJDIR)/JelloAtom.o \
                        $(OBJDIR)/JelloCrystal.o \
                        $(OBJDIR)/JelloRenderer.o \
                        $(OBJDIR)/ClusterJello.o

$(BINDIR)/SharedJelloServer: $(OBJDIR)/JelloAtom.o \
                             $(OBJDIR)/JelloCrystal.o \
                             $(OBJDIR)/SharedJelloServer.o

$(BINDIR)/SharedJello: $(OBJDIR)/JelloAtom.o \
                       $(OBJDIR)/JelloCrystal.o \
                       $(OBJDIR)/JelloRenderer.o \
                       $(OBJDIR)/SharedJello.o

# Rule to install the example programs in a destination directory
install: $(ALL)
	@echo Installing Vrui example programs in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(INSTALLDIR)/bin
	@install $(ALL) $(INSTALLDIR)/bin
	@install -d $(INSTALLDIR)/$(RESOURCEDIR)
	@install $(RESOURCEDIR)/EarthTopography.png $(RESOURCEDIR)/EarthTopography.ppm $(INSTALLDIR)/$(RESOURCEDIR)
