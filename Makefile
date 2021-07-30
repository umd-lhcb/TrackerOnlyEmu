# Author: Yipeng Sun
# Last Change: Thu Jul 29, 2021 at 04:50 PM +0200

.PHONY: sdist clean install install-egg

export PATH := ./scripts:$(PATH)

sdist:
	@python ./setup.py sdist

clean:
	@rm -rf ./dist
	@rm -rf ./TrackerOnlyEmu.egg-info
	@rm -rf ./gen

install:
	@pip install .

install-egg:
	@python ./setup.py install

test-all: test-hlt1 test-l0-hadron test-l0-global-tis

train-l0-hadron-bdt:
	scripts/run2-rdx-l0_hadron_train_bdt.py ./samples/rdx-bdt_train_sample.root ./gen/bdt.pickle \
		--max-depth 4 \
		--debug-ntuple ./gen/debug_bdt.root \
		--test-ntuple ./gen/test_bdt.root

test-hlt1:
	scripts/run2-rdx-hlt1.py ./samples/rdx-tracker_only.root ./gen/emu_hlt1.root

test-l0-hadron:
	scripts/run2-rdx-l0_hadron.py ./samples/rdx-tracker_only.root ./gen/emu_l0_hadron.root

test-l0-global-tis:
	scripts/run2-rdx-l0_global_tis.py ./samples/rdx-tracker_only.root ./gen/emu_l0_global_tis.root
