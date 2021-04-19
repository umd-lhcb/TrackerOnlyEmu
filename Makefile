# Author: Yipeng Sun
# Last Change: Mon Apr 19, 2021 at 03:26 AM +0200

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

test-hlt1:
	@mkdir -p gen
	run2-rdx-hlt1.py ./samples/rdx-tracker_only.root ./gen/emu_hlt1.root

test-l0:
	@mkdir -p gen
	run2-rdx-l0_hadron.py ./samples/rdx-tracker_only.root ./gen/emu_l0.root
