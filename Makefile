# Author: Yipeng Sun
# Last Change: Thu Jul 01, 2021 at 03:08 PM +0200

.PHONY: sdist clean install install-egg

export PATH := ./scripts:$(PATH)

sdist:
	@python ./setup.py sdist

clean:
	@rm -rf ./dist
	@rm -rf ./TrackerOnlyEmu.egg-info
	@rm -rf ./gen

install:
	@pip install . --force-reinstall

install-egg:
	@python ./setup.py install

test-all: test-hlt1 test-l0-hadron test-l0-global-tis

train-l0-hadron-bdt:
	@mkdir -p gen
	run2-rdx-l0_hadron_train_bdt.py ./samples/rdx-bdt_train_sample.root ./gen/bdt.pickle \
		--debug-ntuple ./gen/debug_bdt.root

test-hlt1:
	@mkdir -p gen
	run2-rdx-hlt1.py ./samples/rdx-tracker_only.root ./gen/emu_hlt1.root

test-l0-hadron:
	@mkdir -p gen
	run2-rdx-l0_hadron.py ./samples/rdx-tracker_only.root ./gen/emu_l0_hadron.root

test-l0-global-tis:
	@mkdir -p gen
	run2-rdx-l0_global_tis.py ./samples/rdx-tracker_only.root ./gen/emu_l0_global_tis.root
