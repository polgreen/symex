DIRS = path-symex symex
CPROVER_DIR ?= lib/cbmc/src
export CPROVER_DIR

.PHONY: all
all: symex.dir

.PHONY: setup-submodules
setup-submodules:
	# Configure any extra remotes that are needed before updating the submodule
	# Note that setup_submodules.sh does not update the submodules
	./setup_submodules.sh $(USERS)
	git submodule update

.PHONY: setup-cbmc
setup-cbmc: setup-submodules
	# To make a single step build, proactively do the minisat2-download
	$(MAKE) $(MAKEARGS) -C ../$(CPROVER_DIR) minisat2-download

# building cbmc proper
.PHONY: cprover.dir
cprover.dir:
	$(MAKE) $(MAKEARGS) -C ../$(CPROVER_DIR)

.PHONY: path-symex.dir
path-symex.dir: cprover.dir

.PHONY: symex.dir
symex.dir: cprover.dir path-symex.dir

$(patsubst %, %.dir, $(DIRS)):
	## Entering $(basename $@)
	$(MAKE) $(MAKEARGS) -C $(basename $@)

# generate source files

$(patsubst %, %_generated_files, $(DIRS)):
	$(MAKE) $(MAKEARGS) -C $(patsubst %_generated_files, %, $@) generated_files

.PHONY: generated_files
generated_files: $(patsubst %, %_generated_files, $(DIRS))

# cleaning

.PHONY: clean
clean: $(patsubst %, %_clean, $(DIRS)) cprover_clean

$(patsubst %, %_clean, $(DIRS)):
	$(MAKE) $(MAKEARGS) -C $(patsubst %_clean, %, $@) clean ; \

.PHONY: cprover_clean
cprover_clean:
	$(MAKE) $(MAKEARGS) -C ../$(CPROVER_DIR) clean
