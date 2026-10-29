PROFILE ?= mint060
.PHONY: test verify probe bootstrap
verify:
	python3 scripts/verify_dependencies.py
bootstrap:
	sh scripts/bootstrap.sh
test:
	mkdir -p build/host
	cc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Iinclude src/pixels.c tests/pixels.c -o build/host/pixels
	build/host/pixels
probe: verify
	python3 scripts/build_probe.py --profile $(PROFILE) --mintlib "$(MINTLIB)"
.PHONY: lvgl
lvgl: verify
	python3 scripts/build_lvgl.py --profile $(PROFILE) --mintlib "$(MINTLIB)"
.PHONY: bytes inspect
bytes: lvgl
	python3 scripts/build_bytes.py --profile $(PROFILE) --mintlib "$(MINTLIB)"
inspect:
	python3 scripts/inspect_binary.py build/$(PROFILE)/LVPROBE.APP build/$(PROFILE)/liblvgl.a build/$(PROFILE)/LVBYTES.TTP
