# Login:  xzvara01
# Author: Adam Zvara
# Date:   2.3.2023

# Compiler settings
HC = ghc
HC_OPTS = -Wall

# Source files
SRCS = $(wildcard *.hs)
HS_PROG = flp-fun

# Main target
all:
	$(HC) $(HC_OPTS) -o $(HS_PROG) $(SRCS)

# Clean the build files
clean:
	rm -f $(HS_PROG) *.hi *.o