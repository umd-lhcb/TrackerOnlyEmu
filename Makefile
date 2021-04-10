# Author: Yipeng Sun
# Last Change: Sun Apr 11, 2021 at 01:03 AM +0200

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
