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
	cc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Iinclude src/native_format.c tests/native_format.c -o build/host/native-format
	build/host/native-format
	cc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g -DLG_HOST_PREVIEW -DLV_CONF_INCLUDE_SIMPLE -Iinclude -Iconfig/demo -Ithird_party/lvgl tests/blend.c -o build/host/blend
	build/host/blend
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
.PHONY: demo demo-host
demo: verify
	python3 scripts/build_demo.py --profile $(PROFILE) --mintlib "$(MINTLIB)"
demo-host: verify
	python3 scripts/build_demo.py --host

.PHONY: examples app-test
examples: verify
	python3 scripts/build_demo.py --example hello --profile $(PROFILE) --mintlib "$(MINTLIB)"
	python3 scripts/build_demo.py --example windows --profile $(PROFILE) --mintlib "$(MINTLIB)"
app-test: verify
	python3 scripts/build_demo.py --host --sanitize
	build/host-demo/test-app
	build/host-demo/test-mcp
