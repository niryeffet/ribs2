EXAMPLES=httpd helloworld mydump httpget

all:
	@echo "[ribs2] build"
	@$(MAKE) -C src -s
	@echo "[ribs2] success"
	@$(MAKE) -s $(EXAMPLES:%=example_%)
	@$(MAKE) -s unit_tests

example_%:
	@echo "[examples/$(@:example_%=%)] build"
	@$(MAKE) -C examples/$(@:example_%=%)/src -s
	@echo "[examples/$(@:example_%=%)] success"

unit_tests:
	@echo "[$@] build"
	@$(MAKE) -C tests/src -s
	@echo "[$@] success"

clean: $(EXAMPLES:%=clean_example_%)
	@echo "[ribs2] clean"
	@$(MAKE) -C src -s clean

clean_example_%:
	@echo "[examples/$(@:clean_example_%=%)] clean"
	@$(MAKE) -C examples/$(@:clean_example_%=%)/src -s clean

etags:
	@echo "etags"
	find . -regex ".*\.[cChH]\(pp\)?" -print | etags -

test:   all
	@echo "Running tests"
	@./scripts/run_tests.sh
