# Author: Yipeng Sun
# Last Change: Wed Mar 31, 2021 at 11:54 PM +0200

.PHONY: sdist clean install install-egg

sdist:
	@python ./setup.py sdist

clean:
	@rm -rf ./dist
	@rm -rf ./TrackerOnlyEmu.egg-info

install:
	@pip install . --force-reinstall

install-egg:
	@python ./setup.py install
