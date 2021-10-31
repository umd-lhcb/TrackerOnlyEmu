# Author: Yipeng Sun
# Last Change: Sun Oct 31, 2021 at 03:38 AM +0100

.PHONY: sdist clean install install-egg

export PATH := ./scripts:$(PATH)

sdist:
	@python ./setup.py sdist

clean:
	@rm -rf ./dist
	@rm -rf ./TrackerOnlyEmu.egg-info
	@rm -rf ./gen/*

install:
	@pip install .

install-egg:
	@python ./setup.py install

test-all: test-hlt1 test-l0-global-tis test-l0-hadron-bdt test-l0-hadron-xgb

# RD+'s approach, no longer the nominal
test-l0-hadron-bdt:
	scripts/run2-rdx-l0_hadron_tos.py -m bdt ./samples/run2-rdx-train_bdt.root ./gen/emu_l0_hadron_bdt.root \
		--max-depth 4 \
		--dump ./gen/bdt4.pickle

test-l0-hadron-xgb:
	scripts/run2-rdx-l0_hadron_tos.py -m xgb ./samples/run2-rdx-train_xgb.root ./gen/emu_l0_hadron_xgb.root \
		--max-depth 4 \
		--dump ./gen/xgb4.pickle

test-hlt1:
	scripts/run2-rdx-hlt1.py ./samples/run2-rdx-sample.root ./gen/emu_hlt1.root

test-l0-global-tis:
	scripts/run2-rdx-l0_global_tis.py ./samples/run2-rdx-sample.root ./gen/emu_l0_global_tis.root
