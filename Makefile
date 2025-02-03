# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0


.PHONY: _generate_clean
_generate_clean:

.PHONY: generate
generate: _generate_clean
	$(MAKE) -C doc generate

super-linter:
	docker run --rm --volume $$(pwd):/tmp/lint \
		--env RUN_LOCAL=true \
		--env DEFAULT_BRANCH=main \
		--env IGNORE_GITIGNORED_FILES=true \
		--env VALIDATE_CPP=true \
		ghcr.io/super-linter/super-linter:slim-v6
