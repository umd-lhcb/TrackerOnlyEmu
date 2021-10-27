# Author: Yipeng Sun
# Last Change: Wed Oct 27, 2021 at 03:29 AM +0200

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

test-all: test-hlt1 test-l0-global-tis

# RD+'s approach, no longer the nominal
test-l0-hadron-bdt:
	scripts/run2-rdx-l0_hadron_trainload_bdt.py ./samples/run2_rdx-train.root ./gen/emu_l0_hadron_bdt.root \
		--max-depth 4 --debug \
		--dump-bdt ./gen/bdt.pickle

test-hlt1:
	scripts/run2-rdx-hlt1.py ./samples/rdx-tracker_only.root ./gen/emu_hlt1.root

test-l0-global-tis:
	scripts/run2-rdx-l0_global_tis.py ./samples/rdx-tracker_only.root ./gen/emu_l0_global_tis.root
