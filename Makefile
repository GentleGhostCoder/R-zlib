.PHONY: clean docs

R_PROFILE = .Rprofile

all: clean
	gcc -v

clean:
	rm -f **/*.so **/*.o **/RcppExports.cpp

.compile:
	@Rscript -e "if(!require('devtools')) install.packages('devtools')"
	@Rscript -e "devtools::document()"
	@Rscript -e "devtools::install_dev_deps()"
	@Rscript -e "Rcpp::compileAttributes()"
	@Rscript -e "renv::snapshot()"

check:
	cppcheck src --error-exitcode=1
	@Rscript -e "devtools::check()"

docs:
	@rm html -rf
	@Rscript -e "devtools::document()"
	@Rscript docs/generate_docs.R
	@Rscript -e 'if (file.exists("html/index.html")) browseURL("html/index.html")'

build: clean .compile
	# build into dist/<package-name> folder with devtools
	@mkdir -p dist
	@Rscript -e "devtools::build( path = 'dist' )"

test:
	@Rscript -e "testthat::test_dir('tests')"

install: clean .compile
	@Rscript -e "devtools::install()"

install-fast:
	@Rscript -e "devtools::install()"