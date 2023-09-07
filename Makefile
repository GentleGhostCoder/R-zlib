.PHONY: clean docs

R_PROFILE = .Rprofile


bump-version:
	@BUMP_TYPE=$(filter-out $@,$(MAKECMDGOALS)); \
	FILE="DESCRIPTION"; \
	CURRENT_VERSION=$(shell git describe --tags $(shell git rev-list --tags --max-count=1)); \
	if [ -z "$$CURRENT_VERSION" ]; then \
		echo "No existing tags found. Consider creating an initial tag."; \
		exit 1; \
	fi; \
	MAJOR=$$(echo $$CURRENT_VERSION | cut -d'.' -f1); \
	MINOR=$$(echo $$CURRENT_VERSION | cut -d'.' -f2); \
	PATCH=$$(echo $$CURRENT_VERSION | cut -d'.' -f3); \
	case $$BUMP_TYPE in \
		"major") \
			MAJOR=$$((MAJOR + 1)); \
			MINOR=0; \
			PATCH=0; \
			;; \
		"minor") \
			MINOR=$$((MINOR + 1)); \
			PATCH=0; \
			;; \
		"patch") \
			PATCH=$$((PATCH + 1)); \
			;; \
		*) \
			echo "Unknown bump type"; \
			exit 1; \
			;; \
	esac; \
	NEW_VERSION="$$MAJOR.$$MINOR.$$PATCH"; \
	echo "Bumping version from $$CURRENT_VERSION to $$NEW_VERSION"; \
	awk -v new_version=$$NEW_VERSION 'BEGIN {OFS = FS} $$1 == "Version:" {$$2 = new_version} {print}' $$FILE > temp && mv temp $$FILE; \

# Allows us to pass the bump type as an argument
%:
	@:

all: clean
	gcc -v

clean:
	rm -rf dist
	rm -f **/*.so **/*.o **/RcppExports.cpp

.compile:
	@Rscript -e "install.packages('devtools')"
	@Rscript -e "devtools::document()"
	@Rscript -e "devtools::install_dev_deps()"
	@Rscript -e "Rcpp::compileAttributes()"
	@Rscript -e "renv::restore()"

check:
	cppcheck src --error-exitcode=1
	@Rscript -e "devtools::check(cran=TRUE)"

docs: coverage
	@rm html -rf
	@Rscript -e "devtools::document()"
	@Rscript docs/generate_docs.R
	@Rscript -e 'if (file.exists("html/index.html")) browseURL("html/index.html")'

build: clean .compile
	# build into dist/<package-name> folder with devtools
	@mkdir -p dist
	@Rscript -e "devtools::build( path = 'dist' )"

build-check: build
	@R CMD check --as-cran dist/zlib*

test:
	@Rscript -e "testthat::test_dir('tests')"

coverage:
	@Rscript -e "covr::report(file = 'html/coverage.html')"

install: clean .compile
	@Rscript -e "devtools::install()"

bump-patch:
	git pull
	$(MAKE) bump-version patch

bump-minor:
	git pull
	$(MAKE) bump-version minor

bump-major:
	git pull
	$(MAKE) bump-version major

bump: bump-patch # default

pr: bump
	@if [ -n "$$(git status --porcelain --ignore-submodules)" ]; then \
		echo "You have unstaged/committed changes. Please commit or stash them first."; \
		exit 1; \
	fi && \
	if [ -n "$$(git log @{u}..)" ]; then \
		echo "There are commits that haven't been pushed yet. Please push your changes first."; \
		exit 1; \
	fi && \
	BRANCH_NAME=$(shell git branch --show-current) && \
	gh pr create --base main --head $$BRANCH_NAME --title "PR $$BRANCH_NAME - $(shell git describe --tags $(shell git rev-list --tags --max-count=1))" --body "$(filter-out $@,$(MAKECMDGOALS))"

pr-status:
	@BRANCH_NAME=$(shell git branch --show-current) && \
	COMMIT_HASH=$(shell git rev-parse HEAD) && \
	PR_NUMBER=$$(gh pr list --base $$BRANCH_NAME --json number -q ".[0].number") && \
	CHECK_RUNS=$$(gh api --paginate repos/:owner/:repo/commits/$$COMMIT_HASH/check-runs) && \
	SUCCESSFUL=$$(echo "$$CHECK_RUNS" | jq '[.check_runs[] | select(.status == "completed" and .conclusion == "success")] | length') && \
	IN_PROGRESS=$$(echo "$$CHECK_RUNS" | jq '[.check_runs[] | select(.status == "in_progress")] | length') && \
	QUEUED=$$(echo "$$CHECK_RUNS" | jq '[.check_runs[] | select(.status == "queued")] | length') && \
	echo "$$SUCCESSFUL successful, $$IN_PROGRESS in progress, and $$QUEUED queued checks" && \
	([ $$SUCCESSFUL -gt 0 ] && [ $$IN_PROGRESS -eq 0 ] && [ $$QUEUED -eq 0 ])


#🌈
pr-merge-if-ready: bump
	@if [ -n "$$(git status --porcelain --ignore-submodules)" ]; then \
		echo "You have unstaged/committed changes. Please commit or stash them first."; \
		exit 1; \
	fi && \
	if [ -n "$$(git log @{u}..)" ]; then \
		echo "There are commits that haven't been pushed yet. Please push your changes first."; \
		exit 1; \
	fi && \
	if $(MAKE) pr-status; then \
		BRANCH_NAME=$(shell git branch --show-current) && \
		PR_NUMBER=$$(gh pr list --base $$BRANCH_NAME --json number -q ".[0].number") && \
		gh pr merge $$PR_NUMBER --auto --merge; \
	else \
		echo "PR is not ready to be merged due to pending or failing checks."; \
	fi


release: pr-merge-if-ready
	@# Fetch the latest status of the main branch from the remote
	git fetch origin main:main && \
	OPEN_PRS_COUNT=$$(gh pr list --base main --state open --json number | jq ". | length") && \
    	if [ "$$OPEN_PRS_COUNT" -ne 0 ]; then \
    		echo "There are open PRs targeting the main branch. Resolve them before creating a tag."; \
    		exit 1; \
    	fi && \
	VERSION=$$(poetry version -s) && \
	echo "Detected version: $$VERSION" && \
	if git rev-parse "$$VERSION" >/dev/null 2>&1; then \
		echo "Tag $$VERSION already exists."; \
	else \
		echo "Creating new tag $$VERSION on the remote main branch." && \
		git tag "$$VERSION" origin/main && \
		git push origin "$$VERSION" && \
		gh release create "$$VERSION" --title "$$VERSION 🌈" --generate-notes; \
		echo "Tag $$VERSION has been created on the remote main branch and pushed. A new GitHub release has been made with title '$$VERSION 🌈'."; \
	fi

install-fast:
	R CMD INSTALL .


