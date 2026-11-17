#!/system/bin/sh
ui_print "- AETHER Serenity 0.9.1 hotfix"
if [ "$(getprop ro.product.cpu.abi)" != "arm64-v8a" ]; then
  abort "! AETHER supports arm64-v8a only."
fi
if [ ! -f "$MODPATH/system/bin/aether-supervisor" ]; then
  abort "! Missing aether-supervisor binary. Build the package with scripts/package-module.sh."
fi
if [ ! -f "$MODPATH/web/index.html" ]; then
  abort "! Missing WebUI file: web/index.html"
fi

# Set safe defaults, then restore executable permissions explicitly.
set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm "$MODPATH/service.sh" 0 0 0755
set_perm "$MODPATH/uninstall.sh" 0 0 0755
set_perm "$MODPATH/system/bin/aether-supervisor" 0 0 0755

# Fail installation if the supervisor is not executable after permission setup.
if [ ! -x "$MODPATH/system/bin/aether-supervisor" ]; then
  abort "! aether-supervisor is not executable after permission setup."
fi
